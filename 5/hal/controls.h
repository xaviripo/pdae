#ifndef HAL_CONTROLS_H_
#define HAL_CONTROLS_H_

/******************************************************************************/
// INTERFAZ
/******************************************************************************/

void init_controls_0(void);
void init_controls_1(void);



/******************************************************************************/
// EXTERN
/******************************************************************************/

extern void s1_pressed(void); // To be implemented in main.c
extern void s2_pressed(void); // To be implemented in main.c
extern void up_pressed(void); // To be implemented in main.c
extern void down_pressed(void); // To be implemented in main.c
extern void left_pressed(void); // To be implemented in main.c
extern void right_pressed(void); // To be implemented in main.c
extern void center_pressed(void); // To be implemented in main.c

#endif /* HAL_CONTROLS_H_ */
