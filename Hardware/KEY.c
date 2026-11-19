#include "KEY.h"

KEY g_KEY;

static KEY_Change Key_GetNum(KEY *self)
{
	self->now = KEY_NONE;
	KEY_Change out= KEY_NONE;
	
	for(uint8_t i = 0;i < KEY_NUM_CH;i++)
    {
		if (HAL_GPIO_ReadPin(self->GPIO[i], self->pin[i]) == 0)
		{
			self->now = (KEY_Change)(i+1);	break;
		}
	}
		if (self->now == KEY_NONE && self->last != KEY_NONE)
		{	self->state = self->last;	}

		self->last = self->now;
		
		if (self->state)
		{
			out = self->state;
			self->state = KEY_NONE;
		}
		
	return out;
}

//³õÊ¼»¯
void Key_Init(KEY *self)
{

	self->GPIO[0] = GPIOB;  self->pin[0] = GPIO_PIN_1;
	self->GPIO[1] = GPIOB;  self->pin[1] = GPIO_PIN_0;
	self->GPIO[2] = GPIOA;  self->pin[2] = GPIO_PIN_1;
	
	self->now   = KEY_NONE;
	self->state = KEY_NONE;
	self->last  = KEY_NONE;
	
	self->KEY_Get = Key_GetNum;
}
	


