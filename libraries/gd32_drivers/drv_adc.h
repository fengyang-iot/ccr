#ifndef __DRV_ADC_H__
#define __DRV_ADC_H__

#include <board.h>
#include <drivers/adc.h>
#include <math.h>
extern rt_uint32_t tmp_diff_result;
void  get_dma_data(rt_uint16_t * buf);
void cal_rms(rt_uint16_t * cur);
#define  SAMPLES 400 // 250//200

//enum rms_enum
//{
//	VRMS_1 = 0,
//	VRMS_2,
//	IRMS_2
//};


//rt_uint32_t get_rms(enum rms_enum item);
#endif
