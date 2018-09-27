
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

}