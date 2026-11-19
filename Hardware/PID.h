#ifndef __PID_H__
#define __PID_H__

#include "main.h"
#include "param.h"


void Control_Mix(PID *stand, PID *speed, PID *turn,float *PWM_Base_out,float *PWM_L_OUT,float *PWM_R_OUT);
void Limit_Base(float *PWM_Base);
void PID_Bind(PID *self, float *kp, float *ki, float *kd,
              float *filt, float *limit, float *sum_limit);
void PID_InitAll(void);

#endif
