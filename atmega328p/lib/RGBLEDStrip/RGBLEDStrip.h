#ifndef RGBLEDSTRIP_H
#define RGBLEDSTRIP_H

#define byte unsigned char

struct rgb {
    byte r;
    byte g;
    byte b;
};

void setPixel(const struct rgb &color);

void send(const byte pin);

#endif
