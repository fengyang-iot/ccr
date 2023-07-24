/*
 * Copyright (c) 2006-2022, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author            Notes
 * 2021-02-25     iysheng           first version
 */


#include "drv_adc.h"
#define DBG_TAG             "drv.adc"
#define DBG_LVL             DBG_INFO
#include "database.h"
#include "alarm.h"
#include <rtdbg.h>
#ifdef RT_USING_ADC

#define MAX_EXTERN_ADC_CHANNEL    16

extern struct rt_event alarm_event;
typedef struct {
    struct rt_adc_device adc_dev;
    char name[8];
    rt_base_t adc_pins[16];
    void *private_data;
    void  * dma_buffer;   
} gd32_adc_device;

static gd32_adc_device g_gd32_devs[] = {
#ifdef BSP_USING_ADC0
    {
        {},
        "adc0",
        {
            GET_PIN(A, 0), GET_PIN(A, 1), GET_PIN(A, 2), GET_PIN(A, 3),
            GET_PIN(A, 4), GET_PIN(A, 5), GET_PIN(A, 6), GET_PIN(A, 7),
            GET_PIN(B, 0), GET_PIN(B, 1), GET_PIN(C, 0), GET_PIN(C, 1),
            GET_PIN(C, 2), GET_PIN(C, 3), GET_PIN(C, 4), GET_PIN(C, 5),
        },
        (void *)ADC0,
		RT_NULL,
    },
#endif

#ifdef BSP_USING_ADC1
    {
        {},
        "adc1",
        {
            GET_PIN(A, 0), GET_PIN(A, 1), GET_PIN(A, 2), GET_PIN(A, 3),
            GET_PIN(A, 4), GET_PIN(A, 5), GET_PIN(A, 6), GET_PIN(A, 7),
            GET_PIN(B, 0), GET_PIN(B, 1), GET_PIN(C, 0), GET_PIN(C, 1),
            GET_PIN(C, 2), GET_PIN(C, 3), GET_PIN(C, 4), GET_PIN(C, 5),
        },
        (void *)ADC1,
		RT_NULL,
    },
#endif
};


ALIGN(4)
rt_uint16_t adc_value[SAMPLES*3];

//rt_uint32_t  out_rms[3];
rt_uint32_t rms[3];

// x/4095 *3.3 = 1.5 --> x = 1861
// x/4095 *3.3 = 1.246 --> x = 1546
#define VREFCONST 1861
#define BM_THRESHOLD  150
//#define BM_BIAS_THRESHOLD  
rt_uint32_t tmp_diff_result;
void cal_rms(rt_uint16_t * cur)
{
	
    rt_uint32_t sample_val[3][2]; // 第一列存储正半周的值，第二列负半周值
    rt_uint32_t diff_max = 0;
    rt_uint16_t  cnt_p_n[3][2];


	rt_int32_t tmp;
	rt_uint32_t tmp_square;
	rt_uint32_t tmp_diff = 0;
	static rt_uint8_t bm_cnt = 0;
	rt_memset(sample_val, 0, sizeof(sample_val));
	rt_memset(cnt_p_n, 0, sizeof(cnt_p_n));
	//rt_memset(diff_max, 0, sizeof(diff_max));
	for(int i = 0; i<SAMPLES*3; i+=3)
	{
		for(int j = 0; j<3; j++)
		{
			tmp = (cur[i+j]-VREFCONST);
			tmp_square = tmp*tmp;
			if(tmp >0)
			{
				sample_val[j][0] += tmp_square;
				cnt_p_n[j][0]++;
			}
			else
			{
				sample_val[j][1] += tmp_square;
				cnt_p_n[j][1]++;
			}
		}		
	}

	for(int i = 0 ; i<3; i++)
	{

		rms[i]  = (sample_val[i][0] + sample_val[i][1])/SAMPLES;
		rms[i]  = sqrt(rms[i]);
		//rms[i] = rms[i]/4095.0f * 3.3 *100;	
	}
		if(sample_val[1][0] > sample_val[1][1])
		{
			tmp_diff = (sample_val[1][0] - sample_val[1][1]);
		}
		else
		{
			tmp_diff = (sample_val[1][1] - sample_val[1][0]);
		}

	    tmp_diff = tmp_diff/SAMPLES;
		tmp_diff_result = sqrt(tmp_diff);
		if(tmp_diff_result > diff_max)
		{
			diff_max= tmp_diff_result;
		}
		if(GET_SYS_STATUS_BIT(RELAY_STATUS_POS) && (tmp_diff_result > BM_THRESHOLD))
		{
			bm_cnt++;
			if(bm_cnt >= 4)
			{
            //    SET_ALARM_STATUS_BIT(MagneticBiascode);
			//	rt_event_send(&alarm_event, 0x01);
				bm_cnt = 0;
			}
		}
		else
		{
			bm_cnt = 0;
		}
	//rt_enter_critical();
	//rt_memcpy((rt_uint8_t *)out_rms, (rt_uint8_t*)rms, sizeof(rms));
	//rt_exit_critical();
}


//static rt_uint32_t get_rms(enum rms_enum item)
//{
//	RT_ASSERT(item <=2);
//	return(out_rms[item]);
//}
//static rt_uint8_t flag = 1;
//void DMA0_Channel0_IRQHandler(void)
//{
//	if(SET == dma_interrupt_flag_get(DMA0,DMA_CH0,DMA_INTF_FTFIF))
//	{
//		dma_interrupt_flag_clear(DMA0,DMA_CH0,DMA_INTF_FTFIF);
//		flag = !flag;
//		timer_disable(TIMER1);
//		cal_rms(adc_value);
//		timer_enable(TIMER1);
//	}
//	
//}
/*
 * static void init_pin4adc
 *
 * @ rt_uint32_t pin: pin information
 * return: N/A
 */
static void init_pin4adc(rt_base_t pin)
{
	gpio_init(PIN_GDPORT(pin), GPIO_MODE_AIN, GPIO_OSPEED_50MHZ, PIN_GDPIN(pin));
}



static void dma_config(void)
{
	/* ADC_DMA_channel configuration */
    dma_parameter_struct dma_data_parameter;
    rcu_periph_clock_enable(RCU_DMA0);  
    /* ADC DMA_channel configuration */
    dma_deinit(DMA0, DMA_CH0);
    
    /* initialize DMA single data mode */
    dma_data_parameter.periph_addr = (uint32_t)(&ADC_RDATA(ADC0));
    dma_data_parameter.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
    dma_data_parameter.memory_addr = (uint32_t)(&adc_value[0]);
    dma_data_parameter.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
    dma_data_parameter.periph_width = DMA_PERIPHERAL_WIDTH_16BIT;
    dma_data_parameter.memory_width = DMA_MEMORY_WIDTH_16BIT;  
    dma_data_parameter.direction = DMA_PERIPHERAL_TO_MEMORY;
    dma_data_parameter.number = 3*SAMPLES;
    dma_data_parameter.priority = DMA_PRIORITY_HIGH;
    dma_init(DMA0, DMA_CH0, &dma_data_parameter);
	//dma_interrupt_enable(DMA0,DMA_CH0, DMA_INT_FTF);
//	NVIC_SetPriority(DMA0_Channel0_IRQn,6);
//	NVIC_EnableIRQ(DMA0_Channel0_IRQn);
    dma_circulation_enable(DMA0, DMA_CH0);
  
    /* enable DMA channel */
    dma_channel_enable(DMA0, DMA_CH0);
}
/*
    TIMER1 configuration: generate 3 PWM signals with 3 different duty cycles:
    TIMER1CLK = SystemCoreClock / 120 = 1MHz

    TIMER1 channel0 duty cycle = (4000/ 16000)* 100  = 25%
*/
static void timer_config(void)
{
	 timer_oc_parameter_struct timer_ocintpara;
    timer_parameter_struct timer_initpara;

    rcu_periph_clock_enable(RCU_TIMER1);

    timer_deinit(TIMER1);

    /* TIMER1 configuration */
    timer_initpara.prescaler         = 119;
    timer_initpara.alignedmode       = TIMER_COUNTER_EDGE;
    timer_initpara.counterdirection  = TIMER_COUNTER_UP;
    timer_initpara.period            = 49;//79;
    timer_initpara.clockdivision     = TIMER_CKDIV_DIV1;
    timer_initpara.repetitioncounter = 0;
    timer_init(TIMER1,&timer_initpara);

    /* CH0,CH1 and CH2 configuration in PWM mode */
    timer_ocintpara.outputstate  = TIMER_CCX_ENABLE;
    timer_ocintpara.outputnstate = TIMER_CCXN_DISABLE;
    timer_ocintpara.ocpolarity   = TIMER_OC_POLARITY_HIGH;
    timer_ocintpara.ocnpolarity  = TIMER_OCN_POLARITY_HIGH;
    timer_ocintpara.ocidlestate  = TIMER_OC_IDLE_STATE_LOW;
    timer_ocintpara.ocnidlestate = TIMER_OCN_IDLE_STATE_LOW;

    timer_channel_output_config(TIMER1,TIMER_CH_1,&timer_ocintpara);

    /* CH0 configuration in PWM mode0,duty cycle 25% */
    timer_channel_output_pulse_value_config(TIMER1,TIMER_CH_1,24);
    timer_channel_output_mode_config(TIMER1,TIMER_CH_1,TIMER_OC_MODE_PWM0);
    timer_channel_output_shadow_config(TIMER1,TIMER_CH_1,TIMER_OC_SHADOW_DISABLE);
	    /* auto-reload preload enable */
    timer_auto_reload_shadow_enable(TIMER1);
    /* auto-reload preload enable */
   // timer_enable(TIMER1);

}

static rt_err_t gd32_adc_enabled(struct rt_adc_device *device, rt_uint32_t channel, rt_bool_t enabled)
{
    uint32_t adc_periph;
    gd32_adc_device * gd32_adc = (gd32_adc_device *)device;

    if (channel >= MAX_EXTERN_ADC_CHANNEL)
    {
        LOG_E("invalid channel");
        return -RT_EINVAL;
    }

    adc_periph = (uint32_t )(device->parent.user_data);

    if (enabled == ENABLE)
    {
        init_pin4adc(gd32_adc->adc_pins[channel]);
		init_pin4adc(gd32_adc->adc_pins[channel+1]);
		init_pin4adc(gd32_adc->adc_pins[channel+2]);
		//config dma
		dma_config();
		timer_config();
        adc_deinit(adc_periph);
		adc_special_function_config(adc_periph,ADC_CONTINUOUS_MODE,DISABLE);
		adc_special_function_config(adc_periph,ADC_SCAN_MODE,ENABLE);
		adc_mode_config(ADC_MODE_FREE); 
        adc_channel_length_config(adc_periph, ADC_REGULAR_CHANNEL, 3);
        adc_data_alignment_config(adc_periph, ADC_DATAALIGN_RIGHT);
        adc_external_trigger_source_config(adc_periph, ADC_REGULAR_CHANNEL, ADC0_1_EXTTRIG_REGULAR_T1_CH1);
        adc_external_trigger_config(adc_periph, ADC_REGULAR_CHANNEL, ENABLE);
		
		//enable ADC dma
		adc_dma_mode_enable(adc_periph);
        adc_regular_channel_config(adc_periph, 0, channel,  ADC_SAMPLETIME_13POINT5);
		adc_regular_channel_config(adc_periph, 1, channel+1, ADC_SAMPLETIME_13POINT5);
		adc_regular_channel_config(adc_periph, 2, channel+2, ADC_SAMPLETIME_13POINT5);
        adc_enable(adc_periph);
    }
    else
    {
        adc_disable(adc_periph);
    }

    return 0;
}

static rt_err_t gd32_adc_convert(struct rt_adc_device *device, rt_uint32_t channel, rt_uint32_t *value)
{
   // uint32_t adc_periph;

    if (!value)
    {
        LOG_E("invalid param");
        return -RT_EINVAL;
    }

//    adc_periph = (uint32_t )(device->parent.user_data);
 //   adc_software_trigger_enable(adc_periph, ADC_REGULAR_CHANNEL);
//	while(! adc_flag_get(adc_periph, ADC_FLAG_EOC));
//	rt_uint16_t *p = &adc_value[0][0];
////	for(int i= 0; i<3; i++)
////	{
////		*p++ = adc_regular_data_read(adc_periph);
////	}
//	 adc_discontinuous_mode_config(ADC0, ADC_REGULAR_CHANNEL, 3);
  //  *value = adc_regular_data_read(adc_periph);
	*value = rms[channel];
    return 0;
}

static struct rt_adc_ops g_gd32_adc_ops = {
    gd32_adc_enabled,
    gd32_adc_convert,
};



static int rt_hw_adc_init(void)
{
    int ret, i = 0;

#ifdef BSP_USING_ADC0
    rcu_periph_clock_enable(RCU_ADC0);
#endif

#ifdef BSP_USING_ADC1
    rcu_periph_clock_enable(RCU_ADC1);
#endif

    for (; i < sizeof(g_gd32_devs) / sizeof(g_gd32_devs[0]); i++)
    {
        ret = rt_hw_adc_register(&g_gd32_devs[i].adc_dev, \
            (const char *)g_gd32_devs[i].name, \
            &g_gd32_adc_ops, (void *)g_gd32_devs[i].private_data);
        if (ret != RT_EOK)
        {
            /* TODO err handler */
            LOG_E("failed register %s, err=%d", g_gd32_devs[i].name, ret);
        }
    }

    return ret;
}
INIT_BOARD_EXPORT(rt_hw_adc_init);
#endif


void  get_dma_data(rt_uint16_t * buf)
{
    rt_memcpy(buf, adc_value, SAMPLES*3*2);
    dma_config();
    
}
