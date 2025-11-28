#include <stdint.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/timer.h"
#include "hardware/i2c.h"
#include "pico/binary_info.h"

// PINY SPI
#define PIN_CS   13
#define PIN_SCK  10
#define PIN_MOSI 11

// MAPA SEGMENTOW
// Segmenty: C DP A F G E D B   (nie wiem co tu sie odjebalo ale tak jest)
const bool digit_segments[10][8] = {
    {1,0,1,1,0,1,1,1}, // 0
    {1,0,0,0,0,0,0,1}, // 1
    {0,0,1,0,1,1,1,1}, // 2
    {1,0,1,0,1,0,1,1}, // 3
    {1,0,0,1,1,0,0,1}, // 4
    {1,0,1,1,1,0,1,0}, // 5
    {1,0,1,1,1,1,1,0}, // 6
    {1,0,1,0,0,0,0,1}, // 7
    {1,0,1,1,1,1,1,1}, // 8
    {1,0,1,1,1,0,1,1}, // 9
};

void max7221_send(uint8_t addr, uint8_t data) {   // MAX oczekuje danych, 8 bitow adresu i 8 bitow danych
    gpio_put(PIN_CS, 0);                          // aktywujemy CS, zeby MAX wiedzial ze do niego wysylamy, CS = 0 <- wysylamy, CS = 1 <- MAX przestaje odbierac i wykonuje komende
    uint8_t buf[2] = { addr, data };              // 16 bitow, adres + data
    spi_write_blocking(spi1, buf, 2);             // po sp1 wyslij z tablicy buf 2 wartosci pod kolejnymi jej adresami. blocking oznacza ze czeka az wszystkie dane zostana wyslane
    gpio_put(PIN_CS, 1);                          // ustawiamy CS na wartosc 1, zeby MAX wykonal polecenia ktore mu dalismy
}

void max7221_init() {
    max7221_send(0x09, 0x00);  // decode off
    max7221_send(0x0A, 0x05);  // intensity
    max7221_send(0x0B, 0x07);  // scan limit = 6
    max7221_send(0x0C, 0x01);  // normal operation
    max7221_send(0x0F, 0x00);  // display test off
}