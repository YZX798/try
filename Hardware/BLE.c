#include "BLE.h"

#pragma import(__use_no_semihosting)

struct __FILE { int handle; };
FILE __stdout;

void _sys_exit(int x) { (void)x; }
void _ttywrch(int ch)    { (void)ch; }

int fputc(int ch, FILE *f)
{
    HAL_UART_Transmit(&huart3, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
    return ch;
}

void BLE_Send(float roll,float hope,float out,float speed)
{
		printf("%f,%f,%f,%f\n",roll ,hope,out,speed);
}
