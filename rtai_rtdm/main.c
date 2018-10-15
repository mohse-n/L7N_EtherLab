#include "ecrt.h"

#include <rtai_lxrt.h>
#include <rtdm/rtdm.h>

#include <string.h>
#include <stdio.h>
/* For setting the process's priority (setpriority) */
#include <sys/resource.h>
/* For pid_t and getpid() */
#include <unistd.h>
#include <sys/types.h>
/* For locking the program in RAM (mlockall) to prevent swapping */
#include <sys/mman.h>
/* Header for handling signals (definition of SIGINT) */
#include <signal.h>
/* For sched_setscheduler(...) and sched_param */
#include <sched.h>


/* One motor revolution increments the encoder by 2^19 -1 */
#define ENCODER_RES 524287

/* No reason to define this as global, other than accessibility */
const static unsigned int cycle_us = 1000; /* 1 ms */

void ODwrite(ec_master_t* master, uint16_t slavePos, uint16_t index, uint8_t subIndex, uint8_t objectValue)
{
	/* Blocks until a reponse is received */
	uint8_t retVal = ecrt_master_sdo_download(master, slavePos, index, subIndex, &objectValue, sizeof(objectValue), NULL);
	/* retVal != 0: Failure */
	if (retVal)
		printf("OD write unsuccessful\n");
}

void initDrive(ec_master_t* master, uint16_t slavePos)
{
	/* Reset alarm */
	ODwrite(master, slavePos, 0x6040, 0x00, 128);
	/* Servo on and operational */
	ODwrite(master, slavePos, 0x6040, 0x00, 0xF);
	/* Mode of operation, CSP */
	ODwrite(master, slavePos, 0x6060, 0x00, 0x8);
}

/* We have to pass "master" to ecrt_release_master in signal_handler, but it is not possible
   to define one with more than one argument. Therefore, master should be a global variable. 
*/
ec_master_t* master;
/* The same logic apples in case of defining task as global */
RT_TASK* task;

void signal_handler(int sig)
{
	printf("\nEnding the program...\n");
	
	/* Return a hard real time Linux process, or pthread to the standard Linux behavior. */	
	rt_make_soft_real_time();
	
        stop_rt_timer();
	rt_task_delete(task);
	ecrt_release_master(master);
	
	pid_t pid = getpid();
	kill(pid, SIGKILL);
}

int main(int argc, char **argv)
{
	
	/* Lock the program into RAM and prevent swapping */
	if (mlockall(MCL_CURRENT | MCL_FUTURE) == -1)
	{
		printf("mlockall failed");
		return -1;
	}
	
	signal(SIGINT, signal_handler);
	
	/* Reserve the first master (0) (/etc/init.d/ethercat start) for this program */
	master = ecrt_request_master(0);
	if (!master)
		printf("Requesting master failed\n");
	
	initDrive(master, 0);
	initDrive(master, 1);
	
	uint16_t alias = 0;
	uint16_t position0 = 0;
	uint16_t position1 = 1;
	uint32_t vendor_id = 0x00007595;
	uint32_t product_code = 0x00000000;
	
	/* Creates and returns a slave configuration object, ec_slave_config_t*, for the given alias and position. */
	/* Returns NULL (0) in case of error and pointer to the configuration struct otherwise */
	ec_slave_config_t* drive0 = ecrt_master_slave_config(master, alias, position0, vendor_id, product_code);
	ec_slave_config_t* drive1 = ecrt_master_slave_config(master, alias, position1, vendor_id, product_code);
	
	
	/* If the drive0 = NULL or drive1 = NULL */
	if (!drive0 || !drive1)
	{
		printf("Failed to get slave configuration\n");
		return -1;
	}
	
	/* Structures obtained from $ethercat cstruct -p 0 */
	/***************************************************/
	/* Slave 0's structures */
	ec_pdo_entry_info_t slave_0_pdo_entries[] = 
	{
	{0x6040, 0x00, 16}, /* Controlword */
	{0x607a, 0x00, 32}, /* Target Position */
	{0x6041, 0x00, 16}, /* Statusword */
	{0x6064, 0x00, 32}, /* Position Actual Value */
	};
	
	ec_pdo_info_t slave_0_pdos[] =
	{
	{0x1601, 2, slave_0_pdo_entries + 0}, /* 2nd Receive PDO Mapping */
	{0x1a01, 2, slave_0_pdo_entries + 2}, /* 2nd Transmit PDO Mapping */
	};
	
	ec_sync_info_t slave_0_syncs[] =
	{
	{0, EC_DIR_OUTPUT, 0, NULL            , EC_WD_DISABLE},
	{1, EC_DIR_INPUT , 0, NULL            , EC_WD_DISABLE},
	{2, EC_DIR_OUTPUT, 1, slave_0_pdos + 0, EC_WD_DISABLE},
	{3, EC_DIR_INPUT , 1, slave_0_pdos + 1, EC_WD_DISABLE},
	{0xFF}
	};
	
	/* Slave 1's structures */
	ec_pdo_entry_info_t slave_1_pdo_entries[] = 
	{
	{0x6040, 0x00, 16}, /* Controlword */
	{0x607a, 0x00, 32}, /* Target Position */
	{0x6041, 0x00, 16}, /* Statusword */
	{0x6064, 0x00, 32}, /* Position Actual Value */
	};
	
	ec_pdo_info_t slave_1_pdos[] =
	{
	{0x1601, 2, slave_1_pdo_entries + 0}, /* 2nd Receive PDO Mapping */
	{0x1a01, 2, slave_1_pdo_entries + 2}, /* 2nd Transmit PDO Mapping */
	};
	
	ec_sync_info_t slave_1_syncs[] =
	{
	{0, EC_DIR_OUTPUT, 0, NULL            , EC_WD_DISABLE},
	{1, EC_DIR_INPUT , 0, NULL            , EC_WD_DISABLE},
	{2, EC_DIR_OUTPUT, 1, slave_1_pdos + 0, EC_WD_DISABLE},
	{3, EC_DIR_INPUT , 1, slave_1_pdos + 1, EC_WD_DISABLE},
	{0xFF}
	};
	
	/***************************************************/
	
	if (ecrt_slave_config_pdos(drive0, EC_END, slave_0_syncs))
	{
		printf("Failed to configure slave 0 PDOs\n");
		return -1;
	}
	
	if (ecrt_slave_config_pdos(drive1, EC_END, slave_1_syncs))
	{
		printf("Failed to configure slave 1 PDOs\n");
		return -1;
	}

	unsigned int offset_controlWord0, offset_targetPos0, offset_statusWord0, offset_actPos0;
	unsigned int offset_controlWord1, offset_targetPos1, offset_statusWord1, offset_actPos1;
	
	ec_pdo_entry_reg_t domain1_regs[] =
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
	
	/* Creates a new process data domain. */
	/* For process data exchange, at least one process data domain is needed. */
	ec_domain_t* domain1 = ecrt_master_create_domain(master);
	
	/* Registers PDOs for a domain. */
	/* Returns 0 on success. */
	if (ecrt_domain_reg_pdo_entry_list(domain1, domain1_regs))
	{
		printf("PDO entry registration failed\n");
		return -1;
	}
	
	/* Up to this point, we have only requested the master. See log messages */
	printf("Activating master...\n");
	/* Important points from ecrt.h 
	   - This function tells the master that the configuration phase is finished and
	     the real-time operation will begin. 
	   - It tells the master state machine that the bus configuration is now to be applied.
	   - By calling the ecrt master activate() method, all slaves are configured according to
             the prior method calls and are brought into OP state.
	   - After this function has been called, the real-time application is in charge of cylically
	     calling ecrt_master_send() and ecrt_master_receive(). Before calling this function, the 
	     master thread is responsible for that. 
	   - This method allocated memory and should not be called in real-time context.
	*/
	     
	if (ecrt_master_activate(master))
		return -1;

	
	uint8_t* domain1_pd;
	/* Returns a pointer to (I think) the first byte of PDO data of the domain */
	if (!(domain1_pd = ecrt_domain_data(domain1)))
		return -1;
	
	struct sched_param param;
        param.sched_priority = sched_get_priority_max(SCHED_FIFO) - 1;
        if (sched_setscheduler(0, SCHED_FIFO, &param) == -1) 
	{
		printf("Error in setting the scheduler\n");
		return -1;
	}
	
	/* The slaves (drives) enter OP mode after exchanging a few frames.
	   By setting opFlag to 1, we start exchanging PDO's only after both slaves have reached operational state. 
	*/
	uint8_t opFlag = 0;
	ec_slave_config_state_t slaveState0;
	ec_slave_config_state_t slaveState1;
	
	/* What does nam2num do? */
	task = rt_task_init(nam2num("ec_rtai_rtdm_example"), 0 /* priority */, 0 /* stack size */, 0 /* msg size */);
	/* "rt_set_periodic_mode" sets the periodic mode for the timer. It consists of a fixed frequency 
	timing of the tasks in multiple of the period set with a call to "start_rt_timer".
	*/
	rt_set_periodic_mode();
	period = (int) nano2count((RTIME) cycle_us * 1000);
	/* Start an RTAI timer.
	   - A clock tick will last as the period set in start_rt_timer (requested_ticks).
	   - tick period will be the “real” number of ticks used for the timer period (which can be different from the requested one).
	*/
	start_rt_timer(period);
		
        /* Gives a this process, or pthread, hard real time execution capabilities allowing full kernel preemption. 
	   Note that processes made hard real time should avoid making any Linux System call that can lead to a task switch. 
	   After all, it doesn't make any sense to use a non-realtime OS (i.e. Linux) from a hard real-time process.
	*/   
	rt_make_hard_real_time();
	/* rt_task_make_periodic (struct rt_task_struct *task, RTIME start_time, RTIME period) */
	/* At "rt_get_time() + 10 * period", start the task with an update rate of "period" */
	rt_task_make_periodic(task, rt_get_time() + 10 * period, period);
            
	while(1)
	{
		/* Make the current process wait for the next periodic release point in the processor time line. */
		rt_task_wait_period();
	
		ecrt_master_receive(master);
		ecrt_domain_process(domain1);
	
		/********************************************************************************/
		
		/* If all slave are in operational state, exchange PDO data */
		if (opFlag)
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
			ecrt_slave_config_state(drive0, &slaveState0);
			ecrt_slave_config_state(drive1, &slaveState1);
			
			/* If all slaves have reache OP state, set the flag to 1 */
			if (slaveState0.operational && slaveState1.operational)
			{
				/*printk(KERN_ERR PFX "All slaves have reached OP state\n");*/
				opFlag = 1;
			}
		}
		
		/********************************************************************************/
		
		ecrt_domain_queue(domain1);
		ecrt_master_send(master);
	}
	
	return 0;
	
}
