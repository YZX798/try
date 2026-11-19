#include "turner.h"

Tuner g_tuner;


//单轮调参用表
static const  Tuner_mode_boom   tab_t1[PID_MAX] =
{
	[PID_OFF]      = { "Last",   0,                   0 },
	[PID_Speed]    = { "Speed", &g_param.spd1_hope, 500 },
	[PID_KP_SPEED] = { "Kp",    &g_param.spd1_kp,   500 },
	[PID_KI_SPEED] = { "Ki",    &g_param.spd1_ki,    50 },
	[PID_KD_SPEED] = { "Kd",    &g_param.spd1_kd,    50 },
};

//单轮档位菜单
static const Change_PID_Num 	menu_t1[] =
{
	PID_Speed , PID_KP_SPEED, PID_KI_SPEED, PID_KD_SPEED, 
	PID_OFF
};


//循迹模式调参用表
static const  Tuner_mode_boom   tab_t2[PID_MAX] =
{
	//[档位]		 { name,	target,		 	Max_num}
	[PID_OFF]      = { "Last",   0,                  0 },
	[PID_KP_STAND] = { "Kp_S",  &g_param.stand_kp, 500 },
	[PID_KD_STAND] = { "Kd_S",  &g_param.stand_kd,  50 },
	[PID_KP_SPEED] = { "Kp_V",  &g_param.speed_kp, 500 },
	[PID_KP_TURN]  = { "Kp_T",  &g_param.turn_kp,  100 },
	[PID_KD_TURN]  = { "Kd_T",  &g_param.turn_kd,   10 },
};


//循迹档位菜单
static const  Change_PID_Num   	menu_t2[] =
{
	PID_KP_STAND, PID_KD_STAND,
	PID_KP_SPEED, PID_KP_TURN,
	PID_KD_TURN , PID_OFF
};


//任务切换时初始化
static void tuner_begin(Tuner *self, uint8_t task)
{
	if(task == 0)
	{
		self->tab   = tab_t1;
		self->menu  = menu_t1;
		self->count = (uint8_t)MENU_T1_N;
	}
	if(task == 1)
	{
		self->tab   = tab_t2;
		self->menu  = menu_t2;
		self->count = (uint8_t)MENU_T2_N;
	}

	self->idx     	= self->count - 1;  //OFF档在表尾
	self->dangwei 	= PID_OFF;
}


//调参档位切换
static void tuner_dangwei(Tuner *self)
{
	self->idx++;
	if (self->idx >= self->count) self->idx = 0;
	self->dangwei = self->menu[self->idx];
}


//电位器
static void tuner_pot_change(Tuner *self, float pot)
{
	const Tuner_mode_boom *now;
	
	self->pot = pot;
	
	now = &self->tab[self->dangwei];
	if (now->target == 0) return;	//OFF时退出
	
	*now->target = pot * now->Max_num;
	
	if (now->target == &g_param.speed_kp)
		g_param.speed_ki = g_param.speed_kp / 200;
	
	self->Show  = *now->target;
}


//初始化
void Tuner_Init(Tuner *self)
{
	self->Tuner_Begin   	= 	tuner_begin;
	self->Dangwei_Change	= 	tuner_dangwei;
	self->Pot_Change    	= 	tuner_pot_change;

	self->tab     = tab_t2;
	self->menu    = menu_t2;
	self->count   = (uint8_t)MENU_T2_N;
	self->idx     = 0;
	self->dangwei = PID_OFF;
	self->pot     = 0;
	self->Show    = 0;
}
