#ifndef __CROSS_ZERO_CHECK__
#define __CROSS_ZERO_CHECK__

#include "rtthread.h"
#include "rtdevice.h"

#define ACFreq                     50.0f                                            //Hz
#define ACVoltage                  220.0f                                           //V
#define CCRCurrentSeries           6.6f                                             //A

#define G1NoLoadPassAngle          9.0f										        //单位：度

#define StdInt0                      (float)(1000.0f/(2.0f*ACFreq))                   //10ms 

//#define NFreqBias                  (float)(StdInt0 * 0.95f)                          //50Hz-5%
//#define PFreqBias                  (float)(StdInt0 * 1.05f)                          //50HZ+5%

#define NFreqBias                   (float)(ACFreq*2.0f * 0.95f)                          //50Hz-5%
#define PPeriod					 	(float)(1000.0f/NFreqBias)
#define PFreqBias                  	(float)(ACFreq*2.0f * 1.05f)                          //50HZ+5%
#define NPeriod						(float)(1000.0f/PFreqBias)							//ms
/**** ***********?**********/	
#define FreqMeasureBiasTime        0.15f                                             //unit:ms  =0.15ms


#define  SafeTimeBias               0.1f                        //0.1ms
//#define  LowCZWarnTime             (float)(NFreqBias - 2.0f * SafeTimeBias)          //unit:ms  =9.3ms
#define    LowCZWarnTime             (float)( NPeriod - 2.0f * SafeTimeBias)           //
//#define  HighCZWarnTime            (float)(PFreqBias)                                //unit:ms  =10.5ms
#define    HighCZWarnTime 			 (float)(PPeriod)


#define    Int0StableNum              20
#define    Tw                         1.0f
//LowCZWarnTime < maxTrigTime < HighCZWarnTime
#define  minTrigTime               (float)(Tw + SafeTimeBias)                           //unit:ms  =1ms + 0.1ms=1.1ms

//根据tint0=gCCRInt0Time，实时确定MaxPassAngle对应的SCR最大导通时间
static inline float MaxPassTime(float tint0)
{
	return (float)(tint0 - minTrigTime*1000);
}

#define  MinPassTime               (float)(SafeSCRTrigWidth)   //                         

static inline float MaxPassAngle(float tint0)
{
	return (float)(180.0f * MaxPassTime(tint0)/ tint0); 
}
typedef struct cross_zero
{
	rt_device_t  	timer;   
	rt_hwtimerval_t timeout_s;      /* 定时器超时值 */
	rt_sem_t  		int0_sem;
	rt_thread_t     int0_thread;
	rt_uint8_t      stable_cnt;  //int0连续有效计数
	rt_uint8_t      int0_valid;   //int0有效	
}* cross_zero_t;

extern struct cross_zero g_cross_zero;
rt_bool_t Is_cross_zero_valid(void);
#endif


