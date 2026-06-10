#include <Arduino.h>
#include <SPI.h>
#include "pins.h"
#include "rmc_calc.h"

int32_t last_gauge_val = 0;
extern uint16_t max_val;
extern int8_t gauge_speed;
extern uint8_t powerdown;


void write_cs4192(uint16_t target_val){
	SPCR &= ~(1 << SPE); //disable SPI

	SPCR = 0x03 | (SPCR & 0xFC); // f/64
	SPSR |= 0x01;

	SPCR |= (1 << SPE); //enable SPI
	
	digitalWrite(GAUGE_CS, 1);
	
	SPI.transfer(target_val >> 8 & 0x03);
	SPI.transfer(target_val & 0xff);
	
	digitalWrite(GAUGE_CS, 0);
	
	SPCR &= ~(1 << SPE); //disable SPI
	SPCR = 0x00 | (SPCR & 0xFC); // f/2
	SPCR |= (1 << SPE); //enable SPI
	
}

uint16_t update_gauge(uint16_t target_val){
	
	gauge_speed = clip_r(abs(target_val - last_gauge_val)/10, 1, 40);
	if(powerdown){
		target_val = 0;
		gauge_speed = 40;
	}
	//if(powerdown && !last_gauge_val){
			//PORTB &= ~(1 << PINB0);
			//PORTB &= ~(1 << PINB2);
	//}
	//else{

	//}
	digitalWrite(GAUGE_OE, 1);
	
	write_cs4192(1024-clip_r(target_val, last_gauge_val - gauge_speed, last_gauge_val + gauge_speed) % 1024);
	
	last_gauge_val = clip_r(target_val, last_gauge_val - gauge_speed, last_gauge_val + gauge_speed);
	
	return 0;
	
	
}