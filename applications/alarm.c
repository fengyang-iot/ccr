#include "alarm.h"
#include "board.h"
#include "database.h"
#include "cross_zero_check.h"

#define DBG_TAG  "alarm.c"
#define DBG_LVL		DBG_INFO
#include "rtdbg.h"

#define ALARM_THREAD_STACK_SIZE  512
#define ALARM_THREAD_PRIORITY    8
#define ALARM_THREAD_TICK	     10


ALIGN(RT_ALIGN_SIZE)
struct rt_event alarm_event;

extern rt_uint32_t tmp_diff_result;
struct rt_thread  alarm_thread;
rt_uint8_t alarm_statck[ALARM_THREAD_STACK_SIZE];
extern void check_i_load(float Iload);
extern void check_scr_voltage(float Vscr);
extern void check_load_voltage(float Vload);





//#define    PowerCZmisscode					0x00     //过零故障(Psync Fault)（声光报警）
//#define    MagneticBiascode					0x01     //CCR偏励磁报警（声光报警）
//#define    SCRFaultcode						0x02     //SCR双向击穿故障（声光报警）
//#define    FuseFaultcode					0x03     //FUSE故障（声光报警）
//#define    RelayFaultcode					0x04     //运行中接触器抖动/失效（声光报警）
//#define    CirOpen100pcode					0x05     //回路开路（声光报警）
//#define    CirStrongOverCurcode				0x06     //强过流（声光报警）
const char* err_string[] = {
    "cross_zero error!",
    "maganizim error",
    "SCR Fault",
    "Fuse Error",
    "Relay Error",
    "Circuit Open Error",
    "Over Current Error",
    "Strong Over Current",
};



//可控硅故障检测
void Check_SCR(float rUrms)
{
    if(GET_ALARM_STATUS_BIT(SCRFaultcode))
        return;
	if ((g_ccr_data.ccr_info.angle <= MIN_PASS_ANGLE) && (rUrms >= SCRFaultAlarmValue))
	{      
        SET_ALARM_STATUS_BIT(SCRFaultcode);
	}
}

//熔断器故障检测
void Check_Fuse(float rUrms,float tint0)
{
    static rt_uint8_t count = 0;
    if(g_cross_zero.int0_valid== RT_FALSE)
        return;
   if(GET_ALARM_STATUS_BIT(FuseFaultcode))
        return;
	if ((g_ccr_data.ccr_info.angle >=(0.5f * MaxPassAngle(tint0))) && (rUrms <= FuseFaultAlarmValue))
	{
        
        SET_ALARM_STATUS_BIT(FuseFaultcode);
        count++;
        if(count>=75)
        {
            count = 0;
            SET_ALARM_STATUS_BIT(CirOpen100pcode);
        }
	}
    else
    {
        count = 0;
    }
}

//回路开路检测
void Check_CircuitOpen(float rUrms,float rIrms)
{
    static rt_uint8_t count = 0;
    if(g_cross_zero.int0_valid== RT_FALSE)
        return;
	if ((rIrms<=CirOpenCurrentValue) && (rUrms >=CirOpenVoltageValue))
	{
        count++;
        if(count>=75)
        {
            count = 0;
            SET_ALARM_STATUS_BIT(CirOpen100pcode);
        }

	}
    else
    {
        count = 0;
    }
}

//void Check_Circuit(float rUrms,float rIrms,rt_uint8_t Intensity)
//{
//	float ActualVA_Value,StdVA_Value,nk;    
//	
//	ActualVA_Value	=rUrms * rIrms;	
//	StdVA_Value		=Read_eepromFloat(CCRStdEepromirstAddr[Intensity]);
//	
//	if (ActualVA_Value >= StdVA_Value)
//	{   
//		//Cir Open Warn or Alarm
//		nk = (ActualVA_Value - StdVA_Value)/StdVA_Value;
//		
//		if ((nk >= CirVAUpWarn )&& (nk < CirVAUpAlarm ))
//		{
//			Set_CirAlarmRegBIT(CirOpen5p);
//			Clr_CirAlarmRegBIT(CirOpen30p);
//			gCCRAlarmCodeReg=CirVAUpcode;
//			ClrBeep();
//		} 
//		else if (nk <= CirVAUpAlarm)
//		{
//		    Set_CirAlarmRegBIT(CirOpen30p);	
//			Clr_CirAlarmRegBIT(CirOpen5p);
//			gCCRAlarmCodeReg=CirVAUpcode;
//			SetBeep();			
//		}
//		else
//		{
//			Clr_CirAlarmRegBIT(CirOpen5p);
//			Clr_CirAlarmRegBIT(CirOpen30p);
//			gCCRAlarmCodeReg=NoAlarmcode;
//			ClrBeep();			
//		}
//	}
//	else
//	{   //Cir Close Warn or Alarm
//		nk = (StdVA_Value - ActualVA_Value)/StdVA_Value;
//		
//		if ((nk >= CirVADownWarn)&&(nk < CirVADownAlarm))
//		{
//			Set_CirAlarmRegBIT(CirClose5p);
//			Clr_CirAlarmRegBIT(CirClose30p);
//			gCCRAlarmCodeReg=CirVAUpcode;
//			ClrBeep();
//		} 
//		else if (nk >= CirVADownAlarm)
//		{
//			Set_CirAlarmRegBIT(CirClose30p);
//			Clr_CirAlarmRegBIT(CirClose5p);
//			gCCRAlarmCodeReg=CirVAUpcode;
//			SetBeep();
//		}
//		else
//		{
//			Clr_CirAlarmRegBIT(CirClose5p);
//			Clr_CirAlarmRegBIT(CirClose30p);
//			gCCRAlarmCodeReg=NoAlarmcode;
//			ClrBeep();
//		}		
//	}	
//}
extern rt_uint8_t g_over_cur_flag;
#define OverCurNum 200    //20*200  4s
#define StrongOverNum  40  //20*40  800ms
//电流保护
void check_i_load(float rIrms)
{

    static rt_uint8_t gOverCurWaitCTC = 0;    
    static rt_uint8_t strongOverCnt= 0;
    if ((rIrms >= OverCurAlarmValue) && (rIrms < StrongOverCurAlarmValue) && !GET_ALARM_STATUS_BIT(CirOverCurcode))
	{
        gOverCurWaitCTC++;
        if (gOverCurWaitCTC >= OverCurNum)
        {
            gOverCurWaitCTC=0;
            SET_ALARM_STATUS_BIT(CirOverCurcode);
            
           // ClrBeep();
        }

	}
	else if(rIrms >= StrongOverCurAlarmValue && !GET_ALARM_STATUS_BIT(CirStrongOverCurcode))
	{  
        strongOverCnt++;
        if(gOverCurWaitCTC >= StrongOverNum)
        {  
            SET_ALARM_STATUS_BIT(CirStrongOverCurcode);
        }
	}
	else
	{
		gOverCurWaitCTC = 0;
        strongOverCnt = 0;
	}
}


void print_err_string(void)
{
    int len = sizeof(err_string)/sizeof(char *);
    rt_uint16_t err_status = SYS_ALARM_REG;
    LOG_E("=====  Error status is %X ========", err_status);
    for(int i=0; i<len; i++)
    {
        if(err_status & (1<<i))
        {
            LOG_E("!=== %s ======!", err_string[i]);
        }
    }
    LOG_E("angle: %d\ni_load: %d.%d\nv_scr:%d.%d\n", (rt_uint16_t)g_ccr_data.ccr_info.angle,
          g_ccr_data.ccr_info.i_load/100, g_ccr_data.ccr_info.i_load%100, 
          g_ccr_data.ccr_info.v_scr/10, g_ccr_data.ccr_info.v_scr%10);
}

//static void alarm_thread_entry(void *pa

//	rt_uint32_t e;
//	while(1)
//	{

//			if(& 0x01)  //有故障
//			{
//				
//				RELAY_OFF();
//				CLR_SYS_STATUS_BIT(RELAY_STATUS_POS);
//				SET_SYS_STATUS_BIT(BM_FAULT_POS);
//				LOG_E("bm fuult! diff_result = %d", tmp_diff_result);
//			}
//		}
//	}
//}

static int alarm_init(void)
{
	//rt_err_t res;
	//beep_off
	rt_pin_mode(BEEP_PIN, PIN_MODE_OUTPUT);
	rt_pin_write(BEEP_PIN, PIN_HIGH);
	
	//relay off
	rt_pin_mode(MK_PIN, PIN_MODE_OUTPUT);
	rt_pin_write(MK_PIN, PIN_HIGH);
	
//	//alarm event init
//	rt_event_init(&alarm_event, "alarm event", RT_IPC_FLAG_FIFO);
//	
//	res = rt_thread_init(&alarm_thread, "alarm_thread", alarm_thread_entry, 0,
//					&alarm_statck[0], ALARM_THREAD_STACK_SIZE, 
//					ALARM_THREAD_PRIORITY,
//					ALARM_THREAD_TICK);
//	if(RT_EOK != res)
//	{
//		LOG_E("alarm thread init failed!");
//	}
//	rt_thread_startup(&alarm_thread);
//	
	return 0;
}

INIT_DEVICE_EXPORT(alarm_init);

