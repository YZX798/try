#include "state.h"

State g_State;


//模式选择
static void State_chose(State *self)
{
	KEY_Change My_KEY = self->key->KEY_Get(self->key);

	//任务模式
	if (My_KEY == KEY_STATE)
	{
		if(self->task_state == TASK_1)	self->task_state=TASK_2;
		else if(self->task_state == TASK_2)	self->task_state=TASK_1;
		
		self->run_state  = RUN_STOP;
		
		self->tuner->Tuner_Begin(self->tuner, (uint8_t)self->task_state);
		
	}
	
	//运行模式
	if (My_KEY == KEY_RUN)
	{
		if(self->task_state == TASK_1)
		{
			switch (self->run_state)
			{
			case RUN_STOP:   self->run_state = RUN_Single;	break;
			case RUN_Single: self->run_state = RUN_STOP;    break;
			}
		}
		else if(self->task_state == TASK_2)
		{
			switch (self->run_state)
			{
			case RUN_STOP:   self->run_state = RUN_Single; break;
			case RUN_Single: self->run_state = RUN_FWD;    break;
			case RUN_FWD:    self->run_state = RUN_STOP;   break;
			}
		}
	}
	
	//调参档位
	if (My_KEY == KEY_NUM)
	{
		self->tuner->Dangwei_Change(self->tuner);
	}
	self->tuner->Pot_Change(self->tuner, self->pot->final);
}



//速度环输出确认
static void Control_Speed(State *self,float Encoder_Left ,float Encoder_Right)
{
	PID *P = (self->task_state == TASK_1)?self->spd1:self->speed;
	if(self->run_state == RUN_STOP)
	{
		P->Hope=0;
		P->reset(P);
	}
	else if(self->run_state == RUN_Single) P->Hope = (self->task_state == TASK_1)? g_param.spd1_hope : 0;
	else if(self->run_state == RUN_FWD) P->Hope = (self->task_state == TASK_1)? 0 : g_param.speed_hope;
	
	self->tgt = P->Hope;
	self->real_V = (self->task_state == TASK_1) ? Encoder_Right: (Encoder_Left + Encoder_Right);
	P->update(P, self->real_V, 0);
}



//最终输出
static void Control_Final(State *self)
{
	if (self->task_state == TASK_1)
	{
		PWM_L_OUT = 0;
		PWM_R_OUT = self->spd1->out;
	}
	else if (self->task_state == TASK_2)
	{
		Control_Mix(self->stand, self->speed, self->turn, &PWM_Base_out, &PWM_L_OUT, &PWM_R_OUT);
	}
	if (self->run_state != RUN_STOP && self->tuner->dangwei == PID_OFF)
	{
		if (PWM_L_OUT > 0.0f)      PWM_L_OUT += g_param.pwm_dead;
		else if (PWM_L_OUT < 0.0f) PWM_L_OUT -= g_param.pwm_dead;

		if (PWM_R_OUT > 0.0f)      PWM_R_OUT += g_param.pwm_dead;
		else if (PWM_R_OUT < 0.0f) PWM_R_OUT -= g_param.pwm_dead;
		
		motor_left.set(&motor_left,   PWM_L_OUT);
		motor_right.set(&motor_right, PWM_R_OUT);
	}
	else
	{
		motor_left.set (&motor_left,  0);
		motor_right.set(&motor_right, 0);
	}
}

//OLED任务显示
static char *State_mode_task_name(State *self)
{
	if(self->task_state == TASK_1)	return "T1";
	else if(self->task_state == TASK_2)	return "T2";
	return "Error";
}

//OLED模式显示
static char *State_mode_run_name(State *self)
{
	if (self->run_state == RUN_FWD)											return "FWD";
	else if (self->run_state == RUN_Single && self->task_state == TASK_2)	return "STAND";
	else if (self->run_state == RUN_Single && self->task_state == TASK_1)	return "SPEED";
	return "STOP";
}

//调参档位显示
static char *State_mode_name(State *self)
{
	return self->tuner->tab[self->tuner->dangwei].name;
}

//初始化
void State_Init(State *self)
{
	self->task_state = TASK_2;
	self->run_state  = RUN_first;

	self->real_V= 0;
	self->tgt   = 0;
	self->angle	= 0;

	self->key   = &g_KEY;
	self->tuner = &g_tuner;
	self->stand = &g_stand;
	self->speed = &g_speed;
	self->spd1  = &g_spd1;
	self->turn  = &g_turn;
	self->pot	= &g_pot;

	self->State_choose  = State_chose;
	self->Mode_TaskName = State_mode_task_name;
	self->Mode_RunName  = State_mode_run_name;
	self->Mode_TuneName = State_mode_name;
	self->Speed_Loop = Control_Speed;
	self->Control = Control_Final;

	self->tuner->Tuner_Begin(self->tuner, (uint8_t)self->task_state);

}

