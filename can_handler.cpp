#include <Arduino.h>
#include <can.h>

#include "dash_state.h"
#include "can_msg.h"
#include "can_handler.h"

void vcu_140_handler(dash_state_t *dash_state, can_frame rx_msg) {
    vcu_140_t *msg = (vcu_140_t *)rx_msg.data;

    dash_state->car_state = msg->car_state;
    dash_state->car_gear = msg->car_gear;
    dash_state->car_speed = msg->car_speed;
    dash_state->motor_rpm = msg->motor_rpm;
    dash_state->car_odo = msg->car_odo;

}

void vcu_142_handler(dash_state_t *dash_state, can_frame rx_msg) {
    vcu_142_t *msg = (vcu_142_t *)rx_msg.data;

    dash_state->car_soc = msg->car_soc;
    dash_state->motor_temp = msg->motor_temp - 40; // Adjusting for signed value
    dash_state->battery_temp = msg->battery_temp - 40; // Adjusting for signed value

    // Copy LED matrix data
    for (int i = 0; i < 3; i++) {
        //dash_state->led_matrix[i] = msg->led_matrix[i];
    }
}