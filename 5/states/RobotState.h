#ifndef ROBOTSTATE_H_
#define ROBOTSTATE_H_

#include "../common.h"

typedef struct RobotState
{
	void (*init) ();
	void (*exit) ();

	// update engine (se llama en el bucle principal)
	void (*update) ();

	// screen
	void (*draw_screen) ();
	bool screen_changed;

	// controls
	void (*s2_pressed) ();
	void (*s1_pressed) ();
	void (*up_pressed) ();
	void (*down_pressed) ();
	void (*left_pressed) ();
	void (*right_pressed) ();
	void (*center_pressed) ();
} RobotState;

typedef struct StateContext {
    // callibration
    uint8_t thr_left;
    uint8_t thr_front;
    uint8_t thr_right;
} StateContext;

void RobotState__set_defaults(RobotState* r);

#endif
