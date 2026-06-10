#include <U8g2lib.h>


#include "dash_state.h"
#include "pins.h"

// OLED display dimensions (SH1106 - ch1116 compatible: 128x64)
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// Reset pin for the OLED (can be set to -1 if not used)

// SPI pins

/*
#define OLED_CS    PIN_PB1 // Chip Select pin
#define OLED_DC    PIN_PC7  // Data/Command pin
#define OLED_RESET PIN_PC6
#define OLED_OE    PIN_PD4*/
//WARNING: VERY NOT FIRM NEED TO CHECK SCHEME @MARCO
// Create an instance of the display for SPI

//U8G2_SSD1327_EA_W128128_1_4W_HW_SPI display(U8G2_R0 /* cs=*/ OLED_CS, /* dc=*/ OLED_DC, /* reset=*/ OLED_RESET);
//U8G2_SSD1305_128X32_NONAME_1_4W_HW_SPI display(U8G2_R2, /* cs=*/ OLED_CS, /* dc=*/ OLED_DC, /* reset=*/ OLED_RES);

// SSD1315 128x64 Display instance for OLED2
 U8G2_SH1106_128X64_NONAME_1_4W_HW_SPI display(U8G2_R2, /* cs=*/ OLED_CS, /* dc=*/ OLED_DC, /* reset=*/ OLED_RES);

// Initialize OLED
/*void oled_init() {

    pinMode(OLED_CS, OUTPUT);
    pinMode(OLED_DC, OUTPUT);
    pinMode(OLED_RES, OUTPUT);
    pinMode(OLED_OE, OUTPUT);

    /*digitalWrite(OLED_OE, LOW);
    digitalWrite(OLED_CS, HIGH);
    digitalWrite(OLED_DC, HIGH);
    digitalWrite(OLED_RES, HIGH);
    delay(1);
    digitalWrite(OLED_RES, LOW);//
    digitalWrite(OLED_RES, HIGH);
    delay(10);
    digitalWrite(OLED_RES, LOW);  // Pull down to trigger chip reset
    delay(50);                     // SSD1305 requires a stable low pulse duration
    digitalWrite(OLED_RES, HIGH); // CRITICAL: Must be HIGH for the display to operate!
    delay(50);                     // Let the charge pump stabilize

    display.begin();
    display.clearBuffer();
    display.sendBuffer();
}*/

// Initialize SSD1315 OLED2 (128x64 2.23")

void oled_init() {
    pinMode(OLED_CS, OUTPUT);
    pinMode(OLED_DC, OUTPUT);
    pinMode(OLED_RES, OUTPUT);

    digitalWrite(OLED_RES, HIGH);
    delay(10);
    digitalWrite(OLED_RES, LOW);  // Pull down to trigger chip reset
    delay(50);                      // SSD1315 requires a stable low pulse duration
    digitalWrite(OLED_RES, HIGH); // CRITICAL: Must be HIGH for the display to operate!
    delay(50);                      // Let the charge pump stabilize

    display.begin();
    display.clearBuffer();
    display.sendBuffer();
}

// Display gear, odometer, and mode on OLED2 (SSD1315 128x64)
void oled2_display_gear(uint8_t mode, uint32_t odometer) {
    char odometer_str[12];
    char mode_str[10];
    char gear_char;
    
    snprintf(odometer_str, sizeof(odometer_str), "%03u%03u km", (uint16_t)(odometer / 1000), (uint16_t)(odometer % 1000));
    
    switch(mode) {
        case 0: strcpy(mode_str, "ECO"); break;
        case 1: strcpy(mode_str, "NORMAL"); break;
        case 2: strcpy(mode_str, "SPORT"); break;
        default: strcpy(mode_str, "UNKNOWN"); break;
    }
    
    display.firstPage();
    do {
        display.setFont(u8g2_font_ncenB24_tr);  // Large font for gear
        
        // Display GEAR section (left side)
        display.drawStr(10, 40, "GEAR");
        display.drawStr(10, 65, "P"); // TODO: Update with actual gear
        
        // Display MODE section (center)
        display.setFont(u8g2_font_8x13_m_symbols);
        display.drawStr(65, 25, "MODE");
        display.setFont(u8g2_font_6x12_m_symbols);
        display.drawStr(65, 40, mode_str);
        
        // Display ODOMETER section (bottom right)
        display.drawStr(10, 64, "ODO:");
        display.drawStr(50, 64, odometer_str);
        
    } while (display.nextPage());
}

// Update OLED2 with car state (SSD1315 128x64)
void oled2_update(dash_state_t* dash_state) {
    static uint32_t last_odo = 1000000;
    static uint8_t last_mode = 0xFF;
    static uint8_t last_state = 0xFF;
    
    if(dash_state->car_state == CAR_STATE_IDLE) {
        last_state = dash_state->car_state;
        return;
    }
    
    uint8_t have_change = 0;
    
    if(last_odo != dash_state->car_odo) {
        have_change = 1;
    }
    if(last_mode != dash_state->power_level) {
        have_change = 1;
    }
    if(last_state != dash_state->car_state) {
        have_change = 1;
    }
    
    if(have_change) {
        oled2_display_gear(dash_state->power_level, dash_state->car_odo % 1000000);
    }
    
    last_odo = dash_state->car_odo;
    last_mode = dash_state->power_level;
    last_state = dash_state->car_state;
}



void oled_display_warn(uint8_t warn_id){
    const uint8_t big_excalmation[] = {
        0x00, 0x00, 0x3c, 0x3c, 0x3c, 0x3c, 0x3c, 0x3c, 0x3c, 0x3c, 0x3c, 0x3c, 0x3c, 0x3c, 0x3c, 0x3c, 
        0x00, 0x00, 0x3c, 0x3c, 0x3c, 0x00, 0x00
    };

    display.firstPage();
    do {
        display.drawXBM(16,0, 8, 23, big_excalmation); // Ensure odo_logo is 16x16 pixels
        display.setFont(u8g2_font_6x12_m_symbols);

        switch(warn_id){
            case 1:{ //Shift
                display.drawStr(32,16,"Shift to N");
                break;
            }
            case 2:{ //Shift
                display.drawStr(32,16,"Door Opened");
                break;
            }
            case 3:{ //Shift
                display.drawStr(32,16,"Remove Gun");
                break;
            }
            case 4:{ //Shift
                display.drawStr(32,16,"Press Brake");
                break;
            }
        }
    
    }while (display.nextPage());
}

// Display gear on OLED
void oled_display_gear(uint8_t power, uint32_t odometer, uint16_t range_left) {
    // Define a simple bitmap for the flash logo (16x16 pixels)
    /*const uint8_t batt_logo[] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfc, 0x3f, 0x00, 0x04, 0x20, 0x00, 0xf4, 
        0xef, 0x00, 0xf4, 0xaf, 0x00, 0xf4, 0xaf, 0x00, 0xf4, 0xaf, 0x00, 0xf4, 0xaf, 0x00, 0xf4, 0xef, 
        0x00, 0x04, 0x20, 0x00, 0xfc, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };*/
    const uint8_t flash_logo[] = {
        0x00, 0x00, 0xf0, 0x03, 0xf0, 0x01, 0xf8, 0x00, 0x78, 0x00, 0xfc, 0x07, 0xfc, 0x03, 0xc0, 0x01, 
        0xe0, 0x00, 0x70, 0x00, 0x30, 0x00, 0x00, 0x00
    };

    
    char odometer_str[12];
    char range_left_str[10];
    
    snprintf(odometer_str, sizeof(odometer_str), "%03u%03u km", (uint16_t) (odometer / 1000), (uint16_t) (odometer % 1000)); // Ensure 6 digits with leading zeros
    snprintf(range_left_str, sizeof(range_left_str), "%4u km", (uint32_t)range_left); // Align 'km'

    display.firstPage();
    do {
        // Set font and draw gear larger
        //display.setFont(u8g2_font_inb24_mf); // Larger font for gear
        //display.setDrawColor(1); // Set draw color to white
        //display.drawStr(16, 24, gear); // Display gear at the left side

        // Display odometer
        display.setFont(u8g2_font_6x12_m_symbols); // Set font to 10px size

                display.drawStr(13,8,"MODE"); // Display "MODE" at the top left corner
        switch(power){
            case 0:{ //E
                display.drawStr(16,23,"ECO");
                break;
            }
            case 1:{ //N
                display.drawStr(16,23,"NOR");
                break;
            }
            case 2:{ //S
                display.drawStr(16,23,"SPT");
                break;
            }
        }


        display.drawStr(42, SCREEN_HEIGHT - 9, "ODO"); // Ensure odo_logo is 16x16 pixels
        display.drawStr(SCREEN_WIDTH - display.getStrWidth(odometer_str) - 9, SCREEN_HEIGHT - 9, odometer_str); // Display odometer at bottom right corner

        // Display range left with flash logo
        //display.drawXBM(46, 0, 18, 16, batt_logo); 
        display.drawXBM(46, 0, 12, 12, flash_logo); 
        display.drawStr(SCREEN_WIDTH - display.getStrWidth(range_left_str) - 9, 8, range_left_str); // Display value and "km" next to the logo

    } while (display.nextPage());

    /*display.clearBuffer();// Set text size
    display.setFont(u8g2_font_ncenB14_tr); // Set font
    // display.setTextColor(SSD1327_WHITE); // Set text color
    //display.setDrawColor(1); // Set draw color to white
    display.drawStr(64,64,"Hello World!"); // Display gear at (50, 64)
    //display.drawBox(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT); // Clear the screen
    display.sendBuffer(); // Update the display*/
}

// Update OLED with car state
void oled_update(dash_state_t* dash_state) {
    
    static char last_gear = 0;
    static uint8_t last_warn = 0;
    static uint32_t last_odo = 1000000;
    static uint16_t last_range = 0;
    static uint8_t last_state = 0;
    
    if(dash_state->car_state == CAR_STATE_IDLE){
        digitalWrite(OLED_OE, HIGH);
        last_state = dash_state->car_state;
        return;
    }

    digitalWrite(OLED_OE, LOW);

    if(dash_state->have_warn){
        uint8_t warn_id = 0;

        if(dash_state->shift_n)
            warn_id = 1;
        if(dash_state->door_open)
            warn_id = 2;
        if(dash_state->charge_gun)
            warn_id = 3;
        if(dash_state->press_brake)
            warn_id = 4;

        if(warn_id != last_warn)
            oled_display_warn(warn_id);

        last_warn = warn_id;
        return;
    }


    
    char gear; // Initialize gear character
    uint32_t odometer_display;
    uint8_t range_left_display;
    //dash_state->car_odo /= 10; //convert to 0.1km
    odometer_display = dash_state->car_odo % 1000000; // Ensure odometer supports up to 999999 km
    switch (dash_state->power_level) {
        case 0: gear = 'E'; break;
        case 1: gear = 'N'; break;
        case 2: gear = 'S'; break;
        default: gear = ' '; break;
    }

    char gear_str[2] = {gear, '\0'}; // Convert char to null-terminated string

    uint8_t have_change = 0;

    if(last_gear != gear){
        have_change = 1;
    }
    if(last_odo != odometer_display){
        have_change = 1;
    }
    if(last_range != dash_state->range_left){
        have_change = 1;
    }
    if(last_state == CAR_STATE_IDLE){
        have_change = 1;


        display.clearBuffer();
        display.clearDisplay();
    }
    if(last_warn){
        have_change = 1;
    }

   
    //*
    if((dash_state->led_matrix_raw[1] >> 2) & 0x01){
         display.setContrast(0);
    }
    else{
        display.setContrast(255);
    }//*/

    if(have_change){
        oled_display_gear(dash_state->power_level, odometer_display, dash_state->range_left);
    }

    last_gear = gear;
    last_odo = odometer_display;
    last_range = dash_state->range_left;
    last_state = dash_state->car_state;
    last_warn = 0;
}
