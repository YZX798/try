#include "pot.h"

pot_num g_pot;

static void Pot_Get(pot_num *self)
{
	HAL_ADC_Start(self->hadc);

	if (HAL_ADC_PollForConversion(self->hadc, self->wait_time) == HAL_OK) {
		self->raw = HAL_ADC_GetValue(self->hadc);
	}
	
	HAL_ADC_Stop(self->hadc);
	
	self->final = self->raw/self->handle_num;
}

void pot_Init(pot_num *self)
{
	self->hadc = &hadc1;
	self->raw = 0;
	self->final = 0;
	self->wait_time = 10;
	self->handle_num = 4095;
	self->get = Pot_Get;
}
