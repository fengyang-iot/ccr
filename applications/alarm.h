#ifndef __ALARM_H__
#define __ALARM_H__

#include "database.h"

#define RELAY_CLOSE_DELAY_MS   300

#define MK_PIN GET_PIN(B, 13)
#define BEEP_PIN GET_PIN(A, 6)
#define ALARM_PIN GET_PIN(A, 11)
extern struct rt_semaphore pid_ctrl_sem;
#define RELAY_ON_NO_SET_FALG()  rt_pin_write(MK_PIN, PIN_LOW)

#define RELAY_ON()   do{\
    rt_pin_write(MK_PIN, PIN_LOW);\
    SET_SYS_STATUS_BIT(RELAY_STATUS_POS);\
}while(0)    
#define RELAY_OFF()  do{\
     rt_pin_write(MK_PIN, PIN_HIGH);\
     CLR_SYS_STATUS_BIT(RELAY_STATUS_POS);\
}while(0)
#define BEEP_ON()   rt_pin_write(BEEP_PIN, PIN_LOW)
#define BEEP_OFF()  rt_pin_write(BEEP_PIN, PIN_HIGH)

#define  BM_FAULT_POS                 15   //偏励磁故障
#define  OVER_CUR_POS                 14
#define  STRONG_OVER_CUR_POS          14
//CCR Alarm Code
//#define    NoAlarmcode						0x00
//1.致命故障，不可清除报警
#define    PowerCZmisscode					0x00     //过零故障(Psync Fault)（声光报警）
#define    MagneticBiascode					0x01     //CCR偏励磁报警（声光报警）
#define    SCRFaultcode						0x02     //SCR双向击穿故障（声光报警）
#define    FuseFaultcode					0x03     //FUSE故障（声光报警）
#define    RelayFaultcode					0x04     //运行中接触器抖动/失效（声光报警）
#define    CirOpen100pcode					0x05     //回路开路（声光报警）
#define    CirOverCurcode				    0x06     //过流（光报警）
#define    CirStrongOverCurcode				0x07     //强过流（声光报警）


//2.可清除报警
//#define    CirOverCurcode				    0x0F     //过流（光报警）
#define    IntensityErrcode					0x0E	 //过载引起电流与命令光级不一致（光报警）

#define    CirVADowncode					0x0A     //5%-30%（光报警）,>30%（声光报警）
#define    CirVAUpcode					    0x0B     //5%-30%（光报警）,>30%（声光报警）

#define ACVoltage                  220.0f                                           //V
#define CCRCurrentSeries           6.6f                                             //A

#define   CirOpen5p                3     //Bit3=回路开路5%  -  30%
#define   CirOpen30p               2     //Bit2=回路开路30% - 100%
#define   CirClose5p               1     //Bit1=回路短路5%  -  30%
#define   CirClose30p              0     //Bit0=回路短路30% - 100%

#define   CirVAUpWarn              0.05  //Bit3=回路开路5%  - 30%
#define   CirVAUpAlarm             0.3   //Bit2=回路开路30% - 100%
#define   CirVADownWarn            0.05  //Bit1=回路短路5%  - 30%
#define   CirVADownAlarm           0.3   //Bit0=回路短路30% - 100%
//CCR  FatalAlarm
#define SCRFaultAlarmValue         ACVoltage * 0.65f*10			    //Unit 0.1V
#define FuseFaultAlarmValue        ACVoltage * 0.2f*10				//Unit 0.1V
#define CirOpenVoltageValue        ACVoltage * 0.5f*10				//Unit 0.1V
#define CirOpenCurrentValue        CCRCurrentSeries * 0.15f*100  	//Unit 0.01A
#define OverCurAlarmValue          CCRCurrentSeries * 1.05f*100 	//5%,Unit 0.01A
#define StrongOverCurAlarmValue    CCRCurrentSeries * 1.25f*100		//25%,Unit 0.01A



extern const char* err_string[];
void print_err_string(void);
extern void check_i_load(float Iload);
//extern void check_scr_voltage(float Vscr);
//extern void check_load_voltage(float Vload);
extern void Check_SCR(float rUrms);
extern void Check_Fuse(float rUrms,float tint0);
extern void Check_CircuitOpen(float rUrms,float rIrms);
#endif





