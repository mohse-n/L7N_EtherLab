
#include <string.h>
#include <stdio.h>
#include "ecrt.h"

int main(int argc, char **argv)
{

	ec_master_t* master;
	/* Reserve the first master (0) (/etc/init.d/ethercat start) for this program */
	master = ecrt_request_master(0);
	if (!master)
		printf("Requesting master failed\n");
	
	uint8_t controlWord = 0xF;
	uint8_t retVal;
	
	retVal = ecrt_master_sdo_download(master, 0, 0x6040, 0x00, &controlWord, sizeof(controlWord), NULL)
	if (!retVal)
		printf("OD write successful\n");
}