#include <EEPROM.h>

#include <stdint.h>
#include "odo.h"

uint32_t odo_eeprom_read(){
    uint32_t odo = 0;
    for(int i = 0; i < 4; i++){
        odo |= ((uint32_t)EEPROM.read(0x100 + i)) << (i*8);
    }

    if(odo == 0xFFFFFFFF){
        odo = 0;
    }
    return odo;
}

void odo_eeprom_write(uint32_t odo){
    static uint32_t tmp_odo = 0;

    if(odo == tmp_odo){
        return;
    }

    tmp_odo = odo;
    for(int i = 0; i < 4; i++){
        EEPROM.write(0x100 + i, (odo >> (i*8)) & 0xFF);
    }
}