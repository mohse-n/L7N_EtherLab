
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
	}
	
	return 0;
}