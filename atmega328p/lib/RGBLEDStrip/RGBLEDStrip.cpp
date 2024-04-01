#include <avr/io.h>
#include <util/delay.h>

#include "RGBLEDStrip.h"

// @see https://agelectronica.lat/pdfs/textos/L/LDWS2812.PDF
// HIGH 0.8mks +/- 150ns and 0.45mks +/- 150ns
// LOW 0.4mks +/- 150ns and 0.85mks +/- 150ns
// 1s/8000000 = 125ns for 1 tact

byte buffer[768];
short pointer = 0;

void setBitHigh(const byte pin) {
    PORTB |= _BV(pin); // 1 tactDuration = 125ns
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop"); // 0.75mks

    PORTB &= ~_BV(pin); // 1 tactDuration
    asm("nop");
    asm("nop"); // 0.375mks
}

void setBitLow(const byte pin) {

    PORTB |= _BV(pin); // 1 tactDuration
    asm("nop");
    asm("nop"); // 0.375mks

    PORTB &= ~_BV(pin); // 1 tactDuration
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop"); // 0.875mks
}

void bufferByte(const byte intensity) {
    buffer[pointer] = intensity;
    pointer++;
}

void setPixel(const struct rgb &color) {
    bufferByte(color.g);
    bufferByte(color.r);
    bufferByte(color.b);
}

void send(const byte pin) {
    for (const byte &color : buffer) {
        if (bit_is_set(color, 7)) {
            setBitHigh(pin);
        } else {
            setBitLow(pin);
        }
        if (bit_is_set(color, 6)) {
            setBitHigh(pin);
        } else {
            setBitLow(pin);
        }
        if (bit_is_set(color, 5)) {
            setBitHigh(pin);
        } else {
            setBitLow(pin);
        }
        if (bit_is_set(color, 4)) {
            setBitHigh(pin);
        } else {
            setBitLow(pin);
        }
        if (bit_is_set(color, 3)) {
            setBitHigh(pin);
        } else {
            setBitLow(pin);
        }
        if (bit_is_set(color, 2)) {
            setBitHigh(pin);
        } else {
            setBitLow(pin);
        }
        if (bit_is_set(color, 1)) {
            setBitHigh(pin);
        } else {
            setBitLow(pin);
        }
        if (bit_is_set(color, 0)) {
            setBitHigh(pin);
        } else {
            setBitLow(pin);
        }
    }

    _delay_us(50);

    pointer = 0;
}