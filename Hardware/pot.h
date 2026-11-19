#ifndef __POT_H__
#define __POT_H__

#include "adc.h"

typedef struct pot_num pot_num;

struct pot_num
{
	ADC_HandleTypeDef* hadc;
	float raw;
	float final;
	int wait_time;
	float handle_num;
	
	void (*get)(pot_num *self);
};

extern pot_num g_pot;

void pot_Init(pot_num *self);


#endif
