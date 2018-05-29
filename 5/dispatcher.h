/*
 * dispatcher.h
 *
 *  Created on: May 29, 2018
 *      Author: kanales
 */

#ifndef DISPATCHER_H_
#define DISPATCHER_H_

#include <stdint.h>
#include "RobotStates/RobotState.h"

void setState(RobotState *s);
void set_thresholds(uint8_t left, uint8_t front, uint8_t right);
uint8_t get_thr_left();
uint8_t get_thr_front();
uint8_t get_thr_right();


#endif /* DISPATCHER_H_ */
