#define F_CPU 8000000UL

#include <avr/io.h>
#include <util/delay.h>

#include "../lib/RGBLEDStrip/RGBLEDStrip.h"

#define LED PB0
#define WIDTH 16
#define HEIGHT 16

void display(const struct rgb (&matrix)[HEIGHT][WIDTH]) {
    for (short rowIdx = 0; rowIdx < HEIGHT; rowIdx++){
        if (rowIdx % 2 == 0) {
            for (short cellIdx = 0; cellIdx < WIDTH; cellIdx++) {
                setPixel(matrix[rowIdx][cellIdx]);
            }
        } else {
            for (short cellIdx = WIDTH - 1; cellIdx >= 0; cellIdx--) {
                setPixel(matrix[rowIdx][cellIdx]);
            }
        }
    }

    DDRB |= _BV(LED);
    send(LED);
}

int main() {
    struct rgb m[HEIGHT][WIDTH];
    for (byte i = 0; i < HEIGHT; i++) {
        for (byte j = 0; j < WIDTH; j++) {
            if (i == j) {
                m[i][j] = {15, 0, 0};
            } else {
                m[i][j] = {0, 0, 0};
            }
        }
    }

    display(m);

    DDRD |= _BV(7);
    bool r = true;
    while (true) {
        if (r) {
            PORTD |= _BV(7);
        } else {
            PORTD &= ~_BV(7);
        }

        r = !r;

        _delay_ms(2000);
    }
}