#include "database.h"

ALIGN(RT_ALIGN_SIZE)
ccr_data_t g_ccr_data;

sys_para_t g_sys_param;

int data_base_init(void)
{
	g_ccr_data.ccr_info.angle = MIN_PASS_ANGLE;  //
	return 0;
    
}
INIT_PREV_EXPORT(data_base_init);
