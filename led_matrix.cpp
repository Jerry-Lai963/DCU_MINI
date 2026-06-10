#include <Arduino.h>
#include <SPI.h>

#include "dash_state.h"
#include "pins.h"
#include "can_msg.h"
#include "led_matrix.h"
#include "rmc_calc.h"

#define SET_CH(B,CH) ((B) |= (1 << (8 - (CH+1))))

void led_matrix_can_update(dash_state_t* dash_state, vcu_142_t vcu_142_msg){
    for(uint8_t i = 0; i < LED_MATRIX_SZ; i++){
        //update from can msg
        dash_state->led_matrix_raw[i] = 0;
        //this line is controlling all the i/o, need to re adjust the order of the bits to match the actual can message
        /*dash_state->led_matrix_raw[i] = vcu_142_msg.park_light << 0 
                                        | vcu_142_msg.beam_l << 1 
                                        | vcu_142_msg.beam_h << 2 
                                        | vcu_142_msg.logo << 3 
                                        | vcu_142_msg.fog_r << 4 
                                        | vcu_142_msg.turn_l << 5 
                                        | vcu_142_msg.turn_r << 6 
                                        | vcu_142_msg.brake_err << 7
                                        | vcu_142_msg.handbrake << 8;*/
    }
    if(dash_state->powerdown_req){
        return;
    }

    // 595 #1: SOC gauge on byte[0] using lower-N LEDs pattern
    uint8_t soc_led_count = map_r(vcu_142_msg.bytes[0], 0, 0x64, 0, 7);
    dash_state->led_matrix_raw[0] = soc_led_count ? ((1u << soc_led_count) - 1) : 0;

    // 595 #2: water temperature gauge on byte[1] using lower-N LEDs pattern
    uint8_t temp_led_count = map_r(vcu_142_msg.bytes[1], 0, 0x80, 0, 7);
    dash_state->led_matrix_raw[1] = temp_led_count ? ((1u << temp_led_count) - 1) : 0;

    // 595 #3 and #4: manual mapping for the specific VCU and SR signals
    uint8_t raw3 = 0;
    uint8_t raw4 = 0;

    // helper: map SR channel number to the actual 74HC595 bit position

    // VCU signals
    // READY: Check byte 4, bit 7 (0x80) -> 595 #3 Channel 4
    if (vcu_142_msg.bytes[4] & 0x80) {
        SET_CH(raw3, 4);
    }

    // CHECK ENGINE: Check byte 4, bit 5 (0x20) -> 595 #4 Channel 1
    // Bit check: if byte[4] bit5 is 1, set the light
    if (vcu_142_msg.bytes[4] & 0x20) {
        SET_CH(raw4, 1);
    }

    // SR1 (0x126 12 01)
    uint8_t sr1 = dash_state->srpdu_raw[0];
    if(sr1 & 0x10) {
        // low beam -> 595 #3 ch5 + 595 #4 ch3
        SET_CH(raw3, 5);
        SET_CH(raw4, 3);
        digitalWrite(BL_PARK_EN, 1);
        digitalWrite(BL_5_EN, 1);
    }
    if(sr1 & 0x08) {
        // high beam -> 595 #3 ch3
        SET_CH(raw3, 3);
    }
    if(sr1 & 0x20) {
        // rear fog -> 595 #3 ch7
        SET_CH(raw3, 7);
    }
    if(sr1 & 0x80) {
        // brake fluid -> 595 #4 ch4 + ch1
        //SET_CH(raw4, 1);
        SET_CH(raw4, 4);
        
    }

    // SR2 (0x126 12 02)
    uint8_t sr2 = dash_state->srpdu_raw[1];
    if(sr2 & 0x04) {
        // left indicator -> 595 #3 ch2
        SET_CH(raw3, 2);
    }
    if(sr2 & 0x02) {
        // right indicator -> 595 #3 ch6
        SET_CH(raw3, 6);
    }
    // hazard -> 595 #4 ch2 (check if both left and right indicators are on)
    if((sr2 & 0x04) && (sr2 & 0x02)) {
        SET_CH(raw4, 2);
    }

    dash_state->led_matrix_raw[2] = raw3;
    dash_state->led_matrix_raw[3] = raw4;

        
    /*if(!(sr1 & 0x10)) {
        digitalWrite(BL_PARK_EN, 0);
        digitalWrite(BL_5_EN, 0);
    }*/

    led_matrix_send(dash_state);
}

void led_matrix_update(dash_state_t* dash_state){
}

void led_matrix_send(dash_state_t* dash_state) {
    
    SPCR0 = 0x03 | (SPCR0 & 0xFC); // f/64
    SPSR0 |= 0x01;

    digitalWrite(LED_LATCH, 0);
    digitalWrite(LED_RST, 0);
    digitalWrite(LED_RST, 1);
    //digitalWrite(LED_EN, 1); // comment out later
    digitalWrite(BL_5_EN, 1);

    for(int i = LED_MATRIX_SZ - 1; i >= 0; i--) {
        SPI.transfer(dash_state->led_matrix_raw[i]);
        //SPI.transfer(0XFF);
    }

    digitalWrite(LED_LATCH, 1);
    digitalWrite(LED_EN, 0);

    SPCR0 = 0x00 | (SPCR0 & 0xFC); // f/2

}

void led_matrix_test_pattern(dash_state_t* dash_state) {
    // Ensure the hardware output is enabled
    digitalWrite(LED_EN, 0); 
    digitalWrite(BL_5_EN, 1);

    // ==========================================
    // PHASE 1: Sequential Channel Scan (On -> Off)
    // ==========================================
    // Loop through all 4 shift registers
    for (int chip = 0; chip < LED_MATRIX_SZ; chip++) {
        // Loop through all 8 channels of the current 74HC595 chip (0 to 7)
        for (int ch = 0; ch < 8; ch++) {
            
            // Clear all registers first
            for (int i = 0; i < LED_MATRIX_SZ; i++) {
                dash_state->led_matrix_raw[i] = 0x00; 
            }

            // Turn on ONLY the current channel using your macro
            SET_CH(dash_state->led_matrix_raw[chip], ch);

            // Push to the physical hardware
            led_matrix_send(dash_state);
            delay(150); // Visible delay to trace the path
        }
    }

    // ==========================================
    // PHASE 2: Waterfall Cascade Fill
    // ==========================================
    // Reset all states back to zero
    for (int i = 0; i < LED_MATRIX_SZ; i++) {
        dash_state->led_matrix_raw[i] = 0x00;
    }

    for (int chip = 0; chip < LED_MATRIX_SZ; chip++) {
        for (int ch = 0; ch < 8; ch++) {
            // Accumulate bits on the register without clearing previous channels
            SET_CH(dash_state->led_matrix_raw[chip], ch);
            led_matrix_send(dash_state);
            delay(100);
        }
    }
    
    delay(1000); // Hold everything on for 1 second before releasing
}
#undef SET_CH