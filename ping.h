//
//#include <stdint.h>
//#include <stdbool.h>
//#include <inc/tm4c123gh6pm.h>
//#include "driverlib/interrupt.h"
//
////Initialize the PING sensor
//void ping_init(void);
//
////Read in the ping value
//double ping_read(void);
//
////Send out a pulse on PB3
//void send_pulse();
//
////ISR: Capture rising edge and falling edge time of PING sensor
//void TIMER3B_Handler(void);
//
////Returns the pulse length
//int get_pulse();
//
////Returns the pulse in milliseconds
//double get_pulse_millis();
//
////Returns the overflow
//int get_overflow();

/**
 * Driver for ping sensor
 * @file ping.c
 * @author
 */
#ifndef PING_H_
#define PING_H_

#include <stdint.h>
#include <stdbool.h>
#include <inc/tm4c123gh6pm.h>
#include "driverlib/interrupt.h"

/**
 * Initialize ping sensor. Uses PB3 and Timer 3B
 */
void ping_init (void);

/**
 * @brief Trigger the ping sensor
 */
void ping_trigger (void);

/**
 * @brief Timer3B ping ISR
 */
void TIMER3B_Handler(void);

/**
 * @brief Calculate the distance in cm
 *
 * @return Distance in cm
 */
float ping_getDistance (void);

#endif /* PING_H_ */
