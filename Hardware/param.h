#ifndef __PARAM_H__
#define __PARAM_H__

#include "main.h"

//板载调参参数
typedef struct
{
	//直立环
	float stand_kp;
	float stand_kd;
	float stand_hope;			//直立零点
	float stand_limit;			//输出限幅

	//任务二速度环
	float speed_kp;
	float speed_ki;
	float speed_filt;			//一阶低通系数
	float speed_limit;			//输出限幅
	float speed_sum_limit;		//积分限幅
	float speed_hope;			//目标速度

	//转向环
	float turn_kp;
	float turn_kd;
	float turn_limit;

	//任务一单轮速度环
	float spd1_kp;
	float spd1_ki;
	float spd1_kd;
	float spd1_hope;
	float spd1_filt;
	float spd1_limit;
	float spd1_sum_limit;

	//任务	电位器量程	限幅
	float pot_p;				//电位器 P 类满量程
	float pot_id;				//电位器 I/D 类满量程
	float pwm_out_max;			//单轮 PWM 上限
	float pwm_base_max;			//直立+速度合并后的限幅
	float pwm_dead;				//死区

} Param_t;

extern Param_t g_param;			//参数实例
void Param_Init(void);			//初始化


//前向声明
typedef struct PID PID;

struct PID
{
	//指向 g_param 里的系数
	float *kp;
	float *ki;
	float *kd;
	float *filt;
	float *limit;
	float *sum_limit;
	float  Hope;

	//自己的运行状态
	float err;
	float last_err;
	float sum;
	float filt_out;
	float last_filt_out;
	float out;

	void (*update)(PID *self, float real, float gyro);
	void (*reset)(PID *self);
};

extern PID g_stand;				//直立环
extern PID g_speed;				//循迹速度环
extern PID g_turn;				//转向环
extern PID g_spd1;				//单轮速度环




//其它全局
extern float pot;				//电位器归一化值 0.0~1.0
extern float PWM_Base_out;		//直立 + 速度
extern float PWM_L_OUT;
extern float PWM_R_OUT;

#endif
