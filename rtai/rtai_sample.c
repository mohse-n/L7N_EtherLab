#include "../../include/ecrt.h"

#include <linux/module.h>

/* Need this for nano2count */
#include <rtai_sched.h>
/* For sem_signal etc. */
/* Why do we get an error if we've included it inside ifdef endif ? */
#include <rtai_sem.h>

#define FREQUENCY 2000
#define TIMERTICKS (1000000000 / FREQUENCY)
#define PFX "ec_rtai_sample: "
/* One motor revolution increments the encoder by 2^19 -1 */
#define ENCODER_RES 524287

/* Uncomment to use distributed clocks */
#define DC
/* Uncomment to use semaphore on the master module */
#define SEM
/* Uncomment to use callback funtion for master */
#define CB


#ifdef DC

#define NSEC_PER_SEC (1000000000L)
#define USEC_PER_SEC (1000000L)

/* Period of motion loop, in microseconds */
#define PERIOD_US (USEC_PER_SEC / FREQUENCY)
/* Period of motion loop, in nanoseconds */
#define PERIOD_NS (NSEC_PER_SEC / FREQUENCY)
/* SYNC0 event happens halfway through the cycle */
#define SHIFT0 (PERIOD_NS/2)

#endif


#ifdef CB

#define INHIBIT_TIME 20

#endif





static ec_master_t* master;
static ec_domain_t* domain1 = NULL;
static uint8_t* domain1_pd;

static ec_slave_config_t* drive0 = NULL;
static ec_slave_config_t* drive1 = NULL;

static uint32_t opCounter = 0;

static int32_t actPos0, targetPos0;
static int32_t actPos1, targetPos1;

static RT_TASK task;

#ifdef SEM
static SEM master_sem;
#endif

#ifdef CB
static cycles_t t_last_cycle = 0, t_critical;
#endif

/* Structures obtained from $ethercat cstruct -p 0 */
/*****************************************************************************/

/* Slave 0's structures */
static ec_pdo_entry_info_t slave_0_pdo_entries[] = 
{
	{0x6040, 0x00, 16}, /* Controlword */
	{0x607a, 0x00, 32}, /* Target Position */
	{0x6041, 0x00, 16}, /* Statusword */
	{0x6064, 0x00, 32}, /* Position Actual Value */
};
	
static ec_pdo_info_t slave_0_pdos[] =
{
	{0x1601, 2, slave_0_pdo_entries + 0}, /* 2nd Receive PDO Mapping */
	{0x1a01, 2, slave_0_pdo_entries + 2}, /* 2nd Transmit PDO Mapping */
};
	
static ec_sync_info_t slave_0_syncs[] =
{
	{0, EC_DIR_OUTPUT, 0, NULL            , EC_WD_DISABLE},
	{1, EC_DIR_INPUT , 0, NULL            , EC_WD_DISABLE},
	{2, EC_DIR_OUTPUT, 1, slave_0_pdos + 0, EC_WD_DISABLE},
	{3, EC_DIR_INPUT , 1, slave_0_pdos + 1, EC_WD_DISABLE},
	{0xFF}
};
	
/* Slave 1's structures */
static ec_pdo_entry_info_t slave_1_pdo_entries[] = 
{
	{0x6040, 0x00, 16}, /* Controlword */
	{0x607a, 0x00, 32}, /* Target Position */
	{0x6041, 0x00, 16}, /* Statusword */
	{0x6064, 0x00, 32}, /* Position Actual Value */
};
	
static ec_pdo_info_t slave_1_pdos[] =
{
	{0x1601, 2, slave_1_pdo_entries + 0}, /* 2nd Receive PDO Mapping */
	{0x1a01, 2, slave_1_pdo_entries + 2}, /* 2nd Transmit PDO Mapping */
};
	
static ec_sync_info_t slave_1_syncs[] =
{
	{0, EC_DIR_OUTPUT, 0, NULL            , EC_WD_DISABLE},
	{1, EC_DIR_INPUT , 0, NULL            , EC_WD_DISABLE},
	{2, EC_DIR_OUTPUT, 1, slave_1_pdos + 0, EC_WD_DISABLE},
	{3, EC_DIR_INPUT , 1, slave_1_pdos + 1, EC_WD_DISABLE},
	{0xFF}
};
	
/*****************************************************************************/

static unsigned int offset_controlWord0, offset_targetPos0, offset_statusWord0, offset_actPos0;
static unsigned int offset_controlWord1, offset_targetPos1, offset_statusWord1, offset_actPos1;

const static ec_pdo_entry_reg_t domain1_regs[] =
{
	{0, 0, 0x00007595, 0x00000000, 0x6040, 0x00, &offset_controlWord0},
	{0, 0, 0x00007595, 0x00000000, 0x607a, 0x00, &offset_targetPos0  },
	{0, 0, 0x00007595, 0x00000000, 0x6041, 0x00, &offset_statusWord0 },
	{0, 0, 0x00007595, 0x00000000, 0x6064, 0x00, &offset_actPos0     },
	
	{0, 1, 0x00007595, 0x00000000, 0x6040, 0x00, &offset_controlWord1},
	{0, 1, 0x00007595, 0x00000000, 0x607a, 0x00, &offset_targetPos1  },
	{0, 1, 0x00007595, 0x00000000, 0x6041, 0x00, &offset_statusWord1 },
	{0, 1, 0x00007595, 0x00000000, 0x6064, 0x00, &offset_actPos1     },
	{}
};

/*****************************************************************************/	

void ODwrite(ec_slave_config_t* slaveConfig, uint16_t index, uint8_t subIndex, uint8_t objectValue)
{
	/* For some reason, using "ecrt_master_sdo_download" would result in a kerne oops 
	   (Unable to handle kernel NULL pointer dereference).
	   Writing the value actually happens after activating the master. See the answer to "Read from/write to the slave's object dictionary using SDOs" in the mailing list.
	*/
	uint8_t retVal = ecrt_slave_config_sdo(slaveConfig, index, subIndex, &objectValue, sizeof(objectValue));
	/* retVal != 0: Failure */
	if (retVal)
		printk(KERN_ERR PFX "OD write request unsuccessful\n");
}

void initDrive(ec_slave_config_t* slaveConfig)
{
	/* Reset alarm */
	ODwrite(slaveConfig, 0x6040, 0x00, 128);
	/* Servo on and operational */
	ODwrite(slaveConfig, 0x6040, 0x00, 0xF);
	/* Mode of operation, CSP */
	ODwrite(slaveConfig, 0x6060, 0x00, 0x8);
}

/*****************************************************************************/

/* The parent task can pass 1 long variable (data) to the new task */
void run(long data)
{
	
	#ifdef DC
	struct timeval tv;
        unsigned int sync_ref_counter = 0;
	count2timeval(nano2count(rt_get_real_time_ns()), &tv);
	#endif
	
	while(1)
	{
		#ifdef CB
		t_last_cycle = get_cycles();
		#endif
		
		#ifdef SEM
		rt_sem_wait(&master_sem);
		#endif
		ecrt_master_receive(master);
		ecrt_domain_process(domain1);
		#ifdef SEM
		rt_sem_signal(&master_sem);
		#endif
	
		/********************************************************************************/
		
		/* If all slave are in operational state, exchange PDO data. */
		/* To check slaves' state, I've used a more elegant method in the "user" example.
		   However, compiling with "ecrt_slave_config_state" would freeze the OS at insmod.
		*/
		/* After activating the master, it starts applying the specified
		   configurations (SDOs that should be sent, PDOs that have been defined).
		   Therefore it takes a few cycles for ALL the drives to reach OP and be able to 
		   move the motors. During this time, we don't try to move the motors (i.e. targetPos = actPos).
		   Note that the value 1000 is based purely on trial-and-error.
		*/
		if (opCounter == 1000)
		{
			/* Read PDOs from the datagram */
			actPos0 = EC_READ_S32(domain1_pd + offset_actPos0);
			actPos1 = EC_READ_S32(domain1_pd + offset_actPos1);
		
			/* Process the received data */
			targetPos0 = actPos0 + ENCODER_RES;
			targetPos1 = actPos1 - ENCODER_RES;
		
			/* Write PDOs to the datagram */
			EC_WRITE_U8  (domain1_pd + offset_controlWord0, 0xF );
			EC_WRITE_S32 (domain1_pd + offset_targetPos0  , targetPos0);
		
			EC_WRITE_U8  (domain1_pd + offset_controlWord1, 0xF );
			EC_WRITE_S32 (domain1_pd + offset_targetPos1  , targetPos1);
		
		}
		else
		{
			opCounter = opCounter + 1;
			/* Read PDOs from the datagram */
			actPos0 = EC_READ_S32(domain1_pd + offset_actPos0);
			actPos1 = EC_READ_S32(domain1_pd + offset_actPos1);
		
			/* Process the received data */
			targetPos0 = actPos0;
			targetPos1 = actPos1;
		
			/* Write PDOs to the datagram */
			EC_WRITE_U8  (domain1_pd + offset_controlWord0, 0xF );
			EC_WRITE_S32 (domain1_pd + offset_targetPos0  , targetPos0);
		
			EC_WRITE_U8  (domain1_pd + offset_controlWord1, 0xF );
			EC_WRITE_S32 (domain1_pd + offset_targetPos1  , targetPos1);

		}
		
		/********************************************************************************/
		
		ecrt_domain_queue(domain1);
		
		#ifdef SEM
		rt_sem_wait(&master_sem);
		#endif
		
		#ifdef DC
		tv.tv_usec += PERIOD_US;
			
      		if (tv.tv_usec >= 1000000)  
		{
			tv.tv_usec -= 1000000;
                	tv.tv_sec++;
                }

                ecrt_master_application_time(master, EC_TIMEVAL2NANO(tv));
		ecrt_master_sync_reference_clock(master);
       		ecrt_master_sync_slave_clocks(master);
		#endif
		
		ecrt_master_send(master);
		
		#ifdef SEM
		rt_sem_signal(&master_sem);
		#endif
			
		/* Wait till the next period.
		   - rt_task_wait_period suspends the execution of the currently running real time task until the next period is reached.
		   - The task must have been previously marked for a periodic execution by calling rt_task_make_periodic().
		*/
		rt_task_wait_period();
		
	}
	
}

/*****************************************************************************/

#ifdef CB

void send_callback(void *cb_data)
{
	ec_master_t *m = (ec_master_t *) cb_data;

	// too close to the next real time cycle: deny access...
	if (get_cycles() - t_last_cycle <= t_critical) 
	{
		#ifdef SEM
		rt_sem_wait(&master_sem);
		#endif
		
		ecrt_master_send_ext(m);
		
		#ifdef SEM
		rt_sem_signal(&master_sem);
		#endif
	}
}

#endif

/*****************************************************************************/

#ifdef CB

void receive_callback(void *cb_data)
{
	ec_master_t *m = (ec_master_t *) cb_data;

	// too close to the next real time cycle: deny access...
	if (get_cycles() - t_last_cycle <= t_critical) 
	{
		#ifdef SEM
		rt_sem_wait(&master_sem);
		#endif
		
		ecrt_master_receive(m);
		
		#ifdef SEM
		rt_sem_signal(&master_sem);
		#endif
	}
}

#endif

/*****************************************************************************/
	
int __init init_mod(void)
{
	
	#ifdef SEM
	rt_sem_init(&master_sem, 1);
	#endif
	
	#ifdef CB
	t_critical = cpu_khz * 1000 / FREQUENCY - cpu_khz * INHIBIT_TIME / 1000;
	#endif
	
	/* Reserve the first master (0) (/etc/init.d/ethercat start) for this program */
	master = ecrt_request_master(0);
	/* Return value of out_return */
	int ret = -1;
	
	if (!master) 
	{
		ret = -EBUSY;
		printk(KERN_ERR PFX "Requesting master 0 failed!\n");
		goto out_return;
	}
	
	#ifdef CB
	ecrt_master_callbacks(master, send_callback, receive_callback, master);
	#endif
	
	uint16_t alias = 0;
	uint16_t position0 = 0;
	uint16_t position1 = 1;
	uint32_t vendor_id = 0x00007595;
	uint32_t product_code = 0x00000000;
	
	/* Creates and returns a slave configuration object, ec_slave_config_t*, for the given alias and position. */
	/* Returns NULL (0) in case of error and pointer to the configuration struct otherwise */
	ec_slave_config_t* drive0 = ecrt_master_slave_config(master, alias, position0, vendor_id, product_code);
	ec_slave_config_t* drive1 = ecrt_master_slave_config(master, alias, position1, vendor_id, product_code);
	
	/* If drive0 = NULL or drive1 = NULL */
	if (!drive0 || !drive1)
	{
		printk(KERN_ERR PFX "Failed to get slave configuration\n");
		goto out_release_master;
	}
	
	initDrive(drive0);
	initDrive(drive1);

	if (ecrt_slave_config_pdos(drive0, EC_END, slave_0_syncs))
	{
		printk(KERN_ERR PFX "Failed to configure slave 0 PDOs\n");
		goto out_release_master;
	}
	
	if (ecrt_slave_config_pdos(drive1, EC_END, slave_1_syncs))
	{
		printk(KERN_ERR PFX "Failed to configure slave 1 PDOs\n");
		goto out_release_master;
	}

	/* Creates a new process data domain. */
	/* For process data exchange, at least one process data domain is needed. */
	domain1 = ecrt_master_create_domain(master);
	
	if (!domain1) 
	{
		printk(KERN_ERR PFX "Domain creation failed!\n");
		goto out_release_master;
	}
	
	/* Registers PDOs for a domain. */
	/* Returns 0 on success. */
	if (ecrt_domain_reg_pdo_entry_list(domain1, domain1_regs))
	{
		printk(KERN_ERR PFX "PDO entry registration failed!\n");
		goto out_release_master;
	}
	
	#ifdef DC
	ecrt_slave_config_dc(drive0, 0x0300, PERIOD_NS, SHIFT0, 0, 0);
	ecrt_slave_config_dc(drive1, 0x0300, PERIOD_NS, SHIFT0, 0, 0);
	#endif
	
	if (ecrt_master_activate(master)) 
	{
		printk(KERN_ERR PFX "Failed to activate master!\n");
		goto out_release_master;
	}
	
	/* Returns a pointer to (I think) the first byte of PDO data of the domain */
	domain1_pd = ecrt_domain_data(domain1);
	
	/* Normally, we would start frame exchange at this point (see user/main.c) */
	/*******************************   RTAI config   ********************************/
	
	/* RTIME is defined as long long (typedef long long RTIME).
	   #define TIMERTICKS (1000000000 / FREQUENCY) (ticks in nanoseconds).
	*/
	RTIME requested_ticks = nano2count(TIMERTICKS);
	
	/* Start an RTAI timer.
	   - A clock tick will last as the period set in start_rt_timer (requested_ticks).
	   - tick period will be the “real” number of ticks used for the timer period (which can be different from the requested one).
	*/
	RTIME tick_period = start_rt_timer(requested_ticks);
	printk(KERN_INFO PFX "RT timer started with %i/%i ticks.\n", (int) tick_period, (int) requested_ticks);
	
	
	/* rt_task_init(struct rt_task_struct *task, 
			void(*rt_thread)(int)	   ,	Pointer to function (= name of the function)
			int data		   ,	The partent task can send 1 integer value to the new task
			int stack_size		   ,	
			int priority	           ,	
			int uses_fpu	           ,
			void (*signal)(void))	   );
	*/
	if (rt_task_init(&task, run, 0, 2000, 0, 1, NULL)) 
	{
		printk(KERN_ERR PFX "Failed to init RTAI task!\n");
		goto out_stop_timer;
	}
	
	
	RTIME now = rt_get_time();
	
	/* rt_task_make_periodic (struct rt_task_struct *task, RTIME start_time, RTIME period) */
	/* At "now + tick_period", start the task with update rate of "tick_period" */
	if (rt_task_make_periodic(&task, now + tick_period, tick_period)) 
	{
		printk(KERN_ERR PFX "Failed to run RTAI task!\n");
		goto out_stop_task;
	}
	
	
	printk(KERN_INFO PFX "Initialized.\n");
	return 0;
	
	
	out_stop_task:
		rt_task_delete(&task);
	out_stop_timer:
		stop_rt_timer();
	out_release_master:
		printk(KERN_ERR PFX "Releasing master...\n");
		ecrt_release_master(master);
	out_return:
		#ifdef SEM
		rt_sem_delete(&master_sem);
		#endif
		
		printk(KERN_ERR PFX "Failed to load. Aborting.\n");
		return ret;
	
}


void __exit cleanup_mod(void)
{
	printk(KERN_INFO PFX "Stopping...\n");
	 
	rt_task_delete(&task);
	stop_rt_timer();
	ecrt_release_master(master);
	
	#ifdef SEM
	rt_sem_delete(&master_sem);
	#endif
	
	printk(KERN_INFO PFX "Unloading.\n");
}
	
/*****************************************************************************/
	
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mohsen Alizadeh Noghani");
MODULE_DESCRIPTION("EtherLab RTAI sample module");

module_init(init_mod);
module_exit(cleanup_mod);
	
	
	
	
	

