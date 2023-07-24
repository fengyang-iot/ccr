#ifndef __DATABASE_H__
#define __DATABASE_H__

#include "rtdef.h"
#include "rtthread.h"
#include "scr.h"

//40101
typedef struct sys_param
{
	/* RW 区*/
	rt_uint16_t on_off;	 	// 0:off 	1:on
	rt_uint16_t ctrl_mode; 	// 0:local 	1:remote
	rt_uint16_t intensity;  //灯光级别 
//	rt_uint16_t status;  	//运行状态字
	rt_uint16_t run_time_lv1;
	rt_uint16_t run_time_lv2;
	rt_uint16_t run_time_lv3;
	rt_uint16_t run_time_lv4;
	rt_uint16_t run_time_lv5;
	rt_uint16_t run_time_avg_lv5;
	float  In_a;		     //电流比例系数a	  y = ax+b
    float  In_b;             //电流比例系数b
	float  Un_a;             //SCR电压比例系数a    
    float  Un_b;          	 //SCR电压常数b
    float  Uload_a;          //Uload电压比例系数a    
    float  Uload_b;          //Uload电压比例系数b
}sys_para_t;

extern sys_para_t g_sys_param;
//struct ccr_datastruct
//{
//	
//	struct sys_param  m_sysparam;
//	/* Read Only 区*/
//	rt_uint16_t i_load;		//灯光回路电流		0.01A			
//	rt_uint16_t i_scr;		//可控硅回路电流	0.01A
//	
//	rt_uint32_t v_scr;	   	//可控硅输出电压 	0.1v
//	rt_uint32_t v_load;		//灯光回路电压  	0.1v
//	
//	rt_uint16_t freq;	  	 //电源频率 0.1hz
//	rt_uint16_t half_period;  //半波周期
//	rt_uint16_t angle;		  //导通角
//	
//	rt_uint16_t status;		//详见运行状态字说明  bit0: 1:run
//			
//	float  CCRInt0Time;
//	
//	
//};

struct ccr_datastruct
{
	
	//struct sys_param  m_sysparam;
	/* RW 区*/
	rt_uint16_t on_off;	 	// 0:off 	1:on    /0x64
	rt_uint16_t ctrl_mode; 	// 0:local 	1:remote
	rt_uint16_t intensity;  //电流等级  
	rt_uint16_t reserved[17];
	/* Read Only 区*/
	rt_uint16_t v_scr;	   	//可控硅输出电压 	0.1v   //0x78
	rt_uint16_t i_load;		//灯光回路电流		0.01A	//0x79		
	rt_uint16_t v_load;		//灯光回路电压  	0.1v
	rt_uint16_t run_time_lv1;
	rt_uint16_t run_time_lv2;
	rt_uint16_t run_time_lv3;
	rt_uint16_t run_time_lv4;
	rt_uint16_t run_time_lv5;
	rt_uint16_t run_time_avg_lv5;
	rt_uint16_t status;    //状态字
    rt_uint16_t run_status; //运行状态字
    rt_uint16_t alarm_switch;
	
	rt_uint16_t freq;	  	 //电源频率 0.1hz
	rt_uint16_t half_period;  //半波周期
	float angle;		  //导通角		
	float  In_a;		     //电流比例系数a	  y = ax+b
    float  In_b;             //电流比例系数b
	float  Un_a;             //SCR电压比例系数a    
    float  Un_b;          	 //SCR电压常数b
    float  Uload_a;          //Uload电压比例系数a    
    float  Uload_b;          //Uload电压比例系数b
    float  adc_val_i;
    float  adc_val_vscr;
    float  adc_val_vload;
//	float  CCRInt0Time;//
	
	
};
#define CCR_BUF_LEN  sizeof(struct ccr_datastruct)
union ccr_data
{
	rt_uint16_t  buf[CCR_BUF_LEN];
	struct ccr_datastruct ccr_info;	
};

typedef union ccr_data ccr_data_t;

extern ccr_data_t g_ccr_data;

#define  RELAY_STATUS_OFF   0
#define  RELAY_STATUS_ON   	1

// 系统状态  bit 0~ 3
#define  RELAY_STATUS_POS   0


//FA

#define  SYS_STATUS_W   g_ccr_data.ccr_info.run_status
#define  SYS_ALARM_REG  g_ccr_data.ccr_info.status
#define SET_SYS_STATUS_BIT( BIT)      	(SYS_STATUS_W  = SYS_STATUS_W|(1<<BIT))
#define CLR_SYS_STATUS_BIT(BIT)	   		(SYS_STATUS_W = SYS_STATUS_W&(~(1<<BIT)))
#define GET_SYS_STATUS_BIT( BIT)	  	(SYS_STATUS_W&(1<<BIT))

#define SET_ALARM_STATUS_BIT(BIT)       (SYS_ALARM_REG |= (1<<BIT))
#define CLR_ALARM_STATUS_BIT(BIT)       (SYS_ALARM_REG &= ~(1<<BIT))
#define GET_ALARM_STATUS_BIT(BIT)       (SYS_ALARM_REG &(1<<BIT))


#define IS_SYS_FAULT_EXIST()    (SYS_ALARM_REG & (0x00ff))   // 低8位有置位说明有故障


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


/** Un ****/
rt_inline void update_Un(float Un_a, float Un_b )
{
	g_ccr_data.ccr_info.Un_a = Un_a;
    g_ccr_data.ccr_info.Un_b = Un_b;
}
//rt_inline rt_uint16_t get_Un(void)
//{
//	return g_ccr_data.ccr_info.Un;
//}

/** In ****/
//rt_inline void update_In(rt_uint16_t In)
//{
//	g_ccr_data.ccr_info.In_a = In;
//}
//rt_inline rt_uint16_t get_In(void)
//{
//	return g_ccr_data.ccr_info.In_a;
//}
rt_inline void update_In(float In_a, float In_b)
{
	g_ccr_data.ccr_info.In_a = In_a;
    g_ccr_data.ccr_info.In_b = In_b;
}
rt_inline float get_In_a(void)
{
	return g_ccr_data.ccr_info.In_a;
}
rt_inline float get_In_b(void)
{
	return g_ccr_data.ccr_info.In_b;
}
rt_inline float get_Un_a(void)
{
	return g_ccr_data.ccr_info.Un_a;
}
rt_inline float get_Un_b(void)
{
	return g_ccr_data.ccr_info.Un_b;
}


rt_inline void update_pass_angle(float angle)
{
//    if(angle <=MIN_PASS_ANGLE)
//    {
        g_ccr_data.ccr_info.angle =angle;
//    }
//    else if(angle >=MAX_PASS_ANGLE)
//	{
//		g_ccr_data.ccr_info.angle  = MAX_PASS_ANGLE;
//	}
}

rt_inline float get_pass_angle(float angle)
{
	return g_ccr_data.ccr_info.angle;
}
rt_inline void update_idensity(rt_uint16_t idensity)
{
	if(idensity <=5)
	{
		g_ccr_data.ccr_info.intensity  = idensity;
	}
}

rt_inline rt_uint8_t get_idensity(void)
{
	return g_ccr_data.ccr_info.intensity;
	
}
/** angle**/

//rt_inline  void update_angle(rt_uint16_t angle)
//{
//	if(angle >= MAX_PASS_ANGLE)
//	{
//		g_ccr_data.ccr_info.angle = MAX_PASS_ANGLE;
//	}
//	else if(angle<= MIN_PASS_ANGLE)
//	{
//		
//	}
//}

#endif
