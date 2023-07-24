#ifndef __SCR_H__
#define __SCR_H__

#define __SCR_H__

#include "rtdevice.h"

#define TRIGER_PIN  GET_PIN(B, 14)

#define ENABLE_SCR_TRIGER()  rt_pin_write(TRIGER_PIN, PIN_LOW)
#define DISABLE_SCR_TRIGER() rt_pin_write(TRIGER_PIN, PIN_HIGH)

struct scr_dev
{
	rt_device_t     angle_timer;	//导通延时定时器
	rt_device_t     triger_timer;	//触发时长定时器
	rt_hwtimerval_t angle_delay;    // 定时器超时值
	rt_uint16_t     angle;			//导通角
};

typedef  struct scr_dev * scr_dev_t;
extern struct scr_dev  g_scr_dev;

void angle_timer_start();
#endif
