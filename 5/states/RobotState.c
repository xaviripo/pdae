#include "states/RobotState.h"

void f() {}

void RobotState__set_defaults(RobotState *r) {
	r->init = &f;
	r->exit = &f;
	r->update = &f;
	r->draw_screen = &f;
	r->s2_pressed = &f;
	r->s1_pressed = &f;
	r->up_pressed = &f;
	r->down_pressed = &f;
	r->left_pressed = &f;
	r->right_pressed = &f;
	r->center_pressed = &f;
}
