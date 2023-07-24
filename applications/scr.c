#include "scr.h"
#include "pid.h"
#include "database.h"
#include "board.h"

#define DBG_TAG  "scr.c"
#include "rtdbg.h"

#define  SCR_ANGLE  g_ccr_data.ccr_info.angle
ALIGN(RT_ALIGN_SIZE)
struct scr_dev  g_scr_dev;


#define  FIX_HALF_PERIOD  10000
//计算从过零点到开始触发的延时时间，需要补偿过零误差时间 CROSS_ZERO_TIME_ERR
static rt_uint32_t angle_to_delay_us(float degree)
{
    rt_int16_t tmp_delay;
//    tmp_delay = (rt_int16_t)(degree/180.0f *  g_ccr_data.ccr_info.half_period);     //(500*2)*10000000.0f);
//	//delay = g_ccr_data.ccr_info.half_period - delay + CROSS_ZERO_TIME_ERR;
//	tmp_delay = g_ccr_data.ccr_info.half_period - tmp_delay- 1000+CROSS_ZERO_TIME_ERR;
    
        tmp_delay = (rt_int16_t)(degree/180.0f *  FIX_HALF_PERIOD);     //(500*2)*10000000.0f);
	    tmp_delay = FIX_HALF_PERIOD - tmp_delay;
//	tmp_delay = g_ccr_data.ccr_info.half_period - tmp_delay- 1000+CROSS_ZERO_TIME_ERR;
    
    if(tmp_delay <1000)
    {
        tmp_delay = 1000;
    }
    else if (tmp_delay >9000)
    {
        tmp_delay = 9000;
    }
	return (rt_uint32_t)tmp_delay;
}


static rt_err_t triger_angle_timer_cb(rt_device_t  dev, rt_size_t size)
{+
	rt_device_control(g_scr_dev.triger_angle_timer, HWTIMER_CTRL_STOP, 0);
	ENABLE_SCR_TRIGER();
	//rt_device_write(g_scr_dev.timer_1ms,0,&g_scr_dev.timer_1ms_delay, sizeof(g_scr_dev.timer_1ms_delay));
	return RT_EOK;
}

static rt_err_t timer_1ms_cb(rt_device_t  dev, rt_size_t size)
{  
    triger_angle_timer_start();
	rt_device_control(g_scr_dev.timer_1ms, HWTIMER_CTRL_STOP, 0);
	return RT_EOK;
}

/** 启动 导通角延时定时器*/
void triger_angle_timer_start(void)
{
	g_scr_dev.triger_angle_delay.usec = angle_to_delay_us(SCR_ANGLE); //
	rt_device_write(g_scr_dev.triger_angle_timer,0,&g_scr_dev.triger_angle_delay, sizeof(g_scr_dev.triger_angle_delay));
}


static void scr_timer_init(void)
{
	rt_hwtimer_mode_t mode = HWTIMER_MODE_ONESHOT;
    g_scr_dev.triger_angle_timer = rt_device_find("timer2");
    if (RT_NULL == g_scr_dev.triger_angle_timer)
    {
        LOG_E("cannot find triger_angle_timer");
		return;
    }
	//1Mhz up 
	rt_device_open(g_scr_dev.triger_angle_timer, RT_DEVICE_FLAG_RDWR);
	
	rt_device_set_rx_indicate(g_scr_dev.triger_angle_timer, triger_angle_timer_cb);
		

	rt_device_control(g_scr_dev.triger_angle_timer, HWTIMER_CTRL_MODE_SET, &mode);
	
	g_scr_dev.triger_angle_delay.sec = 0;
	g_scr_dev.triger_angle_delay.usec = angle_to_delay_us(SCR_ANGLE); //
	NVIC_SetPriority(TIMER2_IRQn, 0);
	
	// timer 3;
	 g_scr_dev.timer_1ms = rt_device_find("timer3");
    if (RT_NULL == g_scr_dev.timer_1ms)
    {
        LOG_E("cannot find timer_1ms");
		return;
    }
    
	//1Mhz up 
	rt_device_open(g_scr_dev.timer_1ms, RT_DEVICE_FLAG_RDWR);
	
	rt_device_set_rx_indicate(g_scr_dev.timer_1ms, timer_1ms_cb);
		

	rt_device_control(g_scr_dev.timer_1ms, HWTIMER_CTRL_MODE_SET, &mode);
	
	g_scr_dev.timer_1ms_delay.sec = 0;
	g_scr_dev.timer_1ms_delay.usec = 1000; // 1ms
	
}
static  void triger_pin_init(void )
{
	rt_pin_mode(TRIGER_PIN, PIN_MODE_OUTPUT);
	rt_pin_write(TRIGER_PIN, PIN_LOW); 
    rcu_periph_clock_enable(RCU_GPIOB);
    rcu_periph_clock_enable(RCU_AF);
    gpio_init(GPIOB, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_14);
}


void pwm_enable(void)
{
     timer_primary_output_config(TIMER0,ENABLE);
     timer_enable(TIMER0);
}

void pwm_disable(void)
{
     timer_primary_output_config(TIMER0,DISABLE);
     timer_disable(TIMER0);
}
static void trigier_timer_config(void)
{
	 timer_oc_parameter_struct timer_ocintpara;
    timer_parameter_struct timer_initpara;

    rcu_periph_clock_enable(RCU_TIMER0);

    timer_deinit(TIMER0);

    /* TIMER0 configuration */
    timer_initpara.prescaler         = 119;
    timer_initpara.alignedmode       = TIMER_COUNTER_EDGE;
    timer_initpara.counterdirection  = TIMER_COUNTER_UP;
    timer_initpara.period            = 99;
    timer_initpara.clockdivision     = TIMER_CKDIV_DIV1;
    timer_initpara.repetitioncounter = 0;
    timer_init(TIMER0,&timer_initpara);

    /* CH0,CH1 and CH2 configuration in PWM mode */
    timer_ocintpara.outputstate  = TIMER_CCX_DISABLE;
    timer_ocintpara.outputnstate = TIMER_CCXN_ENABLE;
    timer_ocintpara.ocpolarity   = TIMER_OC_POLARITY_HIGH;
    timer_ocintpara.ocnpolarity  = TIMER_OCN_POLARITY_LOW;
    timer_ocintpara.ocidlestate  = TIMER_OC_IDLE_STATE_LOW;
    timer_ocintpara.ocnidlestate = TIMER_OCN_IDLE_STATE_LOW;

    timer_channel_output_config(TIMER0,TIMER_CH_1,&timer_ocintpara);

    /* CH0 configuration in PWM mode0,duty cycle 25% */
    timer_channel_output_pulse_value_config(TIMER0,TIMER_CH_1,49);
    timer_channel_output_mode_config(TIMER0,TIMER_CH_1,TIMER_OC_MODE_PWM0);
    timer_channel_output_shadow_config(TIMER0,TIMER_CH_1,TIMER_OC_SHADOW_DISABLE);
	    /* auto-reload preload enable */
    timer_auto_reload_shadow_enable(TIMER0);
    timer_primary_output_config(TIMER0,ENABLE);
    /* auto-reload preload enable */
   // timer_enable(TIMER0);

}

int scr_init(void)
{
    triger_pin_init();
	scr_timer_init();
    trigier_timer_config();
	return 0;
}
INIT_COMPONENT_EXPORT(scr_init);
