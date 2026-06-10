#include <Arduino.h>

#include "dash_state.h"
#include "pins.h"

void dash_state_update(dash_state_t* dash_state){
    if((uint32_t)(millis() - dash_state->state_timeout) >= 500){
        dash_state->car_state = CAR_STATE_IDLE;
        dash_state->charge_state = 0;
    }

    dash_state->powerdown_req = 0;
    if(dash_state->car_state == CAR_STATE_IDLE){

        if(dash_state->charge_gun == 0)
            dash_state->powerdown_req = 1;

    }

}