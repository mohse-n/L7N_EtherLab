#include "../../include/ecrt.h"




static ec_master_t* master;
static ec_domain_t* domain1 = NULL;
static uint8_t* domain1_pd;

static ec_slave_config_t* drive0 = NULL;
static ec_slave_config_t* drive1 = NULL;

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
	
/* Structures obtained from $ethercat cstruct -p 0 */
/***************************************************/
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
	
	/***************************************************/

/* Why define data? */
void run(long data)
{
	while(1)
	{
		
	}
	
}

	/***************************************************/
	
int __init init_mod(void)
{
	
	
	
}


void __exit cleanup_mod(void)
{
	
	
	
}
	
	/***************************************************/
	
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mohsen Alizadeh Noghani");
MODULE_DESCRIPTION("EtherLab RTAI sample module");

module_init(init_mod);
module_exit(cleanup_mod);
	
	
	
	
	

