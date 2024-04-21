#ifndef COLOR_H
#define COLOR_H

typedef unsigned char byte;

typedef struct {
    unsigned char r;
    unsigned char g;
    unsigned char b;
} rgb;
typedef struct {
    double h;       // angle in degrees
    double s;       // a fraction between 0 and 1
    double v;       // a fraction between 0 and 1
} hsv;

rgb hsv2rgb(hsv in);
#endif
