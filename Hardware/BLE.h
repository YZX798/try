#ifndef __BLE_H__
#define __BLE_H__

#include "main.h"
#include "usart.h"
#include <stdio.h>
#include "param.h"


void BLE_Send(float roll,float hope,float out,float speed);
int fputc(int ch, FILE *f);

#endif

