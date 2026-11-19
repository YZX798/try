#ifndef __STATE_H__
#define __STATE_H__

#include "adc.h"
#include "param.h"
#include "KEY.h"
#include "main.h"
#include "pot.h"
#include "turner.h"
#include "moto.h"
#include "stdio.h"
#include "encoder.h"
#include "PID.h"

#define RUN_first   RUN_Single

typedef struct State State;
struct State
{
	//数据
	Task_Change task_state;
	RUN_Change  run_state;
	Change_PID_Num Change_state;
	
	KEY   *key;					//按键
	Tuner *tuner;				//调参对象
	PID   *stand;				//直立环
	PID   *speed;				//循迹速度环
	PID   *spd1;				//单轮速度环
	PID   *turn;				//转向环
	encoder	*encoder;			//编码器
	pot_num *pot;
	
	//显示
	float real_V;				//速度环实际输入（显示用）
	float tgt;					//速度环目标（显示用）
	float angle;				//当前角度（显示用）

	//函数
	void (*State_choose)(State *self);			//模式选择
	void (*Speed_Loop)  (State *self,float Encoder_Left ,float Encoder_Right);													//目标速度设置
	void (*Control)     (State *self);													//最终控制
	//OLED显示
	char *(*Mode_TaskName)(State *self);
	char *(*Mode_RunName) (State *self);
	char *(*Mode_TuneName)(State *self);
};

extern State g_State;

void State_Init(State *self);



#endif
