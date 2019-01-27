#ifndef LED_H
#define LED_H

class LED {
    public:
        int pin;

        int value;

        int brightness;

        int GetAdjustedValue();
};

#endif