#pragma once

#include <stdbool.h>
#include <stdint.h>

void input_update(void);
bool input_query(uint8_t);
uint8_t input_get_key(void);