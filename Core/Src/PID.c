/*
 * PID.c
 *
 *  Created on: Dec 24, 2025
 *      Author: Admin
 */
#include "PID.h"
#include "main.h"
#include <stdint.h>

uint32_t power;
/**
 * @brief Initializes PID controller parameters.
 *
 * This function sets the proportional, integral, and derivative gains
 * along with the required setpoint. It also resets integral and previous
 * error values.
 *
 * @param H1          Pointer to PIDController structure.
 * @param kp          Proportional gain.
 * @param ki          Integral gain.
 * @param kd          Derivative gain.
 * @param setpoint    Desired target value.
 */
void PID_Decision(PIDController *H1, float kp, float ki, float kd, float setpoint)
{
    H1->kp = kp;
    H1->ki = ki;
    H1->kd = kd;
    H1->setpoint = setpoint;
    H1->previous_error = 0;
    H1->integral = 0;
}

/**
 * @brief Computes standard PID output (0–1000 range).
 *
 * This function performs a full PID calculation based on current value and time
 * difference. Output is clamped between 0 and 1000 (PWM-compatible).
 *
 * @param H1             Pointer to PIDController structure.
 * @param current_value  Measured feedback value.
 * @param delta_time     Time difference between PID updates in seconds.
 *
 * @return uint32_t      PID output (0 to 1000).
 */
uint32_t PID_Compute_for_Heater_Power(PIDController *H1, float current_value, float delta_time)
{
    // Calculate error
    float error = H1->setpoint - current_value;

    // Calculate integral and derivative terms
    H1->integral += error * delta_time;
    float derivative = (error - H1->previous_error) / delta_time;

    // Compute PID output
    H1->output = (H1->kp * error) + (H1->ki * H1->integral) + (H1->kd * derivative);

    // Clamp output to 0–1000
    if (H1->output > 1023)
        H1->output = 1023;
    else if (H1->output < 0)
        H1->output = 0;

    // Update global power value
    power = H1->output;

    // Save current error for next cycle
    H1->previous_error = error;

    return H1->output;
}

