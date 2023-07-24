#ifndef __ADC_SAMPLE_H__
#define __ADC_SAMPLE_H__

typedef enum
{
  
    E_U_SCR_RMS=0,
	E_U_LOAD_RMS,
    E_I_RMS,
}RMS_TYPE;
rt_uint32_t get_rms(RMS_TYPE type,uint8_t is_average);

#endif
