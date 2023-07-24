#ifndef __PID_H__
#define __PID_H__

typedef float pid_data_type;

//#define PID_USING_POSITION      //位置式PID      
//#define PID_USING_INCREMENTAL   //增量式PID
#define PID_USING_SINGAL_NEURAL  //单神经元PID算法

typedef struct pid_init_parameter
{
    pid_data_type   ref;
    pid_data_type   kp;
    pid_data_type   ki;
    pid_data_type   kd;
    #ifdef PID_USING_SINGAL_NEURAL
    pid_data_type   kneural;  //神经元比例系数
    #endif
    pid_data_type   out_max;
    pid_data_type   out_min;
    pid_data_type   integral_max;
    
}pid_control_para;

typedef struct  PID
{
    pid_data_type   feedback;    // actual output value
    pid_data_type   out;         // controler output value

    pid_data_type   error;      //current error
    pid_control_para pid_parm;
    pid_data_type   error_1;    //last error
    pid_data_type   error_2;    //last 2 error
    pid_data_type  error_sum;
 #ifdef    PID_USING_SINGAL_NEURAL
    pid_data_type   wkp; 
    pid_data_type   wki;
    pid_data_type   wkd;
 #endif
    pid_data_type (*pid_calc)(struct  PID*  pid, pid_data_type feed_back);
}*pid_struct_t;

void pid_init(pid_struct_t pid, pid_control_para *param);
void pid_ref_set(pid_struct_t pid, pid_data_type ref); 
pid_data_type pid_calc(pid_struct_t pid, pid_data_type feed_back);

pid_data_type pid_position_calc(pid_struct_t pid, pid_data_type feed_back);
pid_data_type pid_incremental_calc(pid_struct_t pid, pid_data_type feed_back);
pid_data_type pid_single_neural_calc(pid_struct_t pid, pid_data_type feed_back);
void pid_update_feedback(pid_struct_t pid, pid_data_type feedback);
static inline void pid_kp_set(pid_struct_t pid, pid_data_type kp)
{
    pid->pid_parm.kp = kp;
}

static inline void pid_ki_set(pid_struct_t pid, pid_data_type ki)
{
    pid->pid_parm.ki = ki;
}

static inline void pid_kd_set(pid_struct_t pid, pid_data_type kd)
{
    pid->pid_parm.kd = kd;
}

#endif
