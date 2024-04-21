#include <math.h>

#include "color.h"

byte prepare(double part) {
    return round(part * 255);
}

rgb hsv2rgb (hsv in) {
    rgb out;

    // Achromatic (gray)
    if (in.s == 0) {
        out.r = out.g = out.b = in.v / 255.0f;

        return out;
    }

    // Conversion values
    float tempH = in.h / 60.0f;
    short i = (int)floor(tempH);
    float f = tempH - i;
    double p = prepare(in.v * (1 - in.s));
    double q = prepare(in.v * (1 - in.s * f));
    double t = prepare(in.v * (1 - in.s * (1 - f)));

    // There are 6 cases, one for every 60 degrees
    switch (i) {
        case 0:
            out.r = prepare(in.v);
            out.g = t;
            out.b = p;

            break;

        case 1:
            out.r = q;
            out.g = prepare(in.v);
            out.b = p;

            break;

        case 2:
            out.r = p;
            out.g = prepare(in.v);
            out.b = t;

            break;

        case 3:
            out.r = p;
            out.g = q;
            out.b = prepare(in.v);

            break;

        case 4:
            out.r = t;
            out.g = p;
            out.b = prepare(in.v);

            break;

        case 5:
        default:
            out.r = prepare(in.v);
            out.g = p;
            out.b = q;

            break;
    }

    return out;
}