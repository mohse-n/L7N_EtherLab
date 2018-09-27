
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
	
	ODwrite(master, 0, 0x6040, 0x00, 0xF);
	
	return 0;
}