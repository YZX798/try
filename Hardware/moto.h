#ifndef __MOTO_H
#define __MOTO_H

#include "tim.h"
#include "param.h"


typedef struct Motor Motor;

struct Motor
{
	//数据
	TIM_HandleTypeDef *tim;        // 用哪个定时器出 PWM
	uint32_t            ch;        // 哪个通道
	GPIO_TypeDef       *port;      // 方向脚所在端口
	uint16_t            pin_a;     // 方向脚 A
	uint16_t            pin_b;     // 方向脚 B
	uint8_t             reverse;   // 1 = 这个电机镜像安装，方向要取负

	void (*set)(Motor *self, float pwm);
};

extern Motor motor_left;
extern Motor motor_right;

void Motor_Init(Motor *self, TIM_HandleTypeDef *tim, uint32_t ch,
                GPIO_TypeDef *port, uint16_t pin_a, uint16_t pin_b,
                uint8_t reverse);

void Motor_InitAll(void);

#endif
