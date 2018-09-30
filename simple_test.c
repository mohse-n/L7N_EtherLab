
#include <string.h>
#include <stdio.h>
#include "ecrt.h"

void ODwrite(ec_master_t* master, uint16_t slavePos, uint16_t index, uint8_t subIndex, uint8_t objectValue)
{
	
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
	
	//ODwrite(master, 0, 0x6040, 0x00, 0xF);
	
	uint16_t alias0 = 0;
	uint16_t position0 = 0;
	uint32_t vendor_id0 = 0x00007595;
	uint32_t product_code0 = 0x00000000;
	
	/* Creates and returns a slave configuration object, ec_slave_config_t*, for the given alias and position */
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
	
	ec_domain_t* domain1 = ecrt_master_create_domain(master);
	
	/* Registers PDOs for a domain */
	/* Returns 0 on success */
	if (ecrt_domain_reg_pdo_entry_list(domain1, domain1_regs))
	{
		printf("PDO entry registration failed\n");
		return -1;
	}
	

		
	
	
	
	
	
	
	return 0;
}