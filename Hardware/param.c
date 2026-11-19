#include "param.h"

//参数实例
Param_t g_param;

//默认值
void Param_Init(void)
{
	//直立环
	g_param.stand_kp		=	  62.4;//104
	g_param.stand_kd		=	     3;//4.2
	g_param.stand_hope		=	    0;
	g_param.stand_limit		=	 2000;

	//循迹速度环
	g_param.speed_kp        =	  13.5;
	g_param.speed_ki        =	  g_param.speed_kp/200;
	g_param.speed_filt      =	  0.3;
	g_param.speed_sum_limit =	  200;
	g_param.speed_limit     =	 1000;
	g_param.speed_hope		=	   14;

	//转向环
	g_param.turn_kp			=	   60;
	g_param.turn_kd			=		6;
	g_param.turn_limit		=	  500;

	//单轮速度环
	g_param.spd1_kp			=	   30;
	g_param.spd1_ki			=		2;
	g_param.spd1_kd			=		0;
	g_param.spd1_hope		=		0;
	g_param.spd1_filt		=	  0.3;
	g_param.spd1_limit		=	 3000;
	g_param.spd1_sum_limit	=	  800;

	//量程 / 限幅
	g_param.pot_p			=	  500;
	g_param.pot_id			=	   50;
	g_param.pwm_out_max		=	 3400;
	g_param.pwm_base_max	=	 2500;
	g_param.pwm_dead		=	  360;
}
