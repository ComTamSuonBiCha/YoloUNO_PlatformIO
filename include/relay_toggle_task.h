#pragma once
#include <Arduino.h>

#ifndef __RELAY_TOGGLE_TASK_H__
#define __RELAY_TOGGLE_TASK_H__

#define KEY1_GPIO 0
#define KEY2_GPIO 1
#define KEY3_GPIO 8
#define KEY4_GPIO 9

#define RELAY1_GPIO 4
#define RELAY2_GPIO 5
#define RELAY3_GPIO 6
#define RELAY4_GPIO 7

// void relay_toggle_task(void *pvParameters);

#endif

#ifndef ACTIVE_LOW
#define ACTIVE_LOW 1
#endif

void relay_toggle_task(void *pvParameters);

// helpers you might call from elsewhere
void relays_write_all(bool r1, bool r2, bool r3, bool r4);
void relays_toggle_by_index(uint8_t idx);  // 0..3