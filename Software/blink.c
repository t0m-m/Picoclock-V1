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

// PINY I2C
#define PIN_SDA 14
#define PIN_SCL 15

// PINY PRZYCISKOW
#define BUTTON_PIN 6
#define BUTTON_PIN 7
#define BUTTON_PIN 8
#define BUTTON_PIN 9

// MAX 7221 
void max7221_send(uint8_t addr, uint8_t data) {   // MAX oczekuje danych, 8 bitow adresu i 8 bitow danych
    gpio_put(PIN_CS, 0);                          // aktywujemy CS, zeby MAX wiedzial ze do niego wysylamy, CS = 0 <- wysylamy, CS = 1 <- MAX przestaje odbierac i wykonuje komende
    uint8_t buf[2] = { addr, data };              // 16 bitow, adres + data
    spi_write_blocking(spi1, buf, 2);             // po sp1 wyslij z tablicy buf 2 wartosci pod kolejnymi jej adresami. blocking oznacza ze czeka az wszystkie dane zostana wyslane
    gpio_put(PIN_CS, 1);                          // ustawiamy CS na wartosc 1, zeby MAX wykonal polecenia ktore mu dalismy
}

void max7221_init() {
    max7221_send(0x09, 0x00);  // decode off
    max7221_send(0x0A, 0x05);  // intensity
    max7221_send(0x0B, 0x05);  // scan limit = 6
    max7221_send(0x0C, 0x01);  // normal operation
    max7221_send(0x0F, 0x00);  // display test off
}

// I2C ADDRESS RESERVATION
bool reserved_addr(uint8_t addr) {                        // Funkcja sprawdza czy specjalne adresy i2c nie sa zarezerwowane 
    return (addr & 0x78) == 0 || (addr & 0x78) == 0x78;   // Specjalne adresy sa od 000 0xxx do 111 1xxx
}

// DS 3231
void ds3231_set_time(uint8_t *buf); {
    uint8_t start = 0x00; 
    i2c_write_blocking(i2c1, 0x68, &start, 1, true);
    i2c_write_blocking(i2c1, 0x68, buf, 7, false);
}

void ds3231_read_time(uint8_t *buf) {
    uint8_t start = 0x00;                                // 0x68 to adres i2c urzadzenia DS3231
    i2c_write_blocking(i2c1, 0x68, &start, 1, true);     // Funkcja ustawia bajt od ktorego chce czytac na 1
    i2c_read_blocking(i2c1, 0x68, buf, 7, false);        // Funkcja zczytuje wartości od 0x00 do 0x06, czyli sekundy, minuty, godziny, dzien tygodnia, dzien miesiaca, miesiac, rok 
}

void ds3231_init() {

}

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

// WYSWIETLANIE LICZB Z ZAMIENIONYMI DIG I SEG
void display_digits_swapped(const int *digits, int count) {
    uint8_t segment_masks[8] = {0};

    for (int pos = 0; pos < count; pos++) {
        int d = digits[pos];
        for (int seg = 0; seg < 8; seg++) {
            if (digit_segments[d][seg])
                segment_masks[seg] |= (1u << pos);
        }
    }

    for (int seg = 0; seg < 8; seg++)
        max7221_send(seg + 1, segment_masks[seg]);
}

// ZMIENNE CZASU
int hours = 1;
int minutes = 16;
int seconds = 0;

// ZMIENNA JASNOSCI
uint8_t brightness = 0;

// TIMER 1S 
int64_t tick(alarm_id_t id, void *user_data) {
    seconds++;
    if (seconds >= 60) { seconds = 0; minutes++; }
    if (minutes >= 60) { minutes = 0; hours++; }
    if (hours > 12) { hours = 01; }

    // konwersja HHMMSS na tablicę cyfr
    int digits[6] = {
        hours / 10,
        hours % 10,
        minutes / 10,
        minutes % 10,
        seconds / 10,
        seconds % 10
    };

    display_digits_swapped(digits, 6);

    return 1000000; // wywołuj co 1 sekundę
}

// PRZYCISK
void init_button() {
    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_PIN);
}

// SPRAWDZ CZY PRZYCISK WCISNIEY + DEBOUNCER
bool button_pressed() {
    static uint32_t last_time = 0;

    if (!gpio_get(BUTTON_PIN)) {
        uint32_t now = to_ms_since_boot(get_absolute_time());
        if (now - last_time > 250) {
            last_time = now;
            return true;
        }
    }
    return false;
}

// KONTROLA JASNOCI
void update_brightness() {
    brightness++;
    if (brightness > 15) brightness = 0;
    max7221_send(0x0A, brightness);
}

int main() {                              // DS 3231 oczekuje od nas danych w formacie BCD, czyli np. 23 w dziesietnym = 0x23 w BCD
    uint8_t time_set[7] = {               // tablica w ktorej ustawiamy pierwotny czas 
        {0x00},                           // sekundy
        {0x00},                           // minuty
        {0x12},                           // godziny
        {0x01},                           // dzien tygodnia od 1 do 7, na razie ustawiony poniedzialek
        {0x25},                           // dzien miesiaca
        {0x11},                           // miesiac, na razie ustawiony listopad
        {0x25},                           // rok 
    };

    uint8_t time_buf[7];                  // TABLICA Z CZASEM I DATA Z DS3231
    uint8_t kropki = (1 << 1) | (1 << 3); // Wyswietla kropki HH. MM. SS
    stdio_init_all();
    
    // SPI1 init
    spi_init(spi1, 1000000);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SCK,  GPIO_FUNC_SPI);

    gpio_init(PIN_CS);
    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_put(PIN_CS, 1);

    // I2C init
    i2c_init(i2c1, 100 * 1000);                  // Uzywamy i2c1 i baudrate ustawiony na 100kHz
    gpio_set_function(PIN_SDA, GPIO_FUNC_I2C);   // Przypisanie funkcji odpowiednim pinom
    gpio_set_function(PIN_SCL, GPIO_FUNC_I2C);

    gpio_pull_up(PIN_SDA);                       // Aktywowane pull upy na liniach SDA i SCL 
    gpio_pull_up(PIN_SCL);

    // DS 3231 write time
    ds3231_set_time(time_set);

    // DS 3231 read time
    ds3231_read_time(time_buf);

    
    // MAX 7221 init
    max7221_init();

    // Button init
    init_button();

    // Timer 1 sekunda
    add_alarm_in_ms(1000, tick, NULL, true);

    while (1) {
        tight_loop_contents();
        max7221_send(2,kropki);
        if (button_pressed()) {
        update_brightness();
        }
    }
}
