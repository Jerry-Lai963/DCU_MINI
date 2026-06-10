# CANbus LED Matrix Control and Test Plan

## Overview

This document describes how the dashboard light signals are mapped from CAN bus messages into the 74HC595 shift-register bytes used by `led_matrix_send()`.

The fix in `led_matrix.cpp` corrects the 74HC595 channel mapping so that:
- channel 1 maps to bit 7
- channel 2 maps to bit 6
- channel 3 maps to bit 5
- channel 4 maps to bit 4

## Key CAN IDs and data sources

### `0x142` — VCU dashboard status
This is the primary message used by `led_matrix_can_update()` to update warning lamps, gauges, and ready/check-engine indicators.

The message is decoded into `vcu_142_t`.

### `0x126` — SRPDU compatibility input
This message is parsed by `DCU_Mini.ino` and stored in `main_dash_state.srpdu_raw`.

- If `data[0]<<8 | data[1] == 0x1201`, the value is stored in `srpdu_raw[0]`.
- If `data[0]<<8 | data[1] == 0x1202`, the value is stored in `srpdu_raw[1]`.

### `0x131`, `0x133`, `0x135` — new SRPDU compatibility
These are alternate messages that write directly into `srpdu_raw[0]`, `srpdu_raw[1]`, and `srpdu_raw[2]`.

## `vcu_142_t` byte layout

The structure is defined in `can_msg.h`.

- `bytes[0]` = `car_soc`
- `bytes[1]` = `motor_temp`
- `bytes[2]` = `battery_temp`
- `bytes[3]` contains:
  - bit 0: `charge`
  - bit 6: `beam_l` (also park light)
- `bytes[4]` contains:
  - bit 5: `check_engine`
  - bit 7: `ready`
- `bytes[5]` contains:
  - bit 0: `turn_l`
  - bit 1: `turn_r`
  - bit 4: `beam_h`
  - bit 5: `hazard`
  - bit 6: `brake_err`
- `bytes[6]` contains:
  - bit 0: `seatbelt`
  - bit 1: `rear_diff_lock`
  - bit 2: `fog_r`
  - bit 3: `r_window_defrost`
  - bit 4: `parking_brake`
  - bit 5: `traction_ctrl`

## Output byte mapping for the 74HC595 chain

`led_matrix_send()` shifts out 4 bytes from `dash_state->led_matrix_raw[]`:

- `led_matrix_raw[0]` → 595 #1
- `led_matrix_raw[1]` → 595 #2
- `led_matrix_raw[2]` → 595 #3
- `led_matrix_raw[3]` → 595 #4

### Gauge bytes

- `led_matrix_raw[0]` = SOC gauge bar (lower-N LEDs from byte 0 of `0x142`)
- `led_matrix_raw[1]` = water temperature gauge bar (lower-N LEDs from byte 1 of `0x142`)

### Manual 74HC595 output mapping

#### 595 #3 (`led_matrix_raw[2]`)
- `CH4` → READY
- `CH5` → low beam
- `CH3` → high beam
- `CH6` → rear fog
- `CH2` → left indicator
- `CH6` → right indicator

#### 595 #4 (`led_matrix_raw[3]`)
- `CH1` → check engine / brake warning output
- `CH2` → hazard
- `CH3` → logo light
- `CH4` → brake fluid warning

## SRPDU input bits and mapped outputs

### SR1 (`srpdu_raw[0]`)
- `0x10` = low beam
- `0x08` = high beam
- `0x20` = rear fog
- `0x80` = brake fluid

### SR2 (`srpdu_raw[1]`)
- `0x04` = left indicator
- `0x02` = right indicator
- `0x06` = hazard

## Light control test cases

### 1) Turn on Check Engine

- CAN ID: `0x142`
- Set `bytes[4]` bit 5 = `1`
- Example byte content:
  - `0x00 0x00 0x00 0x00 0x20 ...`

### 2) Turn on READY

- CAN ID: `0x142`
- Set `bytes[4]` bit 7 = `1`
- Example byte content:
  - `0x00 0x00 0x00 0x00 0x80 ...`

### 3) Turn on Low Beam

- CAN ID: `0x126`
- `data[0]` and `data[1]` set to `0x12 0x01`
- `data[2]` = `0x10`

### 4) Turn on High Beam

- CAN ID: `0x126`
- `data[0]` and `data[1]` set to `0x12 0x01`
- `data[2]` = `0x08`

### 5) Turn on Rear Fog

- CAN ID: `0x126`
- `data[0]` and `data[1]` set to `0x12 0x01`
- `data[2]` = `0x20`

### 6) Turn on Brake Fluid Warning

- CAN ID: `0x126`
- `data[0]` and `data[1]` set to `0x12 0x01`
- `data[2]` = `0x80`

### 7) Turn on Left Indicator

- CAN ID: `0x126`
- `data[0]` and `data[1]` set to `0x12 0x02`
- `data[2]` = `0x04`

### 8) Turn on Right Indicator

- CAN ID: `0x126`
- `data[0]` and `data[1]` set to `0x12 0x02`
- `data[2]` = `0x02`

### 9) Turn on Hazard

- CAN ID: `0x126`
- `data[0]` and `data[1]` set to `0x12 0x02`
- `data[2]` = `0x06`

## Test pattern helper

A helper function `led_matrix_test_pattern()` was added to `led_matrix.cpp`.

It lights a representative set of indicators:
- READY
- low beam
- high beam
- rear fog
- check engine/brake warning outputs
- hazard

Call `led_matrix_test_pattern(&main_dash_state);` from `DCU_Mini.ino` if you want to verify the wiring and shift-register outputs with a single test sequence.

## Notes

- The fix in `led_matrix.cpp` ensures the channel-to-bit mapping uses MSB-first ordering.
- If a light still does not light after sending the correct CAN message, verify the physical wiring on 595 #4 channel 1 or channel 2.
