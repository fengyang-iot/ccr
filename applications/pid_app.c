#include "pid_app.h"
#include "rtthread.h"
#include "board.h"
#include "cross_zero_check.h"
#include "alarm.h"
#include "database.h"
#include "adc_sample.h"
#define DBG_TAG "pid_app.c"
#define DBG_LVL  DBG_INFO

#include "rtdbg.h"
//1. 采样周期
//2. 控制周期 



#define  PID_THREAD_STATCK_SIZE  512
#define  PID_THREAD_PRIORITY     8
#define  PID_THREAD_TICK         10
ALIGN(RT_ALIGN_SIZE)
struct rt_thread pid_ctrl_thread;
rt_uint8_t pid_thread_stack[PID_THREAD_STATCK_SIZE];

struct rt_semaphore pid_ctrl_sem;

rt_uint8_t  g_ctrl_cycle;   
rt_uint8_t g_pid_enable; 

const float Istd[5]  = {2.8f,3.4f,4.1f,5.2f,6.6f};
struct PID pid;

void pid_para_init(void)
{
    pid_control_para pid_para;
	pid_para.kp = PID_KP; //0.05f;//0.4f;
	pid_para.ki = PID_KI;//0.35f;
	pid_para.kd = PID_KD;//0.01f;//0.4f;//0.05f;//0.4f;
	pid_para.ref = 0;
	pid_para.out_max = PID_OUT_MAX;
	pid_para.out_min = PID_OUT_MIN;

	#ifdef PID_USING_SINGAL_NEURAL
    pid_para.kneural = PID_K;
    
    pid.wkp = KP_0;
	pid.wki = KI_0;
	pid.wkd = KD_0;	
    #endif
	
	pid_init(&pid, &pid_para);
	pid.out = PID_OUT_MIN;
}
void ccr_init(void)
{
	
	pid_para_init();

    #ifdef PID_USING_POSITION
    pid.pid_calc = pid_position_calc;
    LOG_I("using pid positon ....\r\n");
    #endif
    #ifdef PID_USING_INCREMENTAL
    pid.pid_calc = pid_incremental_calc;
    LOG_I("using pid incremental ....\r\n");
    #endif
    #ifdef PID_USING_SINGAL_NEURAL
   
	pid.pid_calc = pid_single_neural_calc;
    LOG_I("using pid single_neural ....\r\n");
    #endif
	
	if(rt_sem_init(&pid_ctrl_sem, "pid_ctrl_sem",  0, RT_IPC_FLAG_FIFO))
	{
		LOG_E("rt_sem_init failed!");
	}
    g_pid_enable = 1;
}

#define  ANGLE_ARRAY_SIZW  75
rt_uint8_t angle_arry[ANGLE_ARRAY_SIZW];
rt_uint16_t i_arry[ANGLE_ARRAY_SIZW];
rt_uint8_t idensity_last;
rt_uint8_t debug_cnt;


struct  pid_debug  g_pid_debug_struct;
extern pid_data_type   wp, wi,wd;
void save_pid_process_data()
{
    if(idensity_last != get_idensity())
    {
        g_pid_debug_struct.data_arr[0][g_pid_debug_struct.cnt] = g_ccr_data.ccr_info.angle;
        g_pid_debug_struct.data_arr[1][g_pid_debug_struct.cnt] = g_ccr_data.ccr_info.i_load;
        g_pid_debug_struct.data_arr[2][g_pid_debug_struct.cnt] = wp*1000;
        g_pid_debug_struct.data_arr[3][g_pid_debug_struct.cnt] = wi*1000;
        g_pid_debug_struct.data_arr[4][g_pid_debug_struct.cnt] = wd*1000;
       // g_pid_debug_struct.data_arr[2][g_pid_debug_struct.cnt] = g_ccr_data.ccr_info.v_scr;
        g_pid_debug_struct.cnt++;
       if(g_pid_debug_struct.cnt >=SAMPLE_SIZE)
       {
           g_pid_debug_struct.cnt = 0;
           idensity_last = get_idensity();
           g_pid_debug_struct.full_flag = 1;
       }
   }
}
    
void print_pid_precess_data()
{
    if(!g_pid_debug_struct.full_flag)
        return;
    g_pid_debug_struct.full_flag = 0;
    rt_kprintf("angle array is: \r\n");
    for(int i=0;i<SAMPLE_SIZE; i++)
    {
        if(i%10 == 0)
        {
           rt_kprintf("\r\n");
        }
        rt_kprintf("%d, ", g_pid_debug_struct.data_arr[0][i]);

     }
     rt_kprintf("\r\n");
     rt_kprintf("i_load array is: \r\n");
     for(int i=0;i<SAMPLE_SIZE; i++)
     {
           if(i%10 == 0)
           {
               rt_kprintf("\r\n");
           }
           rt_kprintf("%d, ", g_pid_debug_struct.data_arr[1][i]);

      }
      rt_kprintf("\r\n");
     rt_kprintf("wp  is: \r\n");
     for(int i=0;i<SAMPLE_SIZE; i++)
     {
           if(i%10 == 0)
           {
               rt_kprintf("\r\n");
           }
           rt_kprintf("%d, ", g_pid_debug_struct.data_arr[2][i]);

      }
      rt_kprintf("\r\n");
     rt_kprintf("wi is: \r\n");
     for(int i=0;i<SAMPLE_SIZE; i++)
     {
           if(i%10 == 0)
           {
               rt_kprintf("\r\n");
           }
           rt_kprintf("%d, ", g_pid_debug_struct.data_arr[3][i]);

      }
      rt_kprintf("\r\n");
           rt_kprintf("wd is: \r\n");
     for(int i=0;i<SAMPLE_SIZE; i++)
     {
           if(i%10 == 0)
           {
               rt_kprintf("\r\n");
           }
           rt_kprintf("%d, ", g_pid_debug_struct.data_arr[4][i]);

      }
      rt_kprintf("\r\n");
}
//void pid_debug(rt_uint8_t angle, rt_uint16_t i_load)
//{
//    if(idensity_last != get_idensity())
//   {
//       angle_arry[debug_cnt] = angle;
//       i_arry[debug_cnt]  = i_load;
//       debug_cnt++;
//       if(debug_cnt >=ANGLE_ARRAY_SIZW)
//       {
//           debug_cnt = 0;
//           idensity_last = get_idensity();
////           rt_kprintf("angle array is: \r\n");
////           for(int i=0;i<ANGLE_ARRAY_SIZW; i++)
////           {
////               if(i%10 == 0)
////               {
////                   rt_kprintf("\r\n");
////               }
////               rt_kprintf("%d, ", angle_arry[i]);

////           }
////           rt_kprintf("\r\n");
////           rt_kprintf("i_load array is: \r\n");
////           for(int i=0;i<ANGLE_ARRAY_SIZW; i++)
////           {
////               if(i%10 == 0)
////               {
////                   rt_kprintf("\r\n");
////               }
////               rt_kprintf("%d, ", i_arry[i]);

////           }
////           rt_kprintf("\r\n");
//       }
//   }
//}

//static void pid_thread_entry(void *para)
//{
//    rt_uint16_t i_load; 
//    float cal_k;
//    static rt_uint8_t ctrl_times;

//	while(1)
//	{
//		if(g_pid_enable && Is_cross_zero_valid())
//		{
//			if(RELAY_STATUS_OFF == GET_SYS_STATUS_BIT(RELAY_STATUS_POS))
//			{
//				if(IS_SYS_FAULT_EXIST())
//				{
//					continue;
//				}					
//				if(g_ccr_data.ccr_info.intensity >0 && g_ccr_data.ccr_info.intensity <=5)
//				{
//					RELAY_ON();
//					update_pass_angle(MIN_PASS_ANGLE);
//                    pid_para_init();
//					rt_thread_mdelay(RELAY_CLOSE_DELAY_MS);
//					//TODO: 检测接触器是否闭
//					SET_SYS_STATUS_BIT(RELAY_STATUS_POS);
//					pid_ref_set(&pid,Istd[g_ccr_data.ccr_info.intensity-1]);
//					g_ctrl_cycle = CTRL_CYCLE;    // 第一次延长两个控制周期以便获取电流值
//                    ctrl_times  =2;
//				}					
//			}
//			else //接触器已经闭合，根据选定光级调节导通角
//			{
//				if(RT_EOK != rt_sem_take(&pid_ctrl_sem, (rt_int32_t)((CTRL_CYCLE+1)*10 *RT_TICK_PER_SECOND/1000.0)))
//				{
//						LOG_I("take  pid_ctrl_sem failed!");
//				}
//                i_load = get_rms(E_I_RMS);
//                if(g_ccr_data.ccr_info.intensity > 0)
//                {
//                    pid_ref_set(&pid,Istd[g_ccr_data.ccr_info.intensity-1]);
//                }
//                else
//                {
//                    pid_ref_set(&pid, 0);
//                    pid.wkp = 0.5f;
//                    pid.wki = 0.5f;
//                    pid.wkd = 0.5f;
//                    pid.error = pid.error_1 = pid.error_2 = 0;
//                    
//                }
//                if(ctrl_times > 0)// 启动后获取电流值，改变pid参数K
//                {
//                    ctrl_times--;
//                    if(ctrl_times == 0)
//                    {
//                        cal_k  = KMAX+0.1f - 0.0132f * i_load;
//                   //     pid.pid_parm.kneural = VAL_LIMIT(cal_k, KMIN, KMAX);
//                        LOG_I("current is %d, cal_k is %d", i_load, (rt_uint16_t)(cal_k*100));
//                    }
//                }
//                else
//                {
//                    pid_calc(&pid, i_load/100.0f); 
//                    update_pass_angle(pid.out);
//                    pid_debug(pid.out, i_load);
//                    if(i_load > 700)
//                    {
//                        RELAY_OFF();
//                        SET_SYS_STATUS_BIT(OVER_CUR);
//                        CLR_SYS_STATUS_BIT(RELAY_STATUS_POS);
//                        LOG_E("OVER CURRENT %d", g_ccr_data.ccr_info.i_load);
//                    }
//                    update_scr_voltage(get_rms(E_U_SCR_RMS));
//                    update_load_voltage(get_rms(E_U_LOAD_RMS));
//                }
//              //  g_ctrl_cycle = CTRL_CYCLE; 
//			}
//		}
//		rt_thread_mdelay(10);
//	}
//  
//}

rt_uint8_t g_over_cur_flag;
rt_uint16_t i_load_tmep[256];
static void pid_thread_entry(void *para)
{
//    rt_uint16_t i_load; 
//    float cal_k;
	while(1)
	{
       
        if(!g_pid_enable)
        {
            rt_thread_mdelay(100);
            continue;
        }
        
        if(RELAY_STATUS_OFF == GET_SYS_STATUS_BIT(RELAY_STATUS_POS))
        {		
            //有故障
//            if(g_over_cur_flag)
//            {
//                rt_thread_mdelay(200);
//                continue;
//            }
            if(IS_SYS_FAULT_EXIST())
            {
                rt_thread_mdelay(200);
                continue;
            }
            
            // 接触器未闭合            
            if(g_ccr_data.ccr_info.intensity >0 && g_ccr_data.ccr_info.intensity <=5)
            {
                RELAY_ON_NO_SET_FALG();
                update_pass_angle(MIN_PASS_ANGLE);
                pid_para_init();
                rt_thread_mdelay(RELAY_CLOSE_DELAY_MS);
					//TODO: 检测接触器是否闭
                SET_SYS_STATUS_BIT(RELAY_STATUS_POS);
                pid_ref_set(&pid,Istd[g_ccr_data.ccr_info.intensity-1]);
                g_ctrl_cycle = CTRL_CYCLE;    // 第一次延长两个控制周期以便获取电流值
			}
            else
            {
                rt_thread_mdelay(20);
            }
            
        }
        else //接触器已经闭合
        {
            #ifdef ENABLE_PID_DEBUG
                print_pid_precess_data();
            #endif
                rt_thread_mdelay(100);
                //控制周期到
//                if(RT_EOK != rt_sem_take(&pid_ctrl_sem, RT_WAITING_FOREVER))
//                {
//                        LOG_I("take  pid_ctrl_sem failed!");
//                }
//                i_load = g_ccr_data.ccr_info.i_load;
//                //禁止pid
//                if(g_pid_enable == RT_FALSE)
//                {
//                    
//                    if(index <256)
//                    {
//                        i_load_tmep[index++] = g_ccr_data.ccr_info.i_load;  
//                    }
//                    //LOG_I("current is %d\n", i_load);
//                    continue;
//                }
//                if(g_ccr_data.ccr_info.intensity > 0)
//                {
//                    pid_ref_set(&pid,Istd[g_ccr_data.ccr_info.intensity-1]);
//                }
//                else
//                {
//                    pid_ref_set(&pid, 0);
////                    pid.wkp = 0.5f;
////                    pid.wki = 0.5f;
////                    pid.wkd = 0.5f;
////                    pid.error = pid.error_1 = pid.error_2 = 0;
//                    
//                }
//                if(ctrl_times > 0)// 启动后获取电流值，改变pid参数K
//                {
//                    ctrl_times--;
//                    if(ctrl_times == 0)
//                    {
//                        cal_k  = KMAX+0.1f - 0.0132f *g_ccr_data.ccr_info.i_load;
//                   //     pid.pid_parm.kneural = VAL_LIMIT(cal_k, KMIN, KMAX);
//                        LOG_I("current is %d, cal_k is %d", g_ccr_data.ccr_info.i_load, (rt_uint16_t)(cal_k*100));
//                    }
//                }

        } 
    }
}

int  pid_ctrl_app(void)
{
	ccr_init();
	rt_err_t err;
	
	err = rt_thread_init(&pid_ctrl_thread, "pid_ctrl", pid_thread_entry,0, 
						&pid_thread_stack[0],
						PID_THREAD_STATCK_SIZE, 
						PID_THREAD_PRIORITY,
						PID_THREAD_TICK);
	if(err != RT_EOK)
	{
		LOG_E("pid_ctrl_thread init error!");
		return -1;
	}
	
	rt_thread_startup(&pid_ctrl_thread);
	return 0;
}



INIT_APP_EXPORT(pid_ctrl_app);
