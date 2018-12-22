#include <NeoPixelBus.h>

class Convert {
    public:
        /**
         * Convert an analog value ranging from 0 to 1023 to an RgbColor object.
         */
        static RgbColor AnalogToColor(int analogValue);

        /**
         * Convert a range of values and a step count to an array of RgbColor objects that
         * transitions from the start value as a color through the end value as a color.
         */
        static RgbColor* AnalogRangeToColors(int startValue, int endValue, int stepCount);

        /**
         * Convert an RgbColor object to an equivalent value ranging from 0 to 1023.
         */
        static int ColorToAnalog(RgbColor color);

        /**
         * Convert a range of RgbColor objects and a step count to an array of analog
         * values ranging from 0 to 1023 that transitions in values from the start color
         * to the end color.
         */
        static int* ColorRangeToAnalogs(RgbColor startColor, RgbColor endColor, int stepCount);

        /**
         * Convert a range of RgbColor objects and a step count to an array of RgbColor
         * objects representing a color transition from start to end colors.
         */
        static RgbColor* ColorRangeToColors(RgbColor startColor, RgbColor endColor, int stepCount);

        /**
         * Convert an RgbColor object to a brightness-adjusted color object based on an
         * analog brightness value ranging from 0 to 1023. Optional parameters may
         * be provided to specify the lowest and highest brightness values allowed.
         */
        static RgbColor ColorToBrightnessAdjustedColor(RgbColor color, int analogValue, int minimumBrightness = 0, int maximumBrightness = 100);
        
    private:
        // Disallow creating an instance of this object
        Convert() {}
};