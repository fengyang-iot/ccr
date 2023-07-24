/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-08-20     BruceOu      first implementation
 */
//#define DBG_LEVEL         DBG_LOG
#define DBG_TAG  "main.c"
#include <stdio.h>
#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include "cross_zero_check.h"
#include "rtdbg.h"
#include "database.h"
#include "string.h"
#include "i2c_app.h"
#include "alarm.h"
#include "pid_app.h"
#include <math.h>
/* defined the LED2 pin: PF0 */


//static rt_device_t serial_1 = NULL;
char *str =  "hello, world\r\n";
rt_uint32_t adval_vrms, vrms_out;



#define ADC_VREF 330
#define ADC_BITS  ((1<<12)-1)
extern float	   gActualTpInt0;
extern rt_uint8_t g_pid_enable; 
typedef enum{
	
	GJ_0 = 0,
	GJ_1,
	GJ_2,
	GJ_3,
	GJ_4,
	GJ_5
}eGJ;
const 
typedef struct{
	
	eGJ cur_levl;  //电流等级
	
}Dev_data;


int main(void)
{
	while(1){
		rt_thread_mdelay(1000);
		
	};
}

static rt_int16_t my_atoi(const char * str)
{
	rt_uint16_t result = 0;
    rt_uint8_t sign = 0;
	const char * pstr = str;
	if(pstr == NULL)
	{
		return 0;
	}
	rt_uint8_t cnt = 0;
    if(*pstr == '-')
    {
        sign = 1;
        pstr++;
    }
	while( *pstr != '\0')
	{
 
		cnt++;
		result = 10*result + (*pstr-'0');
		pstr++;
	}
    
	return sign? -result: result;
}

static int set_idensity(int argc, char *argv[])
{
	rt_uint8_t idensity;
	if(argc <2)
	{
		LOG_E("usage: set_idensity [1..5]");
	}
	
	idensity  = my_atoi(argv[1]);
	update_idensity(idensity);
	write_sysdata_to_eeprom(EEPROM_WRITE_DATABASE_VALUE);
    if(idensity == 0)
    {
        RELAY_OFF();
        CLR_SYS_STATUS_BIT(RELAY_STATUS_POS);
    }
    else if(idensity <=5 )
	{
        pid_ref_set(&pid, Istd[idensity-1]);
    }
	return 0;
}

MSH_CMD_EXPORT(set_idensity, "set_idensity");


static int relay_ctrl(int argc, char *argv[])
{

	if(argc <2)
	{
		LOG_E("usage: relay_ctrl on/off");
	}
	
	if(rt_strcmp(argv[1], "on") == 0)
	{
		RELAY_ON();
		SET_SYS_STATUS_BIT(RELAY_STATUS_POS);
	}
	else if( rt_strcmp(argv[1], "off" )== 0)
	{
		RELAY_OFF();
		CLR_SYS_STATUS_BIT(RELAY_STATUS_POS);
	}
	return 0;
}


MSH_CMD_EXPORT(relay_ctrl, "relay_ctrl");

static int set_angle(int argc, char *argv[])
{
	rt_uint16_t angle;
	if(argc <2)
	{
		LOG_E("usage: set_angle angle");
	}
	
	angle = my_atoi(argv[1]);
	if((angle >=9) &&  (angle <= 180))
	{
		g_ccr_data.ccr_info.angle = angle;
	}
	
	return 0;
}
MSH_CMD_EXPORT(set_angle, "set angle");



static int set_un(int argc, char * argv[])
{
	rt_int16_t un_a,un_b;
	if(argc <3)
	{
		LOG_E("set un: a  b");
       // LOG_I("系数均扩大10倍输入");
        return 0;
	}
	
	un_a = my_atoi(argv[1]);
    un_b = my_atoi(argv[2]);

	g_ccr_data.ccr_info.Un_a = un_a/10.0f;	
    g_ccr_data.ccr_info.Un_b = un_b/10.0f;
	write_sysdata_to_eeprom(EEPROM_WRITE_DATABASE_VALUE);
	return 0;
}
MSH_CMD_EXPORT(set_un, "set un");

static int set_In(int argc, char * argv[])
{
	rt_int16_t In_a, In_b;
	if(argc <3)
	{
		LOG_E("set In: In_a  In_b" );
        return 0;
	}
	
	In_a = my_atoi(argv[1]);
    In_b = my_atoi(argv[2]);

	g_ccr_data.ccr_info.In_a = In_a/100.0f;
    g_ccr_data.ccr_info.In_b = In_b/100.0f;
	
	write_sysdata_to_eeprom(EEPROM_WRITE_DATABASE_VALUE);
	return 0;
}
MSH_CMD_EXPORT(set_In, "set In");

static int pid_ctrl(int argc, char *argv[])
{
    if(argc <2)
	{
		LOG_E("usage: relay_ctrl on/off");
	}
	
	if(rt_strcmp(argv[1], "on") == 0)
	{
        g_pid_enable = 1;
	}
	else if( rt_strcmp(argv[1], "off" )== 0)
    {
        g_pid_enable = 0;
	}
	return 0;
}
MSH_CMD_EXPORT(pid_ctrl, "pid_ctrl");
static int show_info(void)
{
	LOG_I("freq:\t%d.%dHz", g_ccr_data.ccr_info.freq/10,g_ccr_data.ccr_info.freq%10 );
	LOG_I("Iload:\t%d.%02dA", g_ccr_data.ccr_info.i_load/100, g_ccr_data.ccr_info.i_load%100);
	LOG_I("Uscr:\t%d.%dV", g_ccr_data.ccr_info.v_scr/10, g_ccr_data.ccr_info.v_scr%10);
	LOG_I("PassAngle:\t%d°", (rt_uint16_t)(g_ccr_data.ccr_info.angle));
//	LOG_I("Un:      \t%d", (rt_uint16_t)g_ccr_data.ccr_info.m_sysparam.Un_10);
//	LOG_I("In:     \t%d", (rt_uint16_t)g_ccr_data.ccr_info.m_sysparam.In*10);
    LOG_I("i_lvl:\t%d", g_ccr_data.ccr_info.intensity);
    LOG_I("ADC_I_VAL:\t0.%03dA",  (rt_uint32_t)(g_ccr_data.ccr_info.adc_val_i*1000));
    LOG_I("ADC_uscr_VAL: 0.%03dV",  (rt_uint32_t)(g_ccr_data.ccr_info.adc_val_vscr*1000));
    LOG_I("In_a: %d  In_b: %c%d", (rt_uint32_t)(g_ccr_data.ccr_info.In_a*100), g_ccr_data.ccr_info.In_b<0 ? '-':' ', (rt_int32_t)(fabs(g_ccr_data.ccr_info.In_b)*100));
    LOG_I("Un_a: %d  Un_b: %c%d", (rt_uint32_t)(g_ccr_data.ccr_info.Un_a*10), g_ccr_data.ccr_info.Un_b<0 ? '-':' ', (rt_int32_t)(fabs(g_ccr_data.ccr_info.Un_b)*10));
	return 0;
}
MSH_CMD_EXPORT(show_info, "show info");
//void timer_config(void)
//{
//    /* ---------------------------------------------------------------------------
//    TIMER1 configuration: output compare toggle mode:
//    TIMER1CLK = systemcoreclock / 6000=20K,
//    CH1 update rate = TIMER1 counter clock / CH1VAL = 20000/4000 = 5 Hz
//    ----------------------------------------------------------------------------*/
//    timer_oc_parameter_struct timer_ocintpara;
//    timer_parameter_struct timer_initpara;

//    rcu_periph_clock_enable(RCU_TIMER1);

//    timer_deinit(TIMER1);

//    /* TIMER1 configuration */
//    timer_initpara.prescaler         = 5999;
//    timer_initpara.alignedmode       = TIMER_COUNTER_EDGE;
//    timer_initpara.counterdirection  = TIMER_COUNTER_UP;
//    timer_initpara.period            = 3999;
//    timer_initpara.clockdivision     = TIMER_CKDIV_DIV1;
//    timer_initpara.repetitioncounter = 0;
//    timer_init(TIMER1,&timer_initpara);

//     /* CH1 configuration in OC TOGGLE mode */
//    timer_ocintpara.outputstate  = TIMER_CCX_ENABLE;
//    timer_ocintpara.outputnstate = TIMER_CCXN_DISABLE;
//    timer_ocintpara.ocpolarity   = TIMER_OC_POLARITY_HIGH;
//    timer_ocintpara.ocnpolarity  = TIMER_OCN_POLARITY_HIGH;
//    timer_ocintpara.ocidlestate  = TIMER_OC_IDLE_STATE_LOW;
//    timer_ocintpara.ocnidlestate = TIMER_OCN_IDLE_STATE_LOW;
//    
//    timer_channel_output_config(TIMER1,TIMER_CH_1,&timer_ocintpara);

//    timer_channel_output_pulse_value_config(TIMER1,TIMER_CH_1,3999);
//    timer_channel_output_mode_config(TIMER1,TIMER_CH_1,TIMER_OC_MODE_TOGGLE);
//    timer_channel_output_shadow_config(TIMER1,TIMER_CH_1,TIMER_OC_SHADOW_DISABLE);

//    /* auto-reload preload enable */
//    timer_auto_reload_shadow_enable(TIMER1);
//    timer_enable(TIMER1);
//}
