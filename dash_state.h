#ifndef DASH_STATE_H
#define DASH_STATE_H

#ifndef _AVR_IOXXX_H_
    #include <avr/iom324p.h>
#endif

enum car_state_gear{
	CAR_GEAR_P,
	CAR_GEAR_R,
	CAR_GEAR_N,
	CAR_GEAR_D,
	CAR_GEAR_ERR
};

typedef struct {
    uint8_t car_state;
    uint8_t car_gear;

    uint8_t charge_state;

    int16_t udc;
    int32_t idc;
    
    int16_t power_percent;

    uint8_t car_speed;
    uint16_t motor_rpm;
    uint32_t car_odo;
    uint16_t range_left;

    int16_t car_idc;

    uint8_t car_soc;
    uint8_t motor_temp;
    uint8_t battery_temp;

    uint8_t led_matrix[4];
    uint8_t led_matrix_raw[10];

    uint8_t srpdu_raw[4];

    uint32_t wheelspeed[4];

    uint16_t range;

    //errors

    uint8_t have_warn;

    uint8_t shift_n;
    uint8_t door_open;
    uint8_t charge_gun;
    uint8_t press_brake;

    uint8_t srpdu_drive;

    int16_t curr_mlx[4];

    uint32_t state_timeout;
    uint32_t led_timeout;
    uint8_t powerdown_req;

    uint8_t power_level;

    // Door warning edge detection state
    uint8_t door_open_prev;
    uint32_t door_buzzer_timer;
    uint8_t door_buzzer_active;

} dash_state_t;

enum {
    CAR_STATE_IDLE,
    CAR_STATE_IGN,
    CAR_STATE_ACC,
    CAR_STATE_RUN
};

void dash_state_update(dash_state_t* dash_state);

#endif