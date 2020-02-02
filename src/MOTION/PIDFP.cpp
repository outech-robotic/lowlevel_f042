/*
 * PID.cpp
 *
 *  Created on: 8 déc. 2019
 *      Author: ticta
 */

#include <MOTION/PIDFP.h>

#define COEFF_SHIFT       (16)
#define COEFF_SIZE        (32)
#define COEFF_MULTIPLIER  ((1ULL)<<COEFF_SHIFT)
#define COEFF_MAX         (((1ULL)<<COEFF_SIZE)-1)>>COEFF_SHIFT

PID_FP::PID_FP() {
  reset();
}

void PID_FP::reset(){
  last_error = 0;
  last_setpoint = 0;
  integral_sum = 0;
}


void PID_FP::set_coefficients(float new_kp, float new_ki, float new_kd, uint32_t new_freq){
  kp = new_kp * COEFF_MULTIPLIER;
  kd = new_kd * COEFF_MULTIPLIER * new_freq;
  ki = new_ki * COEFF_MULTIPLIER / new_freq;
}

void PID_FP::set_coefficients(uint32_t new_kp, uint32_t new_ki, uint32_t new_kd){
  kp = new_kp;
  kd = new_kd;
  ki = new_ki;
}

void PID_FP::set_kp(uint32_t new_kp){
  kp = new_kp;
}

void PID_FP::set_ki(uint32_t new_ki){
  ki = new_ki;
}

void PID_FP::set_kd(uint32_t new_kd){
  kd = new_kd;
}



void PID_FP::set_output_limit(int32_t new_limit){
  min = -new_limit*COEFF_MULTIPLIER;
  max = new_limit*COEFF_MULTIPLIER;
}


void PID_FP::set_anti_windup(int32_t new_limit){
  integral_min = -new_limit*COEFF_MULTIPLIER;
  integral_max = new_limit*COEFF_MULTIPLIER;
}


void PID_FP::set_derivative_limit(int32_t new_limit){
  derivative_min = - new_limit*COEFF_MULTIPLIER;
  derivative_max = new_limit*COEFF_MULTIPLIER;
}


void PID_FP::get_coefficients(float* ret_kp, float* ret_ki, float* ret_kd){
  *ret_kp = kp;
  *ret_ki = ki;
  *ret_kd = kd;
}


int16_t PID_FP::compute(int32_t input, int32_t setpoint){
  int32_t res;
  int16_t out;
  int32_t delta_err;
  int32_t error = setpoint-input;
  int64_t p=0,i=0,d=0;
  if(kp){
    p = kp * error;
  }
  if(ki){
    integral_sum += (int64_t)ki * (int64_t)error;
    if(integral_sum>integral_max)
      integral_sum = integral_max;
    else if(integral_sum < integral_min)
      integral_sum = integral_min;
    i = integral_sum;
  }
  if(kd){
    delta_err = (error-last_error) - (setpoint - last_setpoint);
    last_setpoint = setpoint;
    d = kd * delta_err;
    if(d>integral_max)
      d = integral_max;
    else if(d < integral_min)
      d = integral_min;
    last_error = error;
  }
  res = p+i+d;
  // Saturation
  if(res>max)
    res = max;
  else if(res<min)
    res = min;

  out = res>>COEFF_SHIFT;
  if(res& (1ULL << (COEFF_SHIFT - 1)))
    out++;

  return out;
}
