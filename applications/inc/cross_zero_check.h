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


#define    Int0StableNum                   20
#endif


