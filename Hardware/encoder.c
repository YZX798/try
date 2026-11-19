#include "encoder.h"

encoder g_encoder;

void Read_Encoder_Speed(encoder *self)
{
	self->L_encoder = - (short)__HAL_TIM_GetCounter(self->htimA);
	__HAL_TIM_SetCounter(self->htimA,self->re0);
	
	self->R_encoder = (short)__HAL_TIM_GetCounter(self->htimB);
	__HAL_TIM_SetCounter(self->htimB,self->re0);
}


void Encoder_Init(encoder *self)
{
	self->L_encoder=0;
	self->R_encoder=0;
	self->re0=0;
	self->htimA=&htim3;
	self->htimB=&htim4;
	self->Read_Encoder = Read_Encoder_Speed;
}

//oled,gray,encoder
