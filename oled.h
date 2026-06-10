#ifndef OLED_H
#define OLED_H

// Function to initialize the OLED display
void oled_init();

// Function to update the OLED display
void oled_update(dash_state_t* dash_state);
void oled_display_gear(char gear);

// SSD1315 OLED2 (128x64) function declarations
// void oled2_init();
// void oled2_update(dash_state_t* dash_state);
// void oled2_display_gear(uint8_t mode, uint32_t odometer);

#endif // OLED_H