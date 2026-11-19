#ifndef __KEY_H__
#define __KEY_H__

#include "main.h"
#include "adc.h"

#define	KEY_NUM_CH	3

//按键
typedef enum 
{
	KEY_NONE = 0,	//无按键
	KEY_RUN,		//启停键
	KEY_NUM,		//档位切换键
	KEY_STATE,		//任务切换键
}KEY_Change;

typedef struct KEY KEY;

struct KEY
{
	GPIO_TypeDef   *GPIO[KEY_NUM_CH];	//三个键所在端口
	uint16_t		pin [KEY_NUM_CH];	//三个键所在引脚
	KEY_Change		last;				//上次采集值
	KEY_Change		now;				//当前采集值
	KEY_Change		state;				//状态值
	
	KEY_Change (*KEY_Get)(KEY *self);
	
};

extern KEY g_KEY;

extern void Key_Init(KEY *self);

#endif

