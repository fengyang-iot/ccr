#ifndef __PID_APP_H__
#define __PID_APP_H__
#include "rtthread.h"
#include "pid.h"
#include "scr.h"
#include "alarm.h"
//#define ENABLE_PID_DEBUG  //pid 过程数据打印

#define  CTRL_CYCLE_INIT  2 // 2*20ms   40ms  ctrl cycle 为2的倍数
#define  CTRL_CYCLE   1
#define KMAX   2.0f  //1.2f //5.7f 2
#define KMIN   2.6f
#define  PID_KP  0.3f  //5.0f  // 0.3f3
#define  PID_KI  2.0f  // 2.0f //1.0f  //5.0f
#define  PID_KD  0.1f  //0.1f


//#define  PID_KP  0.4f
//#define  PID_KI  0.35f 
//#define  PID_KD  0.4f 
#define  PID_K   KMAX    // 

#define  KP_0  0.3f
#define  KI_0  0.6f
#define  KD_0  0.1f

#define  PID_OUT_MAX  MAX_PASS_ANGLE
#define  PID_OUT_MIN  MIN_PASS_ANGLE

#define  I_LOAD_MAX_LIMIT  700  //7.0A
#define VAL_LIMIT(VAL, MIN, MAX)(\
{\
    typeof(VAL) __val = (VAL);\
    typeof(MIN) __min = (MIN);\
    typeof(MAX) __max = (MAX);\
   (void)(&__val == &__min);\
   (void)(&__val == &__max);\
    __val = (__val < __min)?__min:__val;\
    __val = (__val>__max)?__max:__val;\
})


#define  SAMPLE_SIZE  75
struct  pid_debug
{
    rt_uint16_t data_arr[5][SAMPLE_SIZE];
    rt_uint16_t cnt;
    rt_uint8_t  full_flag;
};

extern rt_uint8_t g_pid_enable; 
extern struct  pid_debug  g_pid_debug_struct;
void save_pid_process_data(void);
void print_pid_precess_data(void);
extern struct PID pid;
extern const float Istd[5];
#endif
