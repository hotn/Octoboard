#include <NeoPixelBus.h>

class Convert {
    public:
        /**
         * Convert an analog value ranging from 0 to 1023 to an RgbColor object.
         */
        static RgbColor AnalogToColor(int analogValue);
}