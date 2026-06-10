// CAN message definitions
#ifndef CAN_MSG_H
#define CAN_MSG_H

typedef struct{
    uint8_t car_state:2;
    uint8_t car_gear:2;
    uint8_t padding1: 4;

    uint8_t car_speed;
    uint16_t motor_rpm;
    uint32_t car_odo:24;
    
    uint8_t rollingCounter:4;
    uint8_t padding2:4;

} vcu_140_t;

typedef struct{
    uint32_t odo_fb;
} dash_141_t;

typedef struct{
    union {
        struct {
            uint8_t car_soc;
            uint8_t motor_temp;
            uint8_t battery_temp;

            uint8_t charge:1;
            uint8_t padding:5;
            uint8_t padding7:1;
            //uint8_t beam_l:1; //also is park light
            uint8_t padding2:1;

            uint8_t padding3:5;
            uint8_t check_engine:1;
            uint8_t padding4:1; //12v battery error
            uint8_t ready:1;

            /*uint8_t turn_l:1;
            uint8_t turn_r:1;
            uint8_t overheat_warning:1;
            uint8_t padding5:1;
            uint8_t beam_h:1;
            uint8_t hazard:1;
            uint8_t brake_err:1; // brake fluid error
            uint8_t low_fuel:1; //low battery warning*/
            uint8_t padding8;

            /*uint8_t seatbelt:1;
            uint8_t rear_diff_lock:1;
            uint8_t fog_r:1;
            uint8_t r_window_defrost:1;
            uint8_t parking_brake:1;
            uint8_t traction_ctrl:1;
            uint8_t padding6:2;*/
            uint8_t padding9;
        };

        uint8_t bytes[8];
    };
} vcu_142_t;

typedef struct{
    uint16_t fl_rpm;
    uint16_t fr_rpm;
    uint16_t rl_rpm;
    uint16_t rr_rpm;
} dcu_145_t;

typedef struct{
    uint16_t car_udc;
    uint16_t car_power_percent;
    uint16_t est_range;
    
    uint8_t pwr_mode:2;
    uint8_t padding:6;

    uint8_t counter:4;
    uint8_t padding1:4;

} vcu_146_t;

typedef struct{
    uint8_t shift_n:1;
    uint8_t door_open:1;
    uint8_t charge_gun:1;
    uint8_t press_brake:1;

} vcu_14a_t;


typedef struct{
    union {
        struct {
            uint8_t type;           // always 0x12
            uint8_t device_id;      // 0x01 or 0x02
            uint8_t ch1:1;
            uint8_t ch2:1;
            uint8_t ch3:1;
            uint8_t ch4:1;
            uint8_t ch5:1;
            uint8_t ch6:1;
            uint8_t ch7:1;
            uint8_t ch8:1;
            uint8_t padding[5];
        };
        uint8_t bytes[8];
    };
} sr_126_t;


#endif