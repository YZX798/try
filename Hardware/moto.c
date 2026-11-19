#include "moto.h"
#include "param.h"

Motor motor_left;
Motor motor_right;

//PWM设置
static void motor_set(Motor *self, float pwm)
{
	int v;
	
	//限幅
	if (pwm >  g_param.pwm_out_max) pwm =  g_param.pwm_out_max;
	if (pwm < -g_param.pwm_out_max) pwm = -g_param.pwm_out_max;
	
	//镜像安装的电机取负
	if (self->reverse) pwm = -pwm;
	
	//方向脚
	if (pwm < 0) {
		HAL_GPIO_WritePin(self->port, self->pin_a, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(self->port, self->pin_b, GPIO_PIN_SET);
	} else {
		HAL_GPIO_WritePin(self->port, self->pin_a, GPIO_PIN_SET);
		HAL_GPIO_WritePin(self->port, self->pin_b, GPIO_PIN_RESET);
	}
	
	//占空比
	v = (int)((pwm < 0) ? -pwm : pwm);
	__HAL_TIM_SET_COMPARE(self->tim, self->ch, (uint32_t)v);
}


//初始化用函数
void Motor_Init(Motor *self, TIM_HandleTypeDef *tim, uint32_t ch,
                GPIO_TypeDef *port, uint16_t pin_a, uint16_t pin_b,
                uint8_t reverse)
{
	self->tim     = tim;
	self->ch      = ch;
	self->port    = port;
	self->pin_a   = pin_a;
	self->pin_b   = pin_b;
	self->reverse = reverse;
	
	self->set = motor_set;			//绑定自己的函数指针
	self->set(self, 0);				//先停住
}


//初始化
void Motor_InitAll(void)
{
	// 左电机：PA5 / PA15，TIM2 CH3，方向不取负
	Motor_Init(&motor_left,  &htim2, TIM_CHANNEL_3,
	           GPIOA, GPIO_PIN_5,  GPIO_PIN_15, 0);
	
	// 右电机：PB14 / PB15，TIM2 CH4，镜像安装所以要取负
	Motor_Init(&motor_right, &htim2, TIM_CHANNEL_4,
	           GPIOB, GPIO_PIN_14, GPIO_PIN_15, 1);
	
}
