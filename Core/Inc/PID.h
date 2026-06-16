/*
 * PID.h
 *
 *  Created on: Dec 24, 2025
 *      Author: Admin
 */

#ifndef INC_PID_H_
#define INC_PID_H_

#include <stdint.h>

typedef struct
{
    float kp;
    float ki;
    float kd;
    float setpoint;
    float previous_error;
    float integral;
    int output;

} PIDController;


void PID_Decision(PIDController *H1, float kp, float ki, float kd, float setpoint);
//uint32_t PID_Compute(PIDController *H1, float current_value, float delta_time);
uint32_t PID_Compute_for_Heater_Power(PIDController *H1, float current_value, float delta_time);

#endif /* INC_PID_H_ */
