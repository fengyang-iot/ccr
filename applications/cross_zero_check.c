#include "cross_zero_check.h"
#include "drv_gpio.h"
#include "database.h"
#include  "scr.h"
#include "pid_app.h"
#include "drv_adc.h"
#include "adc_sample.h"
#include "alarm.h"
#define  DBG_TAG  "CROSS_ZERO_CHECK"
#define  DBG_LVL  DBG_LOG

#include "rtdbg.h"

#define   CROSS_ZERO_INT_PIN  GET_PIN(A, 5)

#define CROSS_ZERO_STACK_SIZE   512
#define CROSS_ZERO_PRIORITY     9

rt_device_t  g_single_stable_timer;

extern rt_uint8_t  g_ctrl_cycle;
extern struct rt_semaphore pid_ctrl_sem;
typedef rt_err_t (*time_out_cb)(rt_device_t  dev, rt_size_t size);

enum
{
	TIMER_CROSS_ZERO = 0,  		//过零检测定时器
	TIMER_SCR_triger_angle_delay,	   //导通角延时定时器
	TIMER_TRGER,       		  // 触发时长定时器
};

#define SINGLE_STABLE_TIMEOUT  9500

//启动定时器

static void single_stable_timer_start(void )
{
    rt_hwtimerval_t timeout;
    timeout.sec = 0;
    timeout.usec = SINGLE_STABLE_TIMEOUT;
	rt_device_write(g_single_stable_timer,0,&timeout, sizeof(timeout));
}
// 关闭定时器
static void single_stable_timer_stop()
{
    
	rt_device_control(g_single_stable_timer, HWTIMER_CTRL_STOP, 0);
}

//回调函数
rt_err_t single_stable_timer_cb(rt_device_t  dev, rt_size_t size)
{
    single_stable_timer_stop();
    DISABLE_SCR_TRIGER();
    return 0;
}

struct cross_zero g_cross_zero;


rt_bool_t Is_cross_zero_valid(void)
{
	return g_cross_zero.int0_valid;
}

// 硬件定时器0,计算过零周期
static void cross_zero_timer_start()
{
	rt_device_write(g_cross_zero.timer,0,&g_cross_zero.timeout_s, sizeof(g_cross_zero.timeout_s));
}
static void cross_zero_timer_stop()
{
	rt_device_control(g_cross_zero.timer, HWTIMER_CTRL_STOP, 0);
}
rt_err_t timer5_cb(rt_device_t  dev, rt_size_t size)
{
	cross_zero_timer_stop();
    g_cross_zero.int0_valid = RT_FALSE;
	return 0;
}

static void cross_zero_timer_init()
{
    rt_hwtimer_mode_t mode = HWTIMER_MODE_ONESHOT;
    g_cross_zero.timer = rt_device_find("timer5");
    if (RT_NULL == g_cross_zero.timer)
    {
        LOG_E("cannot find timer5");
		return;
    }
	//1Mhz up 
	rt_device_open(g_cross_zero.timer, RT_DEVICE_FLAG_RDWR);
	
	rt_device_set_rx_indicate(g_cross_zero.timer, timer5_cb);
		

	rt_device_control(g_cross_zero.timer, HWTIMER_CTRL_MODE_SET, &mode);
    NVIC_SetPriority(TIMER5_IRQn, 5);
	
	g_cross_zero.timeout_s.sec = 0;
	g_cross_zero.timeout_s.usec = (rt_int32_t)(PPeriod*1000.0f);

}



float	 gCCRInt0Time;

rt_uint8_t gInt0ValidFlag;


/** 只限制了频率最大值，小于最大值并且不在频率范围内，使用50hz*/
static void cross_zero_period_check(rt_hwtimerval_t * timer_val)
{
	 float ActualTpInt0;
	 rt_uint16_t freq; //0.1hz
	

	
	 ActualTpInt0 = timer_val->usec/1000.0f; 
	 freq = (rt_uint16_t)(1.0f/ActualTpInt0 * 10000 / 2.0f);
	
	
	if (ActualTpInt0 >= LowCZWarnTime)
	{
		//g_cross_zero.int0_valid =RT_TRUE;
		g_cross_zero.stable_cnt++;
		if(g_cross_zero.stable_cnt >= Int0StableNum)
		{
			g_cross_zero.stable_cnt = Int0StableNum;
			g_cross_zero.int0_valid = RT_TRUE;
		}
		if ((ActualTpInt0 >= NPeriod)&&(ActualTpInt0 <= PPeriod))
		{
			//gCCRInt0Time  =ActualTpInt0;
			update_freq(freq);
			g_ccr_data.ccr_info.half_period = timer_val->usec;
            
            rt_device_control(g_scr_dev.triger_angle_timer, HWTIMER_CTRL_STOP, 0);
             //关闭触发信号
            DISABLE_SCR_TRIGER();
		} 
		else
		{
			LOG_E("freq out of range, freq is %d", freq);
			//update_freq(ACFreq);
			
			//gCCRInt0Time  =StdInt0;
		}
	} 
	else
	{
		g_cross_zero.stable_cnt = 0;
		g_cross_zero.int0_valid	= RT_FALSE;
		g_ctrl_cycle = CTRL_CYCLE;
	}	
}

rt_uint16_t g_current_view[20];
rt_uint16_t g_current_count = 0;
rt_uint16_t cur_buf[SAMPLES*3];
static void cross_zero_check_entry(void *param)
{
	rt_hwtimerval_t timer_val;
    rt_uint16_t iload_rms;
	static rt_uint8_t first_run = 1;
    static rt_uint8_t  even_flag = 0;
    static  rt_uint8_t g_ctrl_cycle = CTRL_CYCLE_INIT;
    if (first_run)
    {
        cross_zero_timer_start();
        first_run = 0;
    }
    else
    {
        cross_zero_timer_stop();
        rt_device_read(g_cross_zero.timer, 0, &timer_val, sizeof(timer_val));
        cross_zero_timer_start();        
        cross_zero_period_check(&timer_val);
        if(g_cross_zero.int0_valid)
        {
            timer_enable(TIMER1); 
            if(RELAY_STATUS_ON == GET_SYS_STATUS_BIT(RELAY_STATUS_POS))
            {
                //开启1ms定时器
                rt_device_write(g_scr_dev.timer_1ms,0,&g_scr_dev.timer_1ms_delay, sizeof(g_scr_dev.timer_1ms_delay));
                
                //开启单稳定时器
                single_stable_timer_start();
                
                if(even_flag)
                {
                     BEEP_ON();
                    even_flag = 0;
                    timer_disable(TIMER1); //关闭adc采样定时器
                    //读出电流瞬时值数据
                    get_dma_data(cur_buf);
                    //开启ADC采样定时器,启动dma输出采集
                     timer_enable(TIMER1); 
                    //计算电流有效值
                     cal_rms(cur_buf);
                   //  update_scr_voltage(get_rms(E_U_SCR_RMS));
                    //计算pid控制值
                    g_ctrl_cycle--;
                    if(g_ctrl_cycle ==0)
                    {
                        g_ctrl_cycle  = CTRL_CYCLE;            
                        if(g_pid_enable)
                        { 
                            if(g_ccr_data.ccr_info.intensity > 0)
                            {
                                pid_ref_set(&pid,Istd[g_ccr_data.ccr_info.intensity-1]);
                            }
                            else
                            {
                                 pid_ref_set(&pid,0);
                            }
                    
                           // iload_rms = get_i_load();  
                            iload_rms =  get_rms(E_I_RMS, 0);                    
                            g_current_view[g_current_count] = iload_rms;
                            g_current_count++;
                            if(g_current_count >=20)
                            {
                                g_current_count = 0;
                            }
                            
                            pid_calc(&pid, iload_rms/100.0f); 
                            update_pass_angle(pid.out);
                        
                            if(( get_i_load() <= 100) && (g_ccr_data.ccr_info.intensity == 0))
                            {
                                RELAY_OFF();
                            }
                            save_pid_process_data();
                            
                            BEEP_OFF();
                        }
                    }
                   
                }
                else
                {
                    even_flag = 1;
                }
        }
						
		}	
	}
}

/**过零中断回调函数*/
static void cross_zero_iqr_cb(void * para)
{
    
    cross_zero_check_entry(0);
	//rt_sem_release(g_cross_zero.int0_sem);	
}

static int cross_zero_check_init()
{

	cross_zero_timer_init();
//	g_cross_zero.int0_sem = rt_sem_create("int0_sem", 0, RT_IPC_FLAG_FIFO);
//	if(g_cross_zero.int0_sem == RT_NULL)
//	{
//		LOG_E("rt_sem_create int0_sem failed");
//	}
//	
//	g_cross_zero.int0_thread = rt_thread_create("cross_zero_check", cross_zero_check_entry, 0, CROSS_ZERO_STACK_SIZE, CROSS_ZERO_PRIORITY,10); 
//	if ( g_cross_zero.int0_thread == RT_NULL)
//	{
//		LOG_E("create cross_zero thread failed ");
//	}	
//	rt_thread_startup(g_cross_zero.int0_thread);
	rt_pin_mode(CROSS_ZERO_INT_PIN, PIN_MODE_INPUT_PULLUP);
    rt_pin_attach_irq(CROSS_ZERO_INT_PIN, PIN_IRQ_MODE_FALLING, cross_zero_iqr_cb, 0 );
	rt_pin_irq_enable(CROSS_ZERO_INT_PIN, PIN_IRQ_ENABLE);
	return 0;
}
INIT_APP_EXPORT(cross_zero_check_init);






//软件单稳定时器  9ms
static int  single_stable_timer_init()
{
    rt_hwtimer_mode_t mode = HWTIMER_MODE_ONESHOT;
     g_single_stable_timer = rt_device_find("timer6");
    if (RT_NULL == g_single_stable_timer)
    {
        LOG_E("cannot find timer6");
		return 0;
    }
	rt_err_t result;
	result = rt_device_open(g_single_stable_timer, RT_DEVICE_FLAG_RDWR);
    if(result != RT_EOK)
    {
        LOG_E(" open g_single_stable_timer failed!");
    }
	
	rt_device_set_rx_indicate(g_single_stable_timer, single_stable_timer_cb);
		

	rt_device_control(g_single_stable_timer, HWTIMER_CTRL_MODE_SET, &mode);
    NVIC_SetPriority(TIMER6_IRQn, 6);
    
    return 0;
	
}
INIT_APP_EXPORT(single_stable_timer_init);
