#include <Arduino.h>

#include "can.h"
#include "mcp2515.h"

#include "dash_state.h"
#include "can_msg.h"


#include "wheel_speed.h"

void wheel_speed_update(dash_state_t* dash_state, MCP2515* can_ch){

    can_frame tx_msg;
    dcu_145_t* speed_msg;
    uint16_t adc_res = 0;

    tx_msg.can_id = 0x145;
    tx_msg.can_dlc = 8;
    speed_msg = (dcu_145_t*) tx_msg.data;
    
    for(int i = 0; i < 3; i++){
        adc_res = analogRead(PIN_A0 + i);

        //TODO: Calibrate
        dash_state->wheelspeed[i] = map(adc_res,0,1024,0,3000); 

    }



    tx_msg.data[0] = dash_state->charge_state;
    tx_msg.data[1] = dash_state->car_soc;

    can_ch->sendMessage(&tx_msg);
}