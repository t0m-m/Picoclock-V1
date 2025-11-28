#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>

extern const bool digit_segments[10][8];

void max7221_init(); // Initialization of the MAX7221
void max7221_send(uint8_t addr, uint8_t data); // Function for sending data to max7221


#endif 