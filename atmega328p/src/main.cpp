#define F_CPU 8000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>

#include "../lib/color/color.h"
#include "../lib/RGBLEDStrip/RGBLEDStrip.h"

#define LED PB0
#define WIDTH 16
#define HEIGHT 16
#define SCALE_MAX 20

#define BUTTON_UP PB1
#define BUTTON_DOWN PB3

byte scale = 1;

rgb matrix[HEIGHT][WIDTH];

byte minIntensity = 0;
byte maxIntensity;

byte rowIdx;
short columnIdx;
byte nextIntensity;
byte height;
byte ejectionHeight;

byte intensitiesMaxHeightsMap[] = {3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9};
byte currentIntensities[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
byte currentHeights[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

void setMaxIntensity() {
    maxIntensity = ceil((13. / SCALE_MAX) * scale) - 1;
}

void incrementScale() {
    if (scale == SCALE_MAX) {
        return;
    }

    scale += 1;
    setMaxIntensity();
}

void decrementScale() {
    if (scale == 1) {
        return;
    }

    scale -= 1;
    setMaxIntensity();
}


void display(const rgb (&matrix)[HEIGHT][WIDTH]) {
    for (rowIdx = 0; rowIdx < HEIGHT; rowIdx++){
        if (rowIdx % 2 == 0) {
            for (columnIdx = 0; columnIdx < WIDTH; columnIdx++) {
                setPixel(matrix[rowIdx][columnIdx]);
            }
        } else {
            for (columnIdx = WIDTH - 1; columnIdx >= 0; columnIdx--) {
                setPixel(matrix[rowIdx][columnIdx]);
            }
        }
    }

    DDRB |= _BV(LED);
    send(LED);
}

void shoutDownFire() {
    for (rowIdx = 0; rowIdx < HEIGHT; rowIdx++) {
        for (columnIdx = 0; columnIdx < WIDTH; columnIdx++) {
            matrix[rowIdx][columnIdx] = {0, 0, 0};
        }
    }

    for (columnIdx = 0; columnIdx < WIDTH; columnIdx++) {
        currentHeights[columnIdx] = 0;
    }

    display(matrix);
}

short min(short a, short b) {
    return a < b ? a : b;
}
short max(short a, short b) {
    return a > b ? a : b;
}

byte intensityShiftRand;
byte getIntensityShift() {
    intensityShiftRand = rand() % 8;

    if (intensityShiftRand <= 4) {
        return 0;
    } else if (intensityShiftRand <= 6) {
        return 1;
    } else {
        return 2;
    }
}

byte heightShiftRand;
byte getHeightShift(byte intensity) {
    heightShiftRand = rand() % 16;

    if (heightShiftRand > 12 - min(intensity, 5)) { // 3 vs 8
        return 0;
    } else if (heightShiftRand > 7 - min(intensity, 5)) { // 5 vs 5
        return 1;
    } else { // 8 vs 3
        return 2;
    }
}

byte getMinRoundDistance(byte idx0, byte idx1) {
    return min(abs(idx0 - idx1) - 1, WIDTH - 2 - abs(idx0 - idx1) - 1);
}

void changeHeight(byte columnIdx, byte height) {
    if (currentHeights[columnIdx] > height && currentHeights[columnIdx] > 1) {
        currentHeights[columnIdx]--;
    } else if (currentHeights[columnIdx] < height) {
        currentHeights[columnIdx]++;
    }
}

byte getRoundIdx(byte idx) {
    if (idx < 0) {
        return WIDTH + idx;
    }
    if (idx > WIDTH - 1) {
        return idx % WIDTH;
    }
    return idx;
}

byte getTotalMaxHeight(byte columnIdx) {
    return round(intensitiesMaxHeightsMap[currentIntensities[columnIdx]] * (0.7 + scale * (1. / SCALE_MAX)));
}

struct ejection {
    short idx;
    byte before;
    byte after;
    byte maxHeight;
};

struct ejection ejection0 {
    .idx =  -1,
    .before =  0,
    .after =  0,
    .maxHeight = 0,
};
struct ejection ejection1 {
    .idx =  -1,
    .before =  0,
    .after =  0,
    .maxHeight = 0,
};

void eject(struct ejection &ejct) {
    if (ejct.before > 1) {
        changeHeight(getRoundIdx(ejct.idx - 2), ejct.maxHeight -  2 - min(getHeightShift(currentIntensities[getRoundIdx(ejct.idx - 2)]) - 1, 0));
    }
    if (ejct.before > 0) {
        changeHeight(getRoundIdx(ejct.idx - 1), ejct.maxHeight - 1 - min(getHeightShift(currentIntensities[getRoundIdx(ejct.idx - 1)]) - 1, 0));
    }

    changeHeight(getRoundIdx(ejct.idx), ejct.maxHeight - getHeightShift(currentIntensities[getRoundIdx(ejct.before)]));

    if (ejct.after > 0) {
        changeHeight(getRoundIdx(ejct.idx + 1), ejct.maxHeight - 1 - min(getHeightShift(currentIntensities[getRoundIdx(ejct.idx + 1)]) - 1, 0));

        if (ejct.after > 1) {
            changeHeight(getRoundIdx(ejct.idx + 2), ejct.maxHeight - 2 - min(getHeightShift(currentIntensities[getRoundIdx(ejct.idx + 2)]) - 1, 0));
        }
    }
}

void createEjection(byte idx, byte height, struct ejection &ejct) {
    ejct.idx = idx;
    ejct.before = 0;
    ejct.after = 0;
    ejct.maxHeight = height;

    if (rand() % 2 == 0) {
        ejct.before++;
    }
    if (rand() % (1 + ejct.before) == 0) {
        ejct.after++;
    }

    if (rand() % (3 * (ejct.before + ejct.after)) == 0) {
        ejct.before++;
    }
    if (rand() % (3 * (ejct.before + ejct.after)) == 0) {
        ejct.after++;
    }
}

bool isEjection(byte columnIdx) {
    return (
        ejection0.idx > -1 && (
            columnIdx == ejection0.idx || (
                (ejection0.before == 2 && (columnIdx == getRoundIdx(ejection0.idx - 2) || columnIdx == getRoundIdx(ejection0.idx - 1))) ||
                (ejection0.before == 1 && columnIdx == getRoundIdx(ejection0.idx - 1)) ||
                (ejection0.after == 2 && (columnIdx == getRoundIdx(ejection0.idx + 2) || columnIdx == getRoundIdx(ejection0.idx + 1))) ||
                (ejection0.after == 1 && columnIdx == getRoundIdx(ejection0.idx + 1))
            )
        )
    ) ||
    (
        ejection1.idx > -1 && (
            columnIdx == ejection1.idx || (
                (ejection1.before == 2 && (columnIdx == getRoundIdx(ejection1.idx - 2) || columnIdx == getRoundIdx(ejection1.idx - 1))) ||
                (ejection1.before == 1 && columnIdx == getRoundIdx(ejection1.idx - 1)) ||
                (ejection1.after == 2 && (columnIdx == getRoundIdx(ejection1.idx + 2) || columnIdx == getRoundIdx(ejection1.idx + 1))) ||
                (ejection1.after == 1 && columnIdx == getRoundIdx(ejection1.idx + 1))
            )
        )
    );
}

double getColorPosition(double min, double max, double multiplier) {
    return min + (max - min) * multiplier;
}
rgb getColor(byte rowIdx, byte columnIdx, double shiftFrom = 0.5, double shiftTo = 0.65) {
    hsv minColor = {
        .h = 5 + (rand() % 4) * 0.03,
        .s = 0.99,
        .v = 0.1 + (rand() % 4) * 0.03
    };
    hsv maxColor = {
        .h = 50,
        .s = 0.95,
        .v = 0.4
    };

    double multiplier = (rowIdx + 1.) / getTotalMaxHeight(columnIdx);

    multiplier *= 0.15 + scale * (1. / SCALE_MAX) * 0.85;

    if (multiplier <= shiftTo) {
        multiplier *= shiftFrom / shiftTo;
    } else {
        multiplier /= shiftTo / shiftFrom;
    }

    return hsv2rgb({
        .h = getColorPosition(minColor.h, maxColor.h, multiplier),
        .s = getColorPosition(minColor.s, maxColor.s, multiplier),
        .v = getColorPosition(minColor.v, maxColor.v, multiplier)
    });
}

void fire() {
    if (ejection0.idx > -1 && currentHeights[ejection0.idx] == ejection0.maxHeight) {
        ejection0.idx = -1;
    }
    if (ejection1.idx > -1 && currentHeights[ejection1.idx] == ejection1.maxHeight) {
        ejection1.idx = -1;
    }

    for (columnIdx = 0; columnIdx < WIDTH; columnIdx++) {
        nextIntensity = max(maxIntensity - getIntensityShift(), 0);

        if (currentIntensities[columnIdx] > nextIntensity && currentIntensities[columnIdx] > minIntensity) {
            currentIntensities[columnIdx]--;
        } else if (currentIntensities[columnIdx] < nextIntensity) {
            currentIntensities[columnIdx]++;
        }
    }

    if (ejection0.idx > -1) {
        eject(ejection0);
    }
    if (ejection1.idx > -1) {
        eject(ejection1);
    }

    for (columnIdx = 0; columnIdx < WIDTH; columnIdx++) {
        if (isEjection(columnIdx)) {
            continue;
        }

        height = intensitiesMaxHeightsMap[currentIntensities[columnIdx]] - getHeightShift(currentIntensities[columnIdx]);

        if (currentHeights[columnIdx] > height && currentHeights[columnIdx] > 1) {
            currentHeights[columnIdx]--;
        } else if (currentHeights[columnIdx] < height) {
            currentHeights[columnIdx]++;
        }
    }

    if (scale >= 5) {
        // max ejections amount = 2
        for (columnIdx = 0; columnIdx < WIDTH && (ejection0.idx == -1 || ejection1.idx == -1); columnIdx++) {
            if (
                currentIntensities[columnIdx] != maxIntensity ||
                currentHeights[columnIdx] != intensitiesMaxHeightsMap[currentIntensities[columnIdx]] ||
                (ejection0.idx > -1 && getMinRoundDistance(ejection0.idx, columnIdx) < 4) ||
                (ejection1.idx > -1 && getMinRoundDistance(ejection1.idx, columnIdx) < 4)
            ) {
                continue;
            }


            if (rand() % 50 == 0) {
                ejectionHeight = getTotalMaxHeight(columnIdx) - getHeightShift(currentIntensities[columnIdx]);

                if (ejection0.idx == -1) {
                    createEjection(columnIdx, ejectionHeight, ejection0);
                } else {
                    createEjection(columnIdx, ejectionHeight, ejection1);
                }
            }
        }
    }

    // create matrix
    for (rowIdx = 0; rowIdx < HEIGHT; rowIdx++) {
        for (columnIdx = 0; columnIdx < WIDTH; columnIdx++) {
            if (rowIdx + 1 <= currentHeights[columnIdx]) {
                matrix[rowIdx][columnIdx] = getColor(rowIdx, columnIdx);
            } else {
                matrix[rowIdx][columnIdx] = {.r =  0, .g =  0, .b =  0};
            }
        }
    }

    display(matrix);
}

byte to = 20;
short buttonUpPressedTime = 0;
short buttonDownPressedTime = 0;
short buttonPressTO = to * 10;
short buttonPressTick = to * 5;
short buttonPressTimeMax = buttonPressTO + buttonPressTick * SCALE_MAX;

short buttonOffPressTime = to * 5;
short buttonIdleTimeTick = to * 50;
short buttonIdleTime = buttonIdleTimeTick;


byte doFire = 1;
int main() {
    setMaxIntensity();

    DDRB &= ~(_BV(BUTTON_UP) | _BV(BUTTON_DOWN));

    while (true) {
        if (doFire == 1) {
            fire();
        }

        if (buttonIdleTime < buttonIdleTimeTick) {
            buttonIdleTime = min(buttonIdleTime + to, buttonIdleTimeTick);
        }

        if (buttonIdleTime >= buttonIdleTimeTick) {
            if (PINB & _BV(BUTTON_UP)) {
                buttonUpPressedTime = min(buttonUpPressedTime + to, buttonPressTimeMax);
            } else {
                buttonUpPressedTime = 0;
            }

            if (PINB & _BV(BUTTON_DOWN)) {
                buttonDownPressedTime = min(buttonDownPressedTime + to, buttonPressTimeMax);
            } else {
                buttonDownPressedTime = 0;
            }

            if (buttonUpPressedTime > 0 && buttonDownPressedTime > 0) {
                if (buttonUpPressedTime >= buttonOffPressTime && buttonDownPressedTime >= buttonOffPressTime) {
                    shoutDownFire();

                    doFire = 0;

                    buttonIdleTime = 0;
                }
            } else if (buttonUpPressedTime > 0 || buttonDownPressedTime > 0) {
                doFire = 1;

                if (buttonUpPressedTime == to || (buttonUpPressedTime >= buttonPressTO && buttonUpPressedTime % buttonPressTick == 0)) {
                    incrementScale();
                } else if (buttonDownPressedTime == to || (buttonDownPressedTime >= buttonPressTO && buttonDownPressedTime % buttonPressTick == 0)) {
                    decrementScale();
                }
            }
        }

        _delay_ms(to);
    }
}