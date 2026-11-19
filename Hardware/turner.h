#ifndef __TURNER_H
#define __TURNER_H

#include "main.h"
#include "param.h"

#define MENU_T1_N   (sizeof(menu_t1) / sizeof(menu_t1[0]))
#define MENU_T2_N   (sizeof(menu_t2) / sizeof(menu_t2[0]))

//任务模式
typedef enum 
{
	TASK_1 = 0,
	TASK_2,
}Task_Change;

//运行模式
typedef enum 
{
	RUN_STOP = 0,
	RUN_Single,
	RUN_FWD,
}RUN_Change;

//调参档位
typedef enum 
{
	PID_OFF = 0,
	PID_KP_STAND,
	PID_KD_STAND,
	PID_KP_SPEED,
	PID_KI_SPEED,
	PID_KD_SPEED,
	PID_KP_TURN,
	PID_KD_TURN,
	PID_Speed,
	PID_MAX,
}Change_PID_Num;

typedef struct
{
	char  *name;
	float *target;
	float  Max_num;
} Tuner_mode_boom;

typedef struct Tuner Tuner;
struct Tuner
{
	//数据
	const Tuner_mode_boom  *tab;			//当前任务的参数表
	const Change_PID_Num   *menu;			//当前任务的档位循环表
	uint8_t               	count;			//参数表总长
	uint8_t               	idx;			//遍历下标
	Change_PID_Num        	dangwei;		//档位
	float                 	pot;			//接受电位器传入值
	float                 	Show;			//传给OLED的值

	//函数
	void (*Tuner_Begin)   	(Tuner *self, uint8_t task);  //0=T1, 1=T2；档位回 OFF
	void (*Dangwei_Change)  (Tuner *self);                    //档位切换
	void (*Pot_Change)    	(Tuner *self, float pot);         //电位器 -> 实时写参数
	char *(*State_Name)		(Tuner *self);                    //显示用：档位名
};

extern Tuner g_tuner;

void Tuner_Init(Tuner *self);

#endif
