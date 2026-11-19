#include "PID.h"

//环
PID g_stand;
PID g_speed;
PID g_turn;
PID g_spd1;

//直立 + 速度环
float PWM_Base_out;

//直立环：误差用角度，D 项直接用角速度
static void stand_update(PID *self, float real, float gyro)
{
	self->err = real - self->Hope;
	self->out = (*self->kp) * self->err + (*self->kd) * gyro;

	if (self->out >  (*self->limit)) self->out =  (*self->limit);
	if (self->out < -(*self->limit)) self->out = -(*self->limit);
}


//速度环
static void speed_update(PID *self, float real, float gyro)
{
	(void)gyro;

	self->err = real - self->Hope;

	// 一阶低通，滤掉编码器的量化抖动
	self->filt_out = (*self->filt) * self->err
	               + (1 - (*self->filt)) * self->last_filt_out;
	self->last_filt_out = self->filt_out;

	// 积分累加 + 限幅
	self->sum += self->filt_out;
	if (self->sum >  (*self->sum_limit)) self->sum =  (*self->sum_limit);
	if (self->sum < -(*self->sum_limit)) self->sum = -(*self->sum_limit);

	self->out = (*self->kp) * self->filt_out + (*self->ki) * self->sum;

	if (self->out >  (*self->limit)) self->out =  (*self->limit);
	if (self->out < -(*self->limit)) self->out = -(*self->limit);
}


//任务一单轮速度环
static void spd1_update(PID *self, float real, float gyro)
{
	(void)gyro;

	self->err =  self->Hope - real;

	self->filt_out = (*self->filt) * self->err
	               + (1 - (*self->filt)) * self->last_filt_out;
	self->last_filt_out = self->filt_out;

	self->sum += self->filt_out;
	if (self->sum >  (*self->sum_limit)) self->sum =  (*self->sum_limit);
	if (self->sum < -(*self->sum_limit)) self->sum = -(*self->sum_limit);

	self->out = (*self->kp) * self->filt_out
	          + (*self->ki) * self->sum
	          + (*self->kd) * (self->err - self->last_err);

	if (self->out >  (*self->limit)) self->out =  (*self->limit);
	if (self->out < -(*self->limit)) self->out = -(*self->limit);

	self->last_err = self->err;
}


//转向环：P + D
static void turn_update(PID *self, float real, float gyro)
{
	(void)gyro;

	self->err = real;
	self->out = (*self->kp) * self->err
			  + (*self->kd) * (self->err - self->last_err);

	if (self->out >  (*self->limit)) self->out =  (*self->limit);
	if (self->out < -(*self->limit)) self->out = -(*self->limit);

	self->last_err = self->err;
}


//清状态
static void pid_reset(PID *self)
{
	self->err           = 0;
	self->last_err      = 0;
	self->sum           = 0;
	self->filt_out      = 0;
	self->last_filt_out = 0;
	self->out           = 0;
}


//绑定系数指针 + 默认的清状态函数
//用不上的系数传 0,对应的 update 里不使用
void PID_Bind (	PID *self, float *kp, float *ki, float *kd,
				float *filt, float *limit, float *sum_limit )
{
	self->kp        = kp;
	self->ki        = ki;
	self->kd        = kd;
	self->filt      = filt;
	self->limit     = limit;
	self->sum_limit = sum_limit;

	self->reset = pid_reset;
	pid_reset(self);
}


//四个对象各自绑定系数和自己的update
void PID_InitAll(void)
{
	// 直立环：	kp , kd , limit
	PID_Bind (	&g_stand, &g_param.stand_kp, 0, &g_param.stand_kd,
				0 ,&g_param.stand_limit, 0);
	g_stand.update = stand_update;
	g_stand.Hope   = g_param.stand_hope;

	// 任务二速度环：kp , ki , filt , limit , sum_limit
	PID_Bind (	&g_speed, &g_param.speed_kp, &g_param.speed_ki,
				0 ,&g_param.speed_filt, &g_param.speed_limit, &g_param.speed_sum_limit );
	g_speed.update = speed_update;
	g_speed.Hope   = g_param.speed_hope;

	// 转向环：kp / kd / limit
	PID_Bind (	&g_turn, &g_param.turn_kp, 0, &g_param.turn_kd,
				0 ,&g_param.turn_limit, 0);
	g_turn.update = turn_update;
	g_turn.Hope	= 0;

	// 任务一单轮速度环：kp / ki / kd / filt / limit / sum_limit
	PID_Bind (	&g_spd1, &g_param.spd1_kp, &g_param.spd1_ki, &g_param.spd1_kd,
				&g_param.spd1_filt, &g_param.spd1_limit, &g_param.spd1_sum_limit );
	g_spd1.update = spd1_update;
	g_spd1.Hope   = g_param.spd1_hope;
}


//限幅
void Limit_Base(float *PWM_Base)
{
	if (*PWM_Base >  g_param.pwm_base_max) *PWM_Base =  g_param.pwm_base_max;
	if (*PWM_Base < -g_param.pwm_base_max) *PWM_Base = -g_param.pwm_base_max;
}


//控制环,合成PWM输出
void Control_Mix(PID *stand, PID *speed, PID *turn,float *PWM_Base_out,float *PWM_L_OUT,float *PWM_R_OUT)
{
	*PWM_Base_out = stand->out + speed->out;
	Limit_Base(PWM_Base_out);

	*PWM_L_OUT = *PWM_Base_out + turn->out;
	*PWM_R_OUT = *PWM_Base_out - turn->out;
}
