/*
 * dispatcher.h
 *
 *  Created on: May 29, 2018
 *      Author: kanales
 */

#ifndef DISPATCHER_H_
#define DISPATCHER_H_

#include <stdint.h>
#include "states/RobotState.h"

void set_state(RobotState *s);
void set_thresholds(uint8_t left, uint8_t front, uint8_t right);
uint8_t get_thr_left();
uint8_t get_thr_front();
uint8_t get_thr_right();

void set_thr_left(uint8_t value);
void set_thr_front(uint8_t value);
void set_thr_right(uint8_t value);


#endif /* DISPATCHER_H_ */
