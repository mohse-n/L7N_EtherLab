#include "ecrt.h"

/* Write to OD entrties using this function */
int ecrt_slave_config_sdo16(
        ec_slave_config_t *sc, /**< Slave configuration */
        uint16_t sdo_index, /**< Index of the SDO to configure. */
        uint8_t sdo_subindex, /**< Subindex of the SDO to configure. */
        uint16_t value /**< Value to set. */
        );


	
	
	
/* Defining PDOs in the program */
	
typedef struct {
    uint16_t alias; /**< Slave alias address. */
    uint16_t position; /**< Slave position. */
    uint32_t vendor_id; /**< Slave vendor ID. */
    uint32_t product_code; /**< Slave product code. */
    uint16_t index; /**< PDO entry index. */
    uint8_t subindex; /**< PDO entry subindex. */
    unsigned int *offset; /**< Pointer to a variable to store the PDO entry's
                       (byte-)offset in the process data. */
    unsigned int *bit_position; /**< Pointer to a variable to store a bit
                                  position (0-7) within the \a offset. Can be
                                  NULL, in which case an error is raised if the
                                  PDO entry does not byte-align. */
} ec_pdo_entry_reg_t;


static uint32_t gkOffOControl;
static uint8_t* gkDomain1PD = NULL;
static ec_domain_t*         gkDomain1 = NULL;

/* Note that the last one is empty */
const static ec_pdo_entry_reg_t gkDomain1Regs[] = {
    {0, gkDriveNum, 0x00007595, 0x00000000, 0x6040, 0, &gkOffOControl},
    {0, gkDriveNum, 0x00007595, 0x00000000, 0x607a, 0, &gkOffOPos},
    {0, gkDriveNum, 0x00007595, 0x00000000, 0x6041, 0, &gkOffIStatus},
    {0, gkDriveNum, 0x00007595, 0x00000000, 0x6062, 0, &gkOffDPos},
    {0, gkDriveNum, 0x00007595, 0x00000000, 0x6064, 0, &gkOffIPos},
    {0, gkDriveNum, 0x00007595, 0x00000000, 0x606b, 0, &gkOffDVel},
    {0, gkDriveNum, 0x00007595, 0x00000000, 0x606c, 0, &gkOffIVel},
    {0, gkDriveNum, 0x00007595, 0x00000000, 0x6081, 0, &gkOffPVel},
    {0, gkDriveNum, 0x00007595, 0x00000000, 0x6060, 0, &gkOffOMode},
    {0, gkDriveNum, 0x00007595, 0x00000000, 0x6061, 0, &gkOffIMode},
    {0, gkDriveNum, 0x00007595, 0x00000000, 0x60ff, 0, &gkOffTVel},
    {0, gkDriveNum, 0x00007595, 0x00000000, 0x6077, 0, &gkOffITorq},
    {}
};

	    if (!(gkDomain1PD = ecrt_domain_data(gkDomain1))) {
      fprintf(stderr,"Domain data initialization failed.\n");
      return -1;
    }
	
	/* In the operational mode (motion loop) */
	EC_WRITE_U16(gkDomain1PD + gkOffOControl, 0xF)

					/* Another example of domain defintion */
					
/* Alias is always zero, slave numbers start from 1 */
					
#define MotorSlavePos0 0, 0
#define MotorSlavePos1 0, 1
const static ec_pdo_entry_reg_t domainOutput_regs_0[] = {
 { MotorSlavePos0, MBDHT2510BA1, 0x6040, 0, &mbdh_cntlwd_0 },
 { MotorSlavePos0, MBDHT2510BA1, 0x6060, 0, &mbdh_modeop_0 },
 { MotorSlavePos0, MBDHT2510BA1, 0x607A, 0, &mbdh_tarpos_0 },
 { MotorSlavePos0, MBDHT2510BA1, 0x60B8, 0, &mbdh_tpbfnc_0 },
 {}
};


const static ec_pdo_entry_reg_t domainInput_regs_1[] = {
 { MotorSlavePos1, MBDHT2510BA1, 0x603f, 0, &mbdh_errcod_1 },
 { MotorSlavePos1, MBDHT2510BA1, 0x6041, 0, &mbdh_statwd_1 },
 { MotorSlavePos1, MBDHT2510BA1, 0x6061, 0, &mbdh_modedp_1 },
 { MotorSlavePos1, MBDHT2510BA1, 0x6064, 0, &mbdh_actpos_1 },
 { MotorSlavePos1, MBDHT2510BA1, 0x60B9, 0, &mbdh_tpdsta_1 },
 { MotorSlavePos1, MBDHT2510BA1, 0x60BA, 0, &mbdh_tpbpos_1 },
 { MotorSlavePos1, MBDHT2510BA1, 0x60F4, 0, &mbdh_errval_1 },
 { MotorSlavePos1, MBDHT2510BA1, 0x60FD, 0, &mbdh_digiin_1 },
 {}
};
	
	
	


/* See line 665 of ethercat/include/ecrt.h */
sc = ecrt_master_slave_config(
        ec_master_t *master, /**< EtherCAT master */
        uint16_t alias, /**< Slave alias. */
        uint16_t position, /**< Slave position. */
        uint32_t vendor_id, /**< Expected vendor ID. */
        uint32_t product_code /**< Expected product code. */
        );
/* See line 524 of ethercat/master/slave_config.c */
ecrt_slave_config_sync_manager(sc, 2, EC_DIR_OUTPUT, EC_WD_ENABLE )
ecrt_slave_config_sync_manager(sc, 3, EC_DIR_INPUT, EC_WD_DISABLE )