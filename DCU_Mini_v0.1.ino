/*
#                       _oo0oo_
#                      o8888888o
#                      88" . "88
#                      (| -_- |)
#                      0\  =  /0
#                    ___/`---'\___
#                  .' \\|     |# '.
#                 / \\|||  :  |||# \
#                / _||||| -:- |||||- \
#               |   | \\\  -  #/ |   |
#               | \_|  ''\---/''  |_/ |
#               \  .-\__  '-'  ___/-. /
#             ___'. .'  /--.--\  `. .'___
#          ."" '<  `.___\_<|>_/___.' >' "".
#         | | :  `- \`.;`\ _ /`;.`/ - ` : | |
#         \  \ `_.   \_ __\ /__ _/   .-` /  /
#     =====`-.____`.___ \_____/___.-`___.-'=====
#                       `=---='
#
#
#     ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#
#               佛祖保佑         永无BUG
#
#
*/
#include <SPI.h>
#include <Wire.h>

#include <can.h>
#include <mcp2515.h>

#include <EEPROM.h>

#include "dash_state.h"
#include "can_msg.h"
#include "pins.h"

#include "wheel_speed.h"
#include "odo.h"

#include "led_matrix.h"

#include "oled.h"

#include "gauge.h"

#include "rmc_calc.h"

//#include "rmc_calc.h"

/* GLOBAL VARIABLES*/

dash_state_t main_dash_state;

enum GAUGE_TYPE{
    TYPE_SPEEDO,
    TYPE_RPM
};

int8_t gauge_speed;
uint16_t max_val;
uint8_t powerdown;
uint8_t gauge_config;
extern uint16_t last_gauge_val;

MCP2515 can0(CAN_CS1);

vcu_140_t* vcu_140_msg;
vcu_142_t* vcu_142_msg;
dcu_145_t* dcu_145_wheelspeed;
vcu_146_t* vcu_146_msg;

vcu_14a_t* vcu_14a_msg;

//millis timer
uint32_t timer_10ms;
uint32_t timer_50ms;
uint32_t timer_100ms;
uint32_t timer_1000ms;
uint32_t timer_srpdu;
uint16_t gauge_val;
int32_t timer_cnt;

uint32_t state_timer = 0;

uint8_t boot = 0;
uint32_t power_timer = 0;

uint32_t odo = 0;

uint8_t flasher = 0; //poop mountain

/* END GLOBAL VARIABLES*/

/*int32_t clip(int32_t in, int32_t min, int32_t max){

    if(in < min) return min;
    if(in > max) return max;
    return in;

}*/


void port_init(){

    pinMode(WAKEUP_IN, INPUT);
    pinMode(WAKEUP_PWR_OUT, OUTPUT);

    pinMode(CAN_CS1, OUTPUT);

    pinMode(LED_LATCH, OUTPUT);
    pinMode(LED_RST, OUTPUT);
    pinMode(LED_EN, OUTPUT);

    digitalWrite(LED_EN, 1);

    pinMode(OLED_CS, OUTPUT);
    pinMode(OLED_RES, OUTPUT);
    pinMode(OLED_DC, OUTPUT);
    pinMode(OLED_OE, OUTPUT);

    // OLED2 (SSD1315 128x64) pin initialization
    /*pinMode(OLED2_CS, OUTPUT);
    pinMode(OLED2_RES, OUTPUT);
    pinMode(OLED2_DC, OUTPUT);*/
    
    pinMode(BL_5_EN, OUTPUT);
    pinMode(BL_12_EN, OUTPUT);
    pinMode(BL_PARK_EN, OUTPUT);
    
    pinMode(BUZZER_OUT, OUTPUT);

    digitalWrite(WAKEUP_PWR_OUT, 1);
    digitalWrite(BL_12_EN, 0);
    digitalWrite(BL_5_EN, 1);
    digitalWrite(BL_PARK_EN, 1);
    digitalWrite(OLED_CS, 1);
    digitalWrite(OLED_OE, 1);
    //led testing, delete this later
   /** digitalWrite(LED_LATCH, 1);
    digitalWrite(PIN_PB7, 1); //clk
    digitalWrite(PIN_PB6, 1); //mosi*/

}


void setup(){

    gauge_config = TYPE_SPEEDO;

    for (uint16_t i = 0; i < 666; i++){
        _NOP();
    }

    port_init();

    SPI.begin();

    oled_init();

    //main_dash_state.srpdu_drive = EEPROM.read(0x01);
    
    can0.reset();
    can0.setBitrate(CAN_500KBPS,MCP_20MHZ);
    can0.setNormalMode();
    
    led_matrix_test_pattern(&main_dash_state);


    timer_10ms = 0;
    timer_50ms = 0;
    timer_100ms = 0;
    timer_1000ms = 0;

    main_dash_state.car_odo = odo_eeprom_read();


    for(int i = 0; i < LED_MATRIX_SZ; i++){
        main_dash_state.led_matrix_raw[i] = 0x00;
    }
    led_matrix_send(&main_dash_state);

    main_dash_state.srpdu_raw[0] = 0;
    main_dash_state.srpdu_raw[1] = 0;
    main_dash_state.srpdu_raw[2] = 0;

    gauge_val = 0;
    powerdown = 0;


}

void loop(){
    uint32_t currTick = millis();
    
    can_frame can0_rx_msg;
    can_frame can1_rx_msg;
    can_frame tx_msg;

    if((uint32_t)(currTick - timer_10ms) >= 10){
        //10ms routine
        update_gauge(gauge_val);
        timer_10ms = currTick;
    }

    if((uint32_t)(currTick - timer_50ms) >= 50){
        //500ms routine
        tx_msg.can_id = 0x691;
        tx_msg.can_dlc = 1;
        tx_msg.data[0] = main_dash_state.srpdu_drive; //SRPDU alive signal

        can0.sendMessage(&tx_msg);

       
        

        timer_50ms = currTick;
    }

    if((uint32_t)(currTick - timer_100ms) >= 100){
        //100ms routine
        //led_matrix_can_update(&main_dash_state, *vcu_142_msg);
        wheel_speed_update(&main_dash_state, &can0);
        led_matrix_send(&main_dash_state);
        
        dash_state_update(&main_dash_state);

        //TESTING PURPOSES ONLY, DELETE THIS LATER
        /*int led_test = 0x00;
        for(int i = 0; i < 16; i++){
            led_test |= (0x01 << i);
            digitalWrite(LED_LATCH, 0);
            shiftOut(PIN_PB6, PIN_PB7, MSBFIRST, led_test);
            digitalWrite(LED_LATCH, 1);
            delay(50);
        }*/
        
        // Door warning with continuous beeping while door is open
        uint8_t door_open_current = (main_dash_state.srpdu_raw[0] & 0x40) != 0;
        
        // While door is open, beep continuously
        if(door_open_current) {
            // Beep pattern: 200ms on, 200ms off, repeat
            uint32_t buzzer_cycle = (currTick / 200) % 2;
            if(buzzer_cycle == 0) {
                digitalWrite(BUZZER_OUT, 1);
                delay(50);
            } else {
                digitalWrite(BUZZER_OUT, 0);
            }
        } else {
            // Door closed - stop buzzer
            digitalWrite(BUZZER_OUT, 0);
        }
        
        // Update previous state for next cycle
        main_dash_state.door_open_prev = door_open_current;
        
        // Handle other warnings
        if(main_dash_state.have_warn && main_dash_state.car_state && !main_dash_state.door_buzzer_active){
            digitalWrite(BUZZER_OUT, !digitalRead(BUZZER_OUT));
        }
        
        
        oled_update(&main_dash_state);

        if(!main_dash_state.door_buzzer_active && !(main_dash_state.have_warn && main_dash_state.car_state)){
            digitalWrite(BUZZER_OUT, 0);
        }
        if(boot){
            digitalWrite(BUZZER_OUT, 1);
        }

        if(digitalRead(WAKEUP_IN)){
            //main_dash_state.powerdown_req = 0;
        }

        tx_msg.can_id = 0x141;
        tx_msg.can_dlc = 8;
        tx_msg.data[0] = main_dash_state.car_odo & 0xFF;
        tx_msg.data[1] = (main_dash_state.car_odo >> 8) & 0xFF;
        tx_msg.data[2] = (main_dash_state.car_odo >> 16) & 0xFF;
        tx_msg.data[3] = (main_dash_state.car_odo >> 24) & 0xFF;
        can0.sendMessage(&tx_msg);

        
        odo_eeprom_write(main_dash_state.car_odo);


        /*
        if((!digitalRead(WAKEUP_IN)) && !main_dash_state.powerdown_req){
            power_timer = currTick;
            main_dash_state.powerdown_req = 1;
        }

        if(main_dash_state.powerdown_req){
            if(currTick - power_timer > 5000){
                digitalWrite(WAKEUP_PWR_OUT, 0);
                for(uint8_t i = 0; i < LED_MATRIX_SZ; i++){
                    main_dash_state.led_matrix_raw[i] = 0x00;
                }
                led_matrix_send(&main_dash_state);
            }
        }*/

        gauge_speed = 10;
        powerdown = 0;
        switch(gauge_config){
            case TYPE_SPEEDO:{
                max_val = S_GAUGE_MAX;
                
                gauge_val = map_r(main_dash_state.car_speed, 0, 160, 0, S_GAUGE_MAX);
                if(main_dash_state.car_state == CAR_STATE_IDLE){
                   powerdown = 1;
                }
                break;
            }
            case TYPE_RPM:{
                gauge_val = main_dash_state.motor_rpm * 4;
                max_val = B_GAUGE_MAX;
                break;
            }
        }

        boot = 0;
        timer_100ms = currTick;
    }

    if((uint32_t)(currTick - timer_1000ms) >= 1000){

        flasher = !flasher;

        timer_1000ms = currTick;
    }


    if(can0.readMessage(&can0_rx_msg) == MCP2515::ERROR_OK){
        switch(can0_rx_msg.can_id){
            case 0x140:{
                //some update stuff
                uint32_t tmp_odo;


                vcu_140_msg = (vcu_140_t*)can0_rx_msg.data;

                if((main_dash_state.car_state == CAR_STATE_IDLE) && (vcu_140_msg->car_state == CAR_STATE_IGN)){
                    boot = 1;
                    digitalWrite(BUZZER_OUT, 1);
                }

                main_dash_state.car_state = vcu_140_msg->car_state;
                main_dash_state.car_gear = vcu_140_msg->car_gear;
                main_dash_state.car_speed = vcu_140_msg->car_speed;
                main_dash_state.motor_rpm = vcu_140_msg->motor_rpm;
                tmp_odo = vcu_140_msg->car_odo;

                if(tmp_odo > main_dash_state.car_odo){
                    main_dash_state.car_odo = tmp_odo;
                    odo_eeprom_write(main_dash_state.car_odo);
                }

                //main_dash_state.car_odo = can0_rx_msg.data[4] | (can0_rx_msg.data[5] << 8) | (can0_rx_msg.data[6] << 16);

                main_dash_state.state_timeout = currTick;
                
                break;
            }

            case 0x142:{
                //some update stuff
                vcu_142_t local_vcu_msg;
                for(uint8_t i = 0; i < 8; i++) {
                    local_vcu_msg.bytes[i] = can0_rx_msg.data[i];
                }
                main_dash_state.car_soc      = local_vcu_msg.bytes[0];
                main_dash_state.motor_temp   = local_vcu_msg.bytes[1];
                main_dash_state.battery_temp = local_vcu_msg.bytes[2];

                // 4. Pass the safely structured data variable into your illumination function
                led_matrix_can_update(&main_dash_state, local_vcu_msg);
                /*vcu_142_msg = (vcu_142_t*)can0_rx_msg.data;
                main_dash_state.car_soc = vcu_142_msg->car_soc;
                main_dash_state.motor_temp = vcu_142_msg->motor_temp;
                main_dash_state.battery_temp = vcu_142_msg->battery_temp;

                led_matrix_can_update(&main_dash_state,*vcu_142_msg);
                /*for(int i = 0; i < LED_MATRIX_SZ; i++){
                    main_dash_state.led_matrix[i] = can0_rx_msg.data[3];
                }*/
                main_dash_state.led_timeout = currTick;

                break;
                
            }

            case 0x146:{

                main_dash_state.power_percent = (uint16_t)( can0_rx_msg.data[2]) - 20;

                main_dash_state.range_left = can0_rx_msg.data[4] | (can0_rx_msg.data[5] << 8);
                main_dash_state.power_level = can0_rx_msg.data[6] & 0x0F;

                break;
            }

            case 0x14A:{

                vcu_14a_msg = (vcu_14a_t*) can0_rx_msg.data;

                main_dash_state.shift_n = vcu_14a_msg->shift_n;
                main_dash_state.door_open = vcu_14a_msg->door_open;
                main_dash_state.charge_gun = vcu_14a_msg->charge_gun;
                main_dash_state.press_brake = vcu_14a_msg->press_brake;

                main_dash_state.have_warn = (   can0_rx_msg.data[0] > 0);
                
                break;
            }

            //SRPDU compatibility


            case 0x126:{
                uint8_t srpdu_raw_id = 0;

                timer_srpdu = currTick;
                switch((uint16_t)((can0_rx_msg.data[0] << 8) | (can0_rx_msg.data[1]))){

                    case 0x1201:{ //P8-1
                        srpdu_raw_id = 0;
                        break;
                    }
                    
                    case 0x1202:{ //P8-2
                        srpdu_raw_id = 1;
                        break;
                    }

                }
                
                main_dash_state.led_timeout = currTick;
                main_dash_state.srpdu_raw[srpdu_raw_id] = can0_rx_msg.data[2];

                break;

            }//*/

            //new srpdu definition compatibility
            case 0x131:{
                main_dash_state.srpdu_raw[0] = can0_rx_msg.data[2];
                break;
            }
            case 0x133:{
                main_dash_state.srpdu_raw[1] = can0_rx_msg.data[2];
                break;
            }
            case 0x135:{
                main_dash_state.srpdu_raw[2] = can0_rx_msg.data[2];
                break;
            }
        } 
    }
}