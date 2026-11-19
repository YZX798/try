#ifndef __ENCODER_H__
#define __ENCODER_H__

#include "main.h"
#include "tim.h"

typedef struct encoder encoder;
struct encoder
{
	TIM_HandleTypeDef *htimA;
	TIM_HandleTypeDef *htimB;
	int	re0;
	int L_encoder;
	int R_encoder;
	void (*Read_Encoder)(encoder *self);

};

extern encoder g_encoder;
void Encoder_Init(encoder *self);

#endif
