#ifndef __DATABASE_H__
#define __DATABASE_H__

#include "rtdef.h"
#include "rtthread.h"


struct sys_param
{
	/* RW 区*/
	rt_uint16_t intensity;  //电流等级  
	rt_uint16_t ctrl_mode; 	// 0:local 	1:remote
	rt_uint16_t on_off;	 	// 0:off 	1:on
	
	rt_uint16_t over_cur_prew_percent;		//过流预警百分比设置值
	rt_uint16_t over_cur_w_percent;			//过流报警百分比设置值
	rt_uint16_t over_vol_prew_percent;		//过压预警百分比设置值
	rt_uint16_t over_vol_w_percent;			//过压报警百分比设置值
	
	rt_uint16_t In;		//电流比例系数						
	rt_uint16_t Un;     //电压比例系数
	rt_uint16_t reserved;
};
struct ccr_datastruct
{
	
	struct sys_param  m_sysparam;
	/* Read Only 区*/
	rt_uint16_t i_load;		//灯光回路电流		0.01A			
	rt_uint16_t i_scr;		//可控硅回路电流	0.01A
	
	rt_uint32_t v_scr;	   	//可控硅输出电压 	0.1v
	rt_uint32_t v_load;		//灯光回路电压  	0.1v
	
	rt_uint16_t freq;	  	//电源频率 0.1hz
	
	rt_uint16_t status;		//详见运行状态字说明
			
	float  CCRInt0Time;
	
	
};
#define CCR_BUF_LEN  sizeof(struct ccr_datastruct)
union ccr_data
{
	rt_uint8_t  buf[CCR_BUF_LEN];
	struct ccr_datastruct ccr_info;	
};

typedef union ccr_data ccr_data_t;

extern ccr_data_t g_ccr_data;



/**  freq **/
rt_inline void update_freq(rt_uint16_t freq)
{
	g_ccr_data.ccr_info.freq = freq;
}

rt_inline rt_uint16_t get_freq(void)
{
	return g_ccr_data.ccr_info.freq;
}

/** scr voltage**/
rt_inline void update_scr_voltage(rt_uint32_t voltage)
{
	g_ccr_data.ccr_info.v_scr = voltage;
}

rt_inline rt_uint32_t get_scr_voltage(void)
{
	 return g_ccr_data.ccr_info.v_scr;
}


/** load voltage **/
rt_inline void update_load_voltage(rt_uint32_t voltage)
{
	g_ccr_data.ccr_info.v_load = voltage;
}

rt_inline rt_uint32_t get_load_voltage(void)
{
	return g_ccr_data.ccr_info.v_load;
}


/** I load  **/
rt_inline void update_i_load(rt_uint16_t i_load)
{
	g_ccr_data.ccr_info.i_load = i_load;
}
rt_inline rt_uint16_t get_i_load(void)
{
	return g_ccr_data.ccr_info.i_load;
}

/** I scr  **/
rt_inline void update_i_scr(rt_uint16_t i_scr)
{
	g_ccr_data.ccr_info.i_scr = i_scr;
}
rt_inline rt_uint16_t get_i_scr(void)
{
	return g_ccr_data.ccr_info.i_scr;
}

/** Un ****/
rt_inline void update_Un(rt_uint16_t Un)
{
	g_ccr_data.ccr_info.m_sysparam.Un = Un;
}
rt_inline rt_uint16_t get_Un(void)
{
	return g_ccr_data.ccr_info.m_sysparam.Un;
}

/** In ****/
rt_inline void update_In(rt_uint16_t In)
{
	g_ccr_data.ccr_info.m_sysparam.In = In;
}
rt_inline rt_uint16_t get_In(void)
{
	return g_ccr_data.ccr_info.m_sysparam.In;
}

#endif
