#ifndef __SCR_H__
#define __SCR_H__

#define __SCR_H__

#include "rtdevice.h"

#define TRIGER_PIN  GET_PIN(B, 14)



void pwm_enable(void);
void pwm_disable(void);

#define ENABLE_SCR_TRIGER()   pwm_enable()
#define DISABLE_SCR_TRIGER()  pwm_disable()



#define MIN_TRIGER_ANGLE()   
#define MAX_TRIGER_ANGLE()



#define MIN_PASS_ANGLE  20
#define MAX_PASS_ANGLE  160 //88

#define  CROSS_ZERO_TIME_ERR   400  //us

struct scr_dev
{
	rt_device_t     triger_angle_timer;	//触发角定时器
	rt_device_t     timer_1ms;	        //触发时长定时器   //1ms 定时器
	rt_hwtimerval_t triger_angle_delay;    // 触发角定时器
	rt_hwtimerval_t timer_1ms_delay;    // 触发信号设定值
	rt_uint16_t     angle;			//导通角
};

typedef  struct scr_dev * scr_dev_t;
extern struct scr_dev  g_scr_dev;

void triger_angle_timer_start(void);
#endif
