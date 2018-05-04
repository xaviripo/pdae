/******************************************************************************/
// INCLUDES
/******************************************************************************/

#include "robot.h"



/******************************************************************************/
// MAIN
/******************************************************************************/

void main(void)
{

    uint8_t ho;
    WDTCTL = WDTPW + WDTHOLD; // Paramos el watchdog timer
    init_ucs_24MHz(); // Inicalizar UCS
    init_timers();
    init_uart();
    init_interrupts();

    // Settings
    set_obstacle_threshold(200);

    // Initial actions
    write_led(LEFT, 0);
    write_led(RIGHT, 0);

    rotate_left(FORWARD, 300);
    rotate_right(FORWARD, 300);

    uint8_t claps = 0;
    for(;;) {

        if(read_claps() > 0) {
            claps++;
            write_led(claps%2==0 ? LEFT : RIGHT, 1);
            write_led(claps%2==0 ? RIGHT : LEFT, 0);
        }

        ho = read_obstacle();

        switch (ho) {
        case 0:
            rotate_left(FORWARD, 300);
            rotate_right(FORWARD, 300);
            break;
        case OBSTACLE_LEFT:
            rotate_left(FORWARD, 300);
            rotate_right(BACKWARD, 300);
            break;
        case OBSTACLE_RIGHT:
            rotate_left(BACKWARD, 300);
            rotate_right(FORWARD, 300);
            break;
        case OBSTACLE_CENTER:
        default:
            rotate_left(FORWARD, 0);
            rotate_right(FORWARD, 0);
        }
    }
}
