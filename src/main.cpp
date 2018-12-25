#include <Arduino.h>
#include <EEPROM.h>
#include <NeoPixelBus.h>
#include "octoboard.h"
#include "convert.h"

const Mode MODES[4] = {Mode::Solid, Mode::GradientLinear, Mode::GradientCircular, Mode::GradientRotating};

//top to bottom
const int LINEAR_LED_INDEXES[20][7] = {
    {0,-1,-1,-1,-1,-1,-1},
    {1,2,-1,-1,-1,-1,-1},
    {3,4,5,6,7,-1,-1},
    {8,9,10,11,-1,-1,-1},
    {12,-1,-1,-1,-1,-1,-1},
    {14,13,15,16,17,-1,-1},
    {18,-1,-1,-1,-1,-1,-1},
    {23,19,24,20,25,21,22},
    {26,28,27,-1,-1,-1,-1},
    {30,31,32,-1,-1,-1,-1},
    {35,36,33,34,-1,-1,-1},
    {37,-1,-1,-1,-1,-1,-1},
    {40,41,39,38,-1,-1,-1},
    {42,43,-1,-1,-1,-1,-1},
    {44,45,46,47,48,-1,-1},
    {49,50,-1,-1,-1,-1,-1},
    {52,53,54,55,51,-1,-1},
    {56,-1,-1,-1,-1,-1,-1},
    {57,58,-1,-1,-1,-1,-1},
    {59,-1,-1,-1,-1,-1,-1}
};

//top-left to bottom-right
const int LINEAR_45_INDEXES[19][5] = {
    {3,-1,-1,-1,-1},
    {18,4,-1,-1,-1},
    {23,14,1,-1,-1},
    {13,8,0,-1,-1},
    {30,19,9,5,-1},
    {15,12,2,-1,-1},
    {31,26,10,6,-1},
    {35,24,20,16,11},
    {44,40,29,7,-1},
    {52,36,28,25,17},
    {53,45,41,33,-1},
    {54,39,34,27,21},
    {56,46,22,-1,-1},
    {55,47,32,-1,-1},
    {57,48,42,38,-1},
    {51,37,-1,-1,-1},
    {59,49,-1,-1,-1},
    {58,43,-1,-1,-1},
    {50,-1,-1,-1,-1}
};

//left to right
const int LINEAR_90_INDEXES[16][7] = {
    {18,-1,-1,-1,-1,-1,-1},
    {23,30,52,-1,-1,-1,-1},
    {14,44,53,-1,-1,-1,-1},
    {3,19,31,40,-1,-1,-1},
    {4,13,35,45,54,-1,-1},
    {8,26,56,-1,-1,-1,-1},
    {1,24,41,-1,-1,-1,-1},
    {9,15,29,36,46,55,57},
    {0,5,12,20,28,-1,-1},
    {10,16,25,33,39,47,59},
    {2,34,48,51,58,-1,-1},
    {11,27,42,-1,-1,-1,-1},
    {6,17,38,49,-1,-1,-1},
    {7,21,-1,-1,-1,-1,-1},
    {22,32,50,-1,-1,-1,-1},
    {37,43,-1,-1,-1,-1,-1}
};

//bottom-left to top-right
const int LINEAR_135_INDEXES[19][6] = {
    {52,-1,-1,-1,-1,-1},
    {53,-1,-1,-1,-1,-1},
    {44,54,56,-1,-1,-1},
    {57,-1,-1,-1,-1,-1},
    {45,55,59,-1,-1,-1},
    {30,40,58,-1,-1,-1},
    {23,46,-1,-1,-1,-1},
    {18,31,35,41,47,51},
    {19,36,48,-1,-1,-1},
    {14,26,39,49,-1,-1},
    {13,24,29,33,42,50},
    {28,34,38,-1,-1,-1},
    {3,15,20,25,43,-1},
    {4,8,9,27,37,-1},
    {12,16,32,-1,-1,-1},
    {1,5,10,21,-1,-1},
    {0,17,-1,-1,-1,-1},
    {2,11,-1,-1,-1,-1},
    {6,7,-1,-1,-1,-1}
};

//inner to outer
const int CIRCULAR_INDEXES[9][12] = {
    {29,28,36,33,-1,-1,-1,-1,-1,-1,-1,-1},
    {20,24,39,34,25,-1,-1,-1,-1,-1,-1,-1},
    {26,35,41,27,-1,-1,-1,-1,-1,-1,-1,-1},
    {16,15,31,46,47,42,-1,-1,-1,-1,-1,-1},
    {12,19,40,48,38,32,21,-1,-1,-1,-1,-1},
    {9,13,30,45,55,51,49,22,17,10,-1,-1},
    {5,8,14,23,44,54,37,11,-1,-1,-1,-1},
    {1,4,18,53,56,57,58,50,43,7,6,2},
    {0,3,52,59,-1,-1,-1,-1,-1,-1,-1,-1}
};

const unsigned long POWER_DEBOUNCE_DELAY = 50;
const unsigned long RUN_MODE_DEBOUNCE_DELAY = 50;

//Settings assignments
const int SOLID_MODE_BRIGHTNESS_ADDRESS = 0;
const int SOLID_MODE_RED_ADDRESS = 1;
const int SOLID_MODE_GREEN_ADDRESS = 2;
const int SOLID_MODE_BLUE_ADDRESS = 3;

const int GRADIENT_LINEAR_MODE_COLOR_1_BRIGHTNESS_ADDRESS = 4;
const int GRADIENT_LINEAR_MODE_COLOR_1_RED_ADDRESS = 5;
const int GRADIENT_LINEAR_MODE_COLOR_1_GREEN_ADDRESS = 6;
const int GRADIENT_LINEAR_MODE_COLOR_1_BLUE_ADDRESS = 7;
const int GRADIENT_LINEAR_MODE_COLOR_2_BRIGHTNESS_ADDRESS = 8;
const int GRADIENT_LINEAR_MODE_COLOR_2_RED_ADDRESS = 9;
const int GRADIENT_LINEAR_MODE_COLOR_2_GREEN_ADDRESS = 10;
const int GRADIENT_LINEAR_MODE_COLOR_2_BLUE_ADDRESS = 11;

const int GRADIENT_CIRCULAR_MODE_COLOR_1_BRIGHTNESS_ADDRESS = 12;
const int GRADIENT_CIRCULAR_MODE_COLOR_1_RED_ADDRESS = 13;
const int GRADIENT_CIRCULAR_MODE_COLOR_1_GREEN_ADDRESS = 14;
const int GRADIENT_CIRCULAR_MODE_COLOR_1_BLUE_ADDRESS = 15;
const int GRADIENT_CIRCULAR_MODE_COLOR_2_BRIGHTNESS_ADDRESS = 16;
const int GRADIENT_CIRCULAR_MODE_COLOR_2_RED_ADDRESS = 17;
const int GRADIENT_CIRCULAR_MODE_COLOR_2_GREEN_ADDRESS = 18;
const int GRADIENT_CIRCULAR_MODE_COLOR_2_BLUE_ADDRESS = 19;

const int GRADIENT_ROTATE_MODE_COLOR_1_BRIGHTNESS_ADDRESS = 20;
const int GRADIENT_ROTATE_MODE_COLOR_1_RED_ADDRESS = 21;
const int GRADIENT_ROTATE_MODE_COLOR_1_GREEN_ADDRESS = 22;
const int GRADIENT_ROTATE_MODE_COLOR_1_BLUE_ADDRESS = 23;
const int GRADIENT_ROTATE_MODE_COLOR_2_BRIGHTNESS_ADDRESS = 24;
const int GRADIENT_ROTATE_MODE_COLOR_2_RED_ADDRESS = 25;
const int GRADIENT_ROTATE_MODE_COLOR_2_GREEN_ADDRESS = 26;
const int GRADIENT_ROTATE_MODE_COLOR_2_BLUE_ADDRESS = 27;
const int GRADIENT_ROTATE_MODE_GRADIENT_TYPE_ADDRESS = 28;
const int GRADIENT_ROTATE_MODE_ROTATE_SPEED_ADDRESS = 29;
const int CURRENT_MODE_ADDRESS = 30;

//Pin assignments
const int POWER_MODE_ROT_CLOCK_PIN = 2;
const int POWER_MODE_ROT_DATA_PIN = 4;
const int POWER_MODE_ROT_SWITCH_PIN = 8;
const int LED_STRIP_DATA_PIN = 7;
const int SETTINGS_POT_1_PIN = 2;
const int SETTINGS_POT_2_PIN = 3;
const int SETTINGS_POT_3_PIN = 4;
const int SETTINGS_POT_4_PIN = 5;
const int EDIT_BUTTON_SWITCH_PIN = 6;
const int EDIT_BUTTON_LIGHT_PIN = 12;

//power/run mode rotary values
int rotaryClockValue;
int rotaryDataValue;
int rotarySwitchValue;
int currentModeIndex;
bool currentPowerState = true;
bool powerIsChanging = false;
int lastPowerButtonState = HIGH;
unsigned long lastPowerDebounceTime = 0;
unsigned long lastRunModeDebounceTime = 0;
int pendingRunModeChange = 0; //-1 for reverse, 1 for forward, 0 for no change

//edit button values
int lastEditButtonState;
bool editModeIsActive;

//current mode values
RgbColor solidRgb;
RgbColor gradientLinearRgb1;
RgbColor gradientLinearRgb2;
int gradientLinearBrightness;
RgbColor gradientCircularRgb1;
RgbColor gradientCircularRgb2;
RgbColor gradientRotateRgb1;
RgbColor gradientRotateRgb2;
int gradientRotateGradientType;
int gradientRotateSpeed;
bool pendingChangesExist; //TODO: this will likely be unnecessary

//pixels
const int PIXEL_COUNT = 60;
NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> strip(PIXEL_COUNT, LED_STRIP_DATA_PIN);

void setup() {
    Serial.begin(9600);

    loadSavedModeSettings();

    pinMode(EDIT_BUTTON_LIGHT_PIN, OUTPUT);
    lastEditButtonState = digitalRead(EDIT_BUTTON_SWITCH_PIN);

    //power/mode rotary encoder
    rotaryClockValue = digitalRead(POWER_MODE_ROT_CLOCK_PIN);
    rotaryDataValue = digitalRead(POWER_MODE_ROT_DATA_PIN);
    rotarySwitchValue = digitalRead(POWER_MODE_ROT_SWITCH_PIN);

    strip.Begin();
}

void loop() {
    checkPowerState();

    if (!isPowerOn()) {
        return;
    }

    Mode currentRunMode = getCurrentRunMode();

    checkRunModeState();

    Mode newRunMode = getCurrentRunMode();

    if (currentRunMode != newRunMode) {
        resetUnsavedChanges();
    }
    else {
        checkEditModeState();
    }

    switch (newRunMode) {
        case Mode::Solid:
            runSolidMode();
            break;
        case Mode::GradientLinear:
            runGradientLinearMode();
            break;
        case Mode::GradientCircular:
            runGradientCircularMode();
            break;
        case Mode::GradientRotating:
            runGradientRotatingMode();
            break;
    }
}

void loadSavedModeSettings() {
    Serial.println("Loading saved settings");

    solidRgb = RgbColor(
        EEPROM.read(SOLID_MODE_RED_ADDRESS), 
        EEPROM.read(SOLID_MODE_GREEN_ADDRESS), 
        EEPROM.read(SOLID_MODE_BLUE_ADDRESS));

    gradientLinearRgb1 = RgbColor(
        EEPROM.read(GRADIENT_LINEAR_MODE_COLOR_1_RED_ADDRESS), 
        EEPROM.read(GRADIENT_LINEAR_MODE_COLOR_1_GREEN_ADDRESS), 
        EEPROM.read(GRADIENT_LINEAR_MODE_COLOR_1_BLUE_ADDRESS));

    gradientLinearRgb2 = RgbColor(
        EEPROM.read(GRADIENT_LINEAR_MODE_COLOR_2_RED_ADDRESS), 
        EEPROM.read(GRADIENT_LINEAR_MODE_COLOR_2_GREEN_ADDRESS), 
        EEPROM.read(GRADIENT_LINEAR_MODE_COLOR_2_BLUE_ADDRESS));

    gradientCircularRgb1 = RgbColor(
        EEPROM.read(GRADIENT_CIRCULAR_MODE_COLOR_1_RED_ADDRESS), 
        EEPROM.read(GRADIENT_CIRCULAR_MODE_COLOR_1_GREEN_ADDRESS), 
        EEPROM.read(GRADIENT_CIRCULAR_MODE_COLOR_1_BLUE_ADDRESS));

    gradientCircularRgb2 = RgbColor(
        EEPROM.read(GRADIENT_CIRCULAR_MODE_COLOR_2_RED_ADDRESS), 
        EEPROM.read(GRADIENT_CIRCULAR_MODE_COLOR_2_GREEN_ADDRESS), 
        EEPROM.read(GRADIENT_CIRCULAR_MODE_COLOR_2_BLUE_ADDRESS));

    gradientRotateRgb1 = RgbColor(
        EEPROM.read(GRADIENT_ROTATE_MODE_COLOR_1_RED_ADDRESS), 
        EEPROM.read(GRADIENT_ROTATE_MODE_COLOR_1_GREEN_ADDRESS), 
        EEPROM.read(GRADIENT_ROTATE_MODE_COLOR_1_BLUE_ADDRESS));

    gradientRotateRgb2 = RgbColor(
        EEPROM.read(GRADIENT_ROTATE_MODE_COLOR_2_RED_ADDRESS), 
        EEPROM.read(GRADIENT_ROTATE_MODE_COLOR_2_GREEN_ADDRESS), 
        EEPROM.read(GRADIENT_ROTATE_MODE_COLOR_2_BLUE_ADDRESS));

    gradientRotateGradientType = EEPROM.read(GRADIENT_ROTATE_MODE_GRADIENT_TYPE_ADDRESS);
    gradientRotateSpeed = EEPROM.read(GRADIENT_ROTATE_MODE_ROTATE_SPEED_ADDRESS);

    currentModeIndex = EEPROM.read(CURRENT_MODE_ADDRESS);

    Serial.println("Saved speed: " + String(gradientRotateSpeed));

    Serial.println("Saved settings loaded");
}

void checkPowerState() {
    int newSwitchValue = digitalRead(POWER_MODE_ROT_SWITCH_PIN);
    
    //Only update debounce time when state changes. Otherwise, debounce return block will always fire.
    if (newSwitchValue != lastPowerButtonState) {
        lastPowerDebounceTime = millis();
    }

    lastPowerButtonState = newSwitchValue;

    //If a state change hasn't existed in its new state for more than the debounce delay time yet, return.
    if (millis() - lastPowerDebounceTime <= POWER_DEBOUNCE_DELAY) {
        Serial.println("Power button debounce");
        return;
    }

    //Check if button is idle.
    if (newSwitchValue == HIGH && !powerIsChanging) {
        return;
    }

    //Check if button is currently being pushed.
    if (newSwitchValue == LOW) {
        //Check if button is currently being held in.
        if (powerIsChanging) {
            return;
        }

        //Button has just been pressed, but not yet released.
        Serial.println("Power state changing.");
        powerIsChanging = true;
        return;
    }

    //Button has just been released.
    powerIsChanging = false;
    currentPowerState = !currentPowerState;

    Serial.println("Power state changed. Power on: " + String(currentPowerState));

    if (!isPowerOn()) {
        resetUnsavedChanges();
        strip.ClearTo(RgbColor(0));
        strip.Show();
    }
}

bool isPowerOn() {
    return currentPowerState;
}

void checkRunModeState() {
    int newClockValue = digitalRead(POWER_MODE_ROT_CLOCK_PIN);
    int newDataValue = digitalRead(POWER_MODE_ROT_DATA_PIN);

    bool clockChanged = newClockValue != rotaryClockValue;
    bool dataChanged = newDataValue != rotaryDataValue;

    //Only update debounce time when state changes. Otherwise, debounce return block will always fire.
    if (clockChanged || dataChanged) {
        Serial.println("Run mode clock: " + String(newClockValue) + " data: " + String(newDataValue));
        lastPowerDebounceTime = millis();

        //Because numerous value changes occur with each rotation, we need to track only the first change and let the rest debounce.
        if (pendingRunModeChange == 0) {
            if (clockChanged) {
                pendingRunModeChange = -1;
            }
            else {
                pendingRunModeChange = 1;
            }
        }
    }

    rotaryClockValue = newClockValue;
    rotaryDataValue = newDataValue;

    //If a state change hasn't existed in its new state for more than the debounce delay time yet, return.
    if (millis() - lastPowerDebounceTime <= POWER_DEBOUNCE_DELAY) {
        Serial.println("Run mode rotary debounce");
        return;
    }

    if (pendingRunModeChange == 0) {
        //no incoming change
        return;
    }

    int modeCount = sizeof(MODES) / sizeof(*MODES);
    Serial.println(modeCount);

    if (pendingRunModeChange == 1) {
        //mode rotated clockwise
        currentModeIndex = (currentModeIndex + 1) % modeCount;
        Serial.println("Run mode change forward. Current run mode: " + String(MODES[currentModeIndex]));
    }
    else if (pendingRunModeChange == -1) {
        //mode rotated counter-clockwise
        //modulo doesn't wrap around when using negatives, so we can't be quite as fancy here.
        currentModeIndex--;
        if (currentModeIndex < 0) {
            currentModeIndex += modeCount;
        }
        Serial.println("Run mode change backward. Current run mode: " + String(MODES[currentModeIndex]));
    }

    if (pendingRunModeChange != 0) {
        EEPROM.write(CURRENT_MODE_ADDRESS, currentModeIndex);
        pendingRunModeChange = 0;
    }
}

Mode getCurrentRunMode() {
    return MODES[currentModeIndex];
}

void checkEditModeState() {
    int newButtonValue = digitalRead(EDIT_BUTTON_SWITCH_PIN);
    
    if (newButtonValue == lastEditButtonState) {
        return;
    }

    lastEditButtonState = newButtonValue;

    bool editModeWasActive = editModeIsActive;

    editModeIsActive = newButtonValue == HIGH;

    if (editModeWasActive && !editModeIsActive) {
        saveChanges();
    }

    Serial.println("Edit mode: " + String(editModeIsActive));

    digitalWrite(EDIT_BUTTON_LIGHT_PIN, editModeIsActive);
}

bool isInEditMode() {
    return editModeIsActive;
}

void saveChanges() {
    Mode mode = getCurrentRunMode();

    Serial.println("Saving changes for mode " + String(mode));

    switch (mode) {
        case Mode::Solid: {
            solidRgb = strip.GetPixelColor(0);

            EEPROM.write(SOLID_MODE_RED_ADDRESS, solidRgb.R);
            EEPROM.write(SOLID_MODE_GREEN_ADDRESS, solidRgb.G);
            EEPROM.write(SOLID_MODE_BLUE_ADDRESS, solidRgb.B);
            break;
        }
        case Mode::GradientLinear: {
            int potBrightnessVal = analogRead(SETTINGS_POT_1_PIN);
            int potStartVal = analogRead(SETTINGS_POT_2_PIN);
            int potEndVal = analogRead(SETTINGS_POT_3_PIN);

            gradientLinearBrightness = potBrightnessVal;
            gradientLinearRgb1 = Convert::AnalogToColor(potStartVal);
            gradientLinearRgb2 = Convert::AnalogToColor(potEndVal);

            //TODO: apply direction of gradient
            //int potDirectionVal = analogRead(SETTINGS_POT_4_PIN);

            EEPROM.write(GRADIENT_LINEAR_MODE_COLOR_1_BRIGHTNESS_ADDRESS, potBrightnessVal);
            EEPROM.write(GRADIENT_LINEAR_MODE_COLOR_1_RED_ADDRESS, gradientLinearRgb1.R);
            EEPROM.write(GRADIENT_LINEAR_MODE_COLOR_1_GREEN_ADDRESS, gradientLinearRgb1.G);
            EEPROM.write(GRADIENT_LINEAR_MODE_COLOR_1_BLUE_ADDRESS, gradientLinearRgb1.B);

            EEPROM.write(GRADIENT_LINEAR_MODE_COLOR_2_BRIGHTNESS_ADDRESS, potBrightnessVal);
            EEPROM.write(GRADIENT_LINEAR_MODE_COLOR_2_RED_ADDRESS, gradientLinearRgb2.R);
            EEPROM.write(GRADIENT_LINEAR_MODE_COLOR_2_GREEN_ADDRESS, gradientLinearRgb2.G);
            EEPROM.write(GRADIENT_LINEAR_MODE_COLOR_2_BLUE_ADDRESS, gradientLinearRgb2.B);
            break;
        }
        case Mode::GradientCircular: {
            //TODO
            break;
        }
        case Mode::GradientRotating: {
            //TODO
            break;
        }
        default: {
            //shouldn't need to do anything here
        }
    }

    for (int i = 0; i < 4; i++) {
        digitalWrite(EDIT_BUTTON_LIGHT_PIN, HIGH);
        delay(300);
        digitalWrite(EDIT_BUTTON_LIGHT_PIN, LOW);
        delay(300);
    }
}

void resetUnsavedChanges() {
    Serial.println("Resetting unsaved changes.");

    editModeIsActive = false;
    digitalWrite(EDIT_BUTTON_LIGHT_PIN, editModeIsActive);
}

void runSolidMode() {
    RgbColor currentColor;
    if (isInEditMode()) {
        int brightnessVal = analogRead(SETTINGS_POT_1_PIN);
        int colorVal = analogRead(SETTINGS_POT_2_PIN);

        currentColor = Convert::AnalogToColor(colorVal);
        currentColor = Convert::ColorToBrightnessAdjustedColor(currentColor, brightnessVal, 5);
    }
    else {
        currentColor = solidRgb;
    }

    strip.ClearTo(currentColor);
    strip.Show();
}

void runGradientLinearMode() {
    RgbColor currentStartColor;
    RgbColor currentEndColor;
    int currentBrightness;
    RgbColor* colors;

    if (isInEditMode()) {
        int potStartVal = analogRead(SETTINGS_POT_2_PIN);
        int potEndVal = analogRead(SETTINGS_POT_3_PIN);

        //TODO: provide mechanism to adjust start/end brightnesses independently

        //TODO: apply direction of gradient
        //int potDirectionVal = analogRead(SETTINGS_POT_4_PIN);

        currentStartColor = Convert::AnalogToColor(potStartVal);
        currentEndColor = Convert::AnalogToColor(potEndVal);
        currentBrightness = analogRead(SETTINGS_POT_1_PIN);
    }
    else {
        currentStartColor = gradientLinearRgb1;
        currentEndColor = gradientLinearRgb2;
        currentBrightness = gradientLinearBrightness;
    }

    colors = Convert::ColorRangeToColors(currentStartColor, currentEndColor, PIXEL_COUNT);

    for(int i = 0; i < PIXEL_COUNT; i++) {
        RgbColor color = Convert::ColorToBrightnessAdjustedColor(colors[i], currentBrightness, 5);
        strip.SetPixelColor(i, color);
    }

    delete[] colors;

    strip.Show();
}

void runGradientCircularMode() {
    //TODO
}

void runGradientRotatingMode() {
    //TODO
}