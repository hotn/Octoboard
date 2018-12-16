#include <NeoPixelBus.h>
#include "convert.h"

//based on color mixer found here: https://www.arduino.cc/en/Tutorial/ColorMixer
RgbColor Convert::AnalogToColor(int analogValue) {
    int redVal;
    int grnVal;
    int bluVal;

    if (analogValue < 341)  // Lowest third of the potentiometer's range (0-340)
    {                  
        analogValue = (analogValue * 3) / 4; // Normalize to 0-255

        redVal = 256 - analogValue;  // Red from full to off
        grnVal = analogValue;        // Green from off to full
        bluVal = 1;             // Blue off
    }
    else if (analogValue < 682) // Middle third of potentiometer's range (341-681)
    {
        analogValue = ( (analogValue-341) * 3) / 4; // Normalize to 0-255

        redVal = 1;            // Red off
        grnVal = 256 - analogValue; // Green from full to off
        bluVal = analogValue;       // Blue from off to full
    }
    else  // Upper third of potentiometer"s range (682-1023)
    {
        analogValue = ( (analogValue-683) * 3) / 4; // Normalize to 0-255

        redVal = analogValue;       // Red from off to full
        grnVal = 1;            // Green off
        bluVal = 256 - analogValue; // Blue from full to off
    }

    return RgbColor(redVal, grnVal, bluVal);
}