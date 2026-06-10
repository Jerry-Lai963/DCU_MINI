#ifndef CAN_HANDLER_H
#define CAN_HANDLER_H

#include <can.h>
#include "can_msg.h"
#include "dash_state.h"

void vcu_140_handler(dash_state_t *dash_state, can_frame rx_msg);
void vcu_142_handler(dash_state_t *dash_state, can_frame rx_msg);

#endif