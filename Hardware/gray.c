#include "gray.h"

GRAY g_gray;

static int Gary_out(GRAY *self)
{
	self->sum_num=0;
	if(HAL_GPIO_ReadPin(self->GPIO[0], self->Pin[0]) == GRAY_True) { self->sum_num++; self->led1 = -4; } else { self->led1 = 0; }
	if(HAL_GPIO_ReadPin(self->GPIO[1], self->Pin[1]) == GRAY_True) { self->sum_num++; self->led2 = -2; } else { self->led2 = 0; }
	if(HAL_GPIO_ReadPin(self->GPIO[2], self->Pin[2]) == GRAY_True) { self->sum_num++; self->led3 =  0; } else { self->led3 = 0; }
	if(HAL_GPIO_ReadPin(self->GPIO[3], self->Pin[3]) == GRAY_True) { self->sum_num++; self->led4 =  2; } else { self->led4 = 0; }
	if(HAL_GPIO_ReadPin(self->GPIO[4], self->Pin[4]) == GRAY_True) { self->sum_num++; self->led5 =  4; } else { self->led5 = 0; }
	
	self->weighted_sum = self->led1 + self->led2 + self->led3 + self->led4 + self->led5;
	
	if(self->sum_num == 0)	self->ALL_Num = 0;
	else		self->ALL_Num = (float)self->weighted_sum / self->sum_num;
	
	self->gray_return = (int)(self->ALL_Num * 2 + 0.5);
	
	return self->gray_return;
}

void Gray_Init(GRAY *self)
{
	self->GPIO[0] = GPIOB;  self->Pin[0] = GPIO_PIN_12;
	self->GPIO[1] = GPIOB;  self->Pin[1] = GPIO_PIN_13;
	self->GPIO[2] = GPIOC;  self->Pin[2] = GPIO_PIN_13;
	self->GPIO[3] = GPIOC;  self->Pin[3] = GPIO_PIN_14;
	self->GPIO[4] = GPIOC;  self->Pin[4] = GPIO_PIN_15;
	
	self->sum_num = 0;
	self->weighted_sum = 0;
	self->ALL_Num = 0;
	self->led1 = self->led2 = self->led3 = self->led4 = self->led5 = 0;
	self->gray_return = 0;
	
	self->Gary_BACK = Gary_out;
}
