#include "ecrt.h"

/* See line 665 of ethercat/include/ecrt.h */
sc = ecrt_master_slave_config(
        ec_master_t *master, /**< EtherCAT master */
        uint16_t alias, /**< Slave alias. */
        uint16_t position, /**< Slave position. */
        uint32_t vendor_id, /**< Expected vendor ID. */
        uint32_t product_code /**< Expected product code. */
        );
/* See line 524 of ethercat/master/slave_config.c */
ecrt_slave_config_sync_manager(sc, 2, EC_DIR_INPUT, ? )
ecrt_slave_config_sync_manager(sc, 3, EC_DIR_INPUT, ? )