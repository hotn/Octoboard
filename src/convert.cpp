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

        redVal = 255 - analogValue;  // Red from full to off
        grnVal = analogValue;        // Green from off to full
        bluVal = 0;             // Blue off
    }
    else if (analogValue < 682) // Middle third of potentiometer's range (341-681)
    {
        analogValue = ( (analogValue-341) * 3) / 4; // Normalize to 0-255

        redVal = 0;            // Red off
        grnVal = 255 - analogValue; // Green from full to off
        bluVal = analogValue;       // Blue from off to full
    }
    else  // Upper third of potentiometer"s range (682-1023)
    {
        analogValue = ( (analogValue-683) * 3) / 4; // Normalize to 0-255

        redVal = analogValue;       // Red from off to full
        grnVal = 0;            // Green off
        bluVal = 255 - analogValue; // Blue from full to off
    }

    return RgbColor(redVal, grnVal, bluVal);
}

RgbColor* Convert::AnalogRangeToColors(int startValue, int endValue, int stepCount) {
    RgbColor* colors = new RgbColor[stepCount];

    double stepAmount = (endValue - startValue) / (double)(stepCount - 1);
    for(int i = 0; i < stepCount; i++) {
        colors[i] = AnalogToColor(startValue + round(stepAmount * i));
    }

    return colors;
}

int Convert::ColorToAnalog(RgbColor color) {
    if (color.R == 0) {
        return color.B * 4 / 3 + 341;
    }
    else if (color.G == 0) {
        return color.R  * 4 / 3 + 683;
    }
    else if (color.B == 0) {
        return color.G * 4 / 3;
    }

    return 0;
}

int* Convert::ColorRangeToAnalogs(RgbColor startColor, RgbColor endColor, int stepCount) {
    int* values = new int[stepCount];

    int startValue = ColorToAnalog(startColor);
    int endValue = ColorToAnalog(endColor);
    double stepAmount = (endValue - startValue) / (double)(stepCount - 1);
    for(int i = 0; i < stepCount; i++) {
        values[i] = startValue + round(stepAmount * i);
    }

    return values;
}

RgbColor* Convert::ColorRangeToColors(RgbColor startColor, RgbColor endColor, int stepCount) {
    int* values = ColorRangeToAnalogs(startColor, endColor, stepCount);
    RgbColor* colors = new RgbColor[stepCount];

    for(int i = 0; i < stepCount; i++) {
        colors[i] = AnalogToColor(values[i]);
    }

    delete[] values;

    return colors;
}