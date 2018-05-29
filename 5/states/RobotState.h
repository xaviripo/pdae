#ifndef ROBOTSTATE_H_
#define ROBOTSTATE_H_

typedef struct RobotState
{
	void (*init) ();
	void (*exit) ();

	// update engine (se llama en el bucle principal)
	void (*update) ();

	// screen
	void (*draw_screen) ();

	// controls
	void (*s2_pressed) ();
	void (*s1_pressed) ();
	void (*up_pressed) ();
	void (*down_pressed) ();
	void (*left_pressed) ();
	void (*right_pressed) ();
	void (*center_pressed) ();
} RobotState;

void RobotState__set_defaults(RobotState* r);

#endif
