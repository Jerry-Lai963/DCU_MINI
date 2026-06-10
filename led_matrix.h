#ifndef LED_MATRIX_H
#define LED_MATRIX_H

#include "dash_state.h"

#define LED_MATRIX_SZ 4

enum led_matrix_table{

    LED_GEAR_R,
    LED_CHECK,
    LED_GEAR_N,
    LED_BRAKE_ERR,
    LED_GEAR_D,
    LED_NC_1,
    LED_NC_2,
    LED_NC_3, //595 3

    LED_FOG_R,
    LED_TURN_L,
    LED_BEAM_H,
    LED_READY,
    LED_BEAM_L,
    LED_TURN_R,
    LED_FOG_F,
    LED_NC_4 //595 4
};




#define LED_MATRIX_SET(ARR,ID,STATE) do{ARR[ID/8] |= (STATE & 0x01) << (7 - ((ID) % 8));} while(0) 

void led_matrix_update(dash_state_t* dash_state);

void led_matrix_can_update(dash_state_t* dash_state, vcu_142_t vcu_142_msg);

void led_matrix_test_pattern(dash_state_t* dash_state);

void led_matrix_send(dash_state_t* dash_state);

#endif // LED_MATRIX_H