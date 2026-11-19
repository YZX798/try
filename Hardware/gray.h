#ifndef __GRAY_H__
#define __GRAY_H__

#include "main.h"

#define GRAY_True		0
#define GRAY_Flase		1

typedef struct GRAY GRAY;
struct GRAY
{
	GPIO_TypeDef *GPIO[5];
	uint16_t	  Pin[5];
	
	int led1, led2, led3, led4, led5;
	int sum_num;
	int weighted_sum;
	int ALL_Num;
	int gray_return;

	int (*Gary_BACK)(GRAY *self);
};

void Gray_Init(GRAY *self);
extern GRAY g_gray;

#endif
