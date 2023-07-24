#include "pid.h"
#include <string.h>
#include <math.h>
#include <stdint.h>

void pid_init(pid_struct_t pid, pid_control_para *param)
{
    memcpy((uint8_t *)&(pid->pid_parm), (uint8_t* )param\
            ,sizeof(pid_control_para));
    
    pid->out = 0;
    pid->feedback = 0;
#ifdef PID_USING_SINGAL_NEURAL   
    pid->out = param->out_min;
#else
    pid->out = 0;
#endif
    pid->error = 0;
    pid->error_1 = 0;
#ifdef PID_USING_POSITION
    pid->error_sum = 0;
#endif

    pid->error_2 = 0;
}

static pid_data_type pid_out_limit(pid_struct_t pid, pid_data_type out)
{
    if(out > pid->pid_parm.out_max)
    {
        out = pid->pid_parm.out_max;
    }
    else if( out < pid->pid_parm.out_min)
    {
        out = pid->pid_parm.out_min;
    }
    
    return out;
}

//static  pid_data_type pid_integral_limit(pid_struct_t pid, pid_data_type out)
//{
//    if(out > pid->pid_parm.integral_max)
//    {
//        out = pid->pid_parm.integral_max;
//    }
//    return out;
//}
void pid_ref_set(pid_struct_t pid, pid_data_type ref)
{
    pid->pid_parm.ref = ref;
}


void pid_update_feedback(pid_struct_t pid, pid_data_type feedback)
{
    #ifdef PID_USING_POSITION
    pid->feedback += pid->out*0.1 -1;
    #else
    pid->feedback = pid->out -1;
    #endif
}

pid_data_type pid_calc(pid_struct_t pid, pid_data_type feed_back)
{
    return pid->pid_calc(pid, feed_back);
}
//位置式pid
#ifdef PID_USING_POSITION
pid_data_type pid_position_calc(pid_struct_t pid, pid_data_type feed_back)
{
    pid_data_type  out = 0;
    pid_data_type  kp_out, ki_out, kd_out;

    pid->feedback += feed_back;  

    pid->error = pid->pid_parm.ref - pid->feedback;
    
    kp_out = pid->pid_parm.kp * pid->error;

    if(pid->error <10) //积分分离
    {
        pid->error_sum += pid->error;
        ki_out = pid->pid_parm.ki * pid->error_sum;
    //    ki_out = pid_integral_limit(pid, ki_out);    //积分限幅
    }
    kd_out = pid->pid_parm.kd * (pid->error - pid->error_1);
    
    out = kp_out + ki_out + kd_out;
   // pid->out = pid_out_limit(pid, out);
    pid->out = out;
    pid->error_1 = pid->error;

    return pid->out;
}

#endif

#ifdef PID_USING_INCREMENTAL
//增量式pid
pid_data_type pid_incremental_calc(pid_struct_t pid, pid_data_type feed_back)
{
    pid_data_type  out = 0;
    pid_data_type  kp_out, ki_out, kd_out;


    pid->feedback = feed_back;  

    pid->error = pid->pid_parm.ref - pid->feedback;
    
    kp_out = pid->pid_parm.kp * (pid->error - pid->error_1);


    ki_out = pid->pid_parm.ki * pid->error;

  // ki_out = pid_integral_limit(pid, ki_out);    //积分限幅

    kd_out = pid->pid_parm.kd * (pid->error - 2* pid->error_1 + pid->error_2);
    
    pid->out =  pid->out + kp_out + ki_out + kd_out;
    pid->out = pid_out_limit(pid, pid->out);
    //pid->out = out;
    pid->error_2 = pid->error_1;
    pid->error_1 = pid->error; 


    return pid->out;
}
#endif
#define K_MAX_SET 2.0f
#ifdef PID_USING_SINGAL_NEURAL
pid_data_type  wkp, wki, wkd, wp, wi,wd;
pid_data_type  wadd, x1, x2, x3;
float p_out, i_out, d_out;
pid_data_type pid_single_neural_calc(pid_struct_t pid, pid_data_type feed_back)
{
    pid_data_type  out = 0;
//    pid_data_type  wkp, wki, wkd, wp, wi,wd;
//    pid_data_type  wadd, x1, x2, x3;

    pid->feedback = feed_back;  

    pid->error = pid->pid_parm.ref - pid->feedback;
    if(fabs(pid->error) < 0.06f)
        return  pid->out;

    x1 = pid->error - pid->error_1;
    x2 = pid->error;
    x3 = pid->error - 2*pid->error_1 + pid->error_2;

    
    wkp = pid->wkp;
    wki = pid->wki;
    wkd = pid->wkd;
    wadd = fabs(wkp) + fabs(wki) + fabs(wkd);
    wp = wkp/wadd;
    wi = wki/wadd;
    wd = wkd/wadd;


    p_out = pid->pid_parm.kneural*wp*x1;
    i_out = pid->pid_parm.kneural*wi*x2;
    d_out = pid->pid_parm.kneural*wd*x3;
    
//    if((pid->pid_parm.ref >0) && fabs(x1) < 0.05f && fabs(x2)>0.09f)
//    {
//        pid->pid_parm.kneural = K_MAX_SET*1.5f;
//    }
//    else
//    {
//        pid->pid_parm.kneural = 2.0f;
//    }
    //out = pid->out + pid->pid_parm.kneural*(wp * x1 + wi* x2 + wd *x3);
    out = pid->out +p_out + i_out + d_out;
    pid->out = pid_out_limit(pid, out);

    
  //  wkp = pid->wkp + pid->pid_parm.kp * pid->error * pid->out * (2*pid->error - pid->error_1);
    wkp = pid->wkp + pid->pid_parm.kp * pid->error * pid->out * (2*pid->error - pid->error_1);
// 积分分离
//    if( pid->error / pid->pid_parm.ref  < 0.15f)
//    {
    wki = pid->wki + pid->pid_parm.ki * pid->error * pid->out * (2*pid->error - pid->error_1);
//    }
//    else
//    {
//        wki = 0;
//    }
    wkd = pid->wkd + pid->pid_parm.kd * pid->error * pid->out * (2*pid->error - pid->error_1);
    

    pid->wkp = wkp;
    pid->wki = wki;
    pid->wkd = wkd;

    pid->error_2 = pid->error_1;
    pid->error_1 = pid->error; 


    return pid->out;
}

#endif
