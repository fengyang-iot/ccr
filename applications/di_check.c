#include "rtthread.h"
#include "board.h"
#include "alarm.h"
#include "i2c_app.h"
#include "pid_app.h"

#define DBG_TAG  "di_check"
#define DBG_LVL  DBG_INFO
#include "rtdbg.h"

#define  RELAY_STATE_PIN GET_PIN(B, 12)
#define  R_L_PIN  		GET_PIN(A, 12)
#define  G0_PIN	  		GET_PIN(A, 15)
#define  G1_PIN	  		GET_PIN(B, 3)
#define  G2_PIN	  		GET_PIN(B, 4)
#define  G3_PIN	  		GET_PIN(B, 5)
#define  G4_PIN	  		GET_PIN(B, 6)
#define  G5_PIN	  		GET_PIN(B, 7)


#define DI_VALID(DI_PIN)  (rt_pin_read(DI_PIN) == PIN_LOW)


#define DI_THREAD_STACK_SIZE  1024
#define DI_THREAD_PRIORITY     12
#define DI_THREAD_TICK         10

ALIGN(RT_ALIGN_SIZE)
struct rt_thread  di_thread;
rt_uint8_t di_stack[DI_THREAD_STACK_SIZE];



static void di_thread_entry(void * param)
{
//	static rt_uint8_t idensity_last = 255;
	rt_uint8_t idensity =255;
	while(1)
	{
		if(DI_VALID(G0_PIN))
		{
			idensity = 0;
		}
		else if(DI_VALID(G1_PIN))
		{
			idensity = 1;
		}
		else if(DI_VALID(G2_PIN))
		{
			idensity = 2;
		}
		else if(DI_VALID(G3_PIN))
		{
			idensity = 3;
		}
		else if(DI_VALID(G4_PIN))
		{
			idensity = 4;
		}
		else if(DI_VALID(G5_PIN))
		{
			idensity = 5;
		}
		else
		{
			//idensity = 0;
		}
//		//继电器状态检测
//		if(DI_VALID(RELAY_STATE_PIN))
//		{
//		}
//		else
//		{
//		}
//		//本地/远程控制
//		if(DI_VALID(R_L_PIN))
//		{
//			
//		}
		if((get_idensity() != idensity) && (idensity<=5))
		{
			if(idensity ==0)
			{
                   pid_ref_set(&pid, 0);
                
			}
            else
            {
                 pid_ref_set(&pid, Istd[idensity-1]);
            }
			update_idensity(idensity);
			write_sysdata_to_eeprom(EEPROM_WRITE_DATABASE_VALUE);
		}
		rt_thread_mdelay(20);
		
	}
}

int di_init(void)
{
	rt_err_t res;
	
	rt_pin_mode(R_L_PIN, PIN_MODE_INPUT_PULLUP);
	rt_pin_mode(RELAY_STATE_PIN, PIN_MODE_INPUT);
	rt_pin_mode(G0_PIN, PIN_MODE_INPUT_PULLUP);
	rt_pin_mode(G1_PIN, PIN_MODE_INPUT_PULLUP);
	rt_pin_mode(G2_PIN, PIN_MODE_INPUT_PULLUP);
	rt_pin_mode(G3_PIN, PIN_MODE_INPUT_PULLUP);
	rt_pin_mode(G4_PIN, PIN_MODE_INPUT_PULLUP);
	rt_pin_mode(G5_PIN, PIN_MODE_INPUT_PULLUP);
	
	res = rt_thread_init(&di_thread, "di_thread", di_thread_entry, 0,
					&di_stack[0], DI_THREAD_STACK_SIZE, 
					DI_THREAD_PRIORITY, DI_THREAD_TICK);
	if(res != RT_EOK)
	{
		LOG_E("thread_init  di_therad failed!");
	}
	rt_thread_startup(&di_thread);
	return 0;
}

INIT_APP_EXPORT(di_init);




