
#include <string.h>
#include <stdio.h>
/* For using usleep */
#include <unistd.h>
#include "ecrt.h"



/* One motor revolution increments the encoder by 2^19 -1 */
#define ENCODER_RES 524287


void ODwrite(ec_master_t* master, uint16_t slavePos, uint16_t index, uint8_t subIndex, uint8_t objectValue)
{
	/* Blocks until a reponse is received */
	uint8_t retVal = ecrt_master_sdo_download(master, slavePos, index, subIndex, &objectValue, sizeof(objectValue), NULL);
	/* retVal != 0: Failure */
	if (retVal)
		printf("OD write unsuccessful\n");
}

int main(int argc, char **argv)
{

	ec_master_t* master;
	/* Reserve the first master (0) (/etc/init.d/ethercat start) for this program */
	master = ecrt_request_master(0);
	if (!master)
		printf("Requesting master failed\n");
	
	ODwrite(master, 0, 0x6040, 0x00, 0xF);
	ODwrite(master, 0, 0x6060, 0x00, 0x8);
	
	uint16_t alias0 = 0;
	uint16_t position0 = 0;
	uint32_t vendor_id0 = 0x00007595;
	uint32_t product_code0 = 0x00000000;
	
	/* Creates and returns a slave configuration object, ec_slave_config_t*, for the given alias and position. */
	/* Returns NULL in case of error and pointer to the configuration struct otherwise */
	ec_slave_config_t* drive0 = ecrt_master_slave_config(master, alias0, position0, vendor_id0, produc_code0);
	
	/* If the drive0 = NULL */
	if (!drive0)
	{
		printf("Failed to get slave configuration\n");
		return -1;
	}
	
	/* Structures obtained from $ethercat cstruct -p 0 */
	/***************************************************/
	
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
	
	/***************************************************/
	
	if (ecrt_slave_config_pdos(drive0, EC_END, slave_0_syncs))
	{
		printf("Failed to configure PDOs\n");
		return -1;
	}
	

	unsigned int offset_controlWord, offset_targetPos, , offset_statusWord, offset_actPos;
	
	ec_pdo_entry_reg_t domain1_regs[] =
	{
	{0, 0, 0x00007595, 0x00000000, 0x6040, 0x00, &offset_controlWord},
	{0, 0, 0x00007595, 0x00000000, 0x607a, 0x00, &offset_targetPos  },
	{0, 0, 0x00007595, 0x00000000, 0x6041, 0x00, &offset_statusWord },
	{0, 0, 0x00007595, 0x00000000, 0x6064, 0x00, &offset_actPos     },
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
	
	
	int i = 0;
	int32_t actPos;
	int32_t targetPos;
	
	
	
	/* After a few frames the slave (drive) enters OP mode */
	while (i <= 10000)
	{
		/* Fetches received frames from the newtork device and processes the datagrams. */
		ecrt_master_receive(master);
		/* Evaluates the working counters of the received datagrams and outputs statistics,
		   if necessary.
		   This function is NOT essential to the receive/process/send procedure and can be 
		   commented out 
		*/
		ecrt_domain_process(domain1);
		/********************************************************************************/
		
		/* Read PDOs from the datagram */
		actPos = EC_READ_S32(domain1_pd + offset_actPos);
		
		/* Process the received data */
		targetPos = actPos + ENCODER_RES;
		
		/* Write PDOs to the datagram */
		EC_WRITE_U8  (domain1_pd + offset_controlWord, 0xF );
		EC_WRITE_S32 (domain1_pd + offset_targetPos  , targetPos);
		
		/********************************************************************************/
		/* Queues all domain datagrams in the master's datagram queue. 
		   Call this function to mark the domain's datagrams for exchanging at the
		   next call of ecrt_master_send() 
		*/
		ecrt_domain_queue(domain1);
		/* Sends all datagrams in the queue.
		   This method takes all datagrams that have been queued for transmission,
		   puts them into frames, and passes them to the Ethernet device for sending. 
		*/
		ecrt_master_send(master);
		
		i = i + 1;
		usleep(300);
	}
	
	return 0;
}