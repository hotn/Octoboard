#include <Arduino.h>
#include <EEPROM.h>
#include <NeoPixelBus.h>
#include "octoboard.h"

const Mode MODES[4] = {Mode::Solid, Mode::GradientLinear, Mode::GradientCircular, Mode::GradientRotating};
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
const int GRADIENT_ROTATE_MODE_ROTATE_SPEED = 29;

//Pin assignments
const int CLOCK_PIN = 2;
const int DATA_PIN = 4;
const int SWITCH_PIN = 8;

//power/run mode rotary values
int rotaryClockValue;
int rotaryDataValue;
int rotarySwitchValue;
int currentModeIndex = 0;
bool currentPowerState = true;
bool powerIsChanging = false;
int lastPowerButtonState = HIGH;
unsigned long lastPowerDebounceTime = 0;
unsigned long lastRunModeDebounceTime = 0;

//edit button values
bool editModeIsActive;

//current mode values
RgbwColor solidRgb;
RgbwColor gradientLinearRgb1;
RgbwColor gradientLinearRgb2;
RgbwColor gradientCircularRgb1;
RgbwColor gradientCircularRgb2;
RgbwColor gradientRotateRgb1;
RgbwColor gradientRotateRgb2;
int gradientRotateGradientType;
int gradientRotateSpeed;
bool pendingChangesExist; //TODO: this will likely be unnecessary

void setup() {
  Serial.begin(9600);

  loadSavedModeSettings();

  //power/mode rotary encoder
  rotaryClockValue = digitalRead(CLOCK_PIN);
  rotaryDataValue = digitalRead(DATA_PIN);
  rotarySwitchValue = digitalRead(SWITCH_PIN);
}

void loop() {
  checkPowerState();

  if (!isPowerOn()) {
    resetUnsavedChanges();
    return;
  }

  checkRunModeState();
  checkEditModeState();

  switch (getCurrentRunMode()) {
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

  solidRgb = RgbwColor(
    EEPROM.read(SOLID_MODE_RED_ADDRESS), 
    EEPROM.read(SOLID_MODE_GREEN_ADDRESS), 
    EEPROM.read(SOLID_MODE_BLUE_ADDRESS), 
    EEPROM.read(SOLID_MODE_BRIGHTNESS_ADDRESS));

  gradientLinearRgb1 = RgbwColor(
    EEPROM.read(GRADIENT_LINEAR_MODE_COLOR_1_RED_ADDRESS), 
    EEPROM.read(GRADIENT_LINEAR_MODE_COLOR_1_GREEN_ADDRESS), 
    EEPROM.read(GRADIENT_LINEAR_MODE_COLOR_1_BLUE_ADDRESS), 
    EEPROM.read(GRADIENT_LINEAR_MODE_COLOR_1_BRIGHTNESS_ADDRESS));

  gradientLinearRgb2 = RgbwColor(
    EEPROM.read(GRADIENT_LINEAR_MODE_COLOR_2_RED_ADDRESS), 
    EEPROM.read(GRADIENT_LINEAR_MODE_COLOR_2_GREEN_ADDRESS), 
    EEPROM.read(GRADIENT_LINEAR_MODE_COLOR_2_BLUE_ADDRESS), 
    EEPROM.read(GRADIENT_LINEAR_MODE_COLOR_2_BRIGHTNESS_ADDRESS));

  gradientCircularRgb1 = RgbwColor(
    EEPROM.read(GRADIENT_CIRCULAR_MODE_COLOR_1_RED_ADDRESS), 
    EEPROM.read(GRADIENT_CIRCULAR_MODE_COLOR_1_GREEN_ADDRESS), 
    EEPROM.read(GRADIENT_CIRCULAR_MODE_COLOR_1_BLUE_ADDRESS), 
    EEPROM.read(GRADIENT_CIRCULAR_MODE_COLOR_1_BRIGHTNESS_ADDRESS));

  gradientCircularRgb2 = RgbwColor(
    EEPROM.read(GRADIENT_CIRCULAR_MODE_COLOR_2_RED_ADDRESS), 
    EEPROM.read(GRADIENT_CIRCULAR_MODE_COLOR_2_GREEN_ADDRESS), 
    EEPROM.read(GRADIENT_CIRCULAR_MODE_COLOR_2_BLUE_ADDRESS), 
    EEPROM.read(GRADIENT_CIRCULAR_MODE_COLOR_2_BRIGHTNESS_ADDRESS));

  gradientRotateRgb1 = RgbwColor(
    EEPROM.read(GRADIENT_ROTATE_MODE_COLOR_1_RED_ADDRESS), 
    EEPROM.read(GRADIENT_ROTATE_MODE_COLOR_1_GREEN_ADDRESS), 
    EEPROM.read(GRADIENT_ROTATE_MODE_COLOR_1_BLUE_ADDRESS), 
    EEPROM.read(GRADIENT_ROTATE_MODE_COLOR_1_BRIGHTNESS_ADDRESS));

  gradientRotateRgb2 = RgbwColor(
    EEPROM.read(GRADIENT_ROTATE_MODE_COLOR_2_RED_ADDRESS), 
    EEPROM.read(GRADIENT_ROTATE_MODE_COLOR_2_GREEN_ADDRESS), 
    EEPROM.read(GRADIENT_ROTATE_MODE_COLOR_2_BLUE_ADDRESS), 
    EEPROM.read(GRADIENT_ROTATE_MODE_COLOR_2_BRIGHTNESS_ADDRESS));

  gradientRotateGradientType = EEPROM.read(GRADIENT_ROTATE_MODE_GRADIENT_TYPE_ADDRESS);
  gradientRotateSpeed = EEPROM.read(GRADIENT_ROTATE_MODE_ROTATE_SPEED);

  Serial.println("Saved settings loaded");
}

void checkPowerState() {
  int newSwitchValue = digitalRead(SWITCH_PIN);
  //Serial.println(newSwitchValue);

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
}

bool isPowerOn() {
    return currentPowerState;
}

void checkRunModeState() {
  int newClockValue = digitalRead(CLOCK_PIN);
  int newDataValue = digitalRead(DATA_PIN);

  

  //Only update debounce time when state changes. Otherwise, debounce return block will always fire.
  if (newClockValue != rotaryClockValue || newDataValue != rotaryDataValue) {
    lastPowerDebounceTime = millis();
  }

  rotaryClockValue = newClockValue;
  rotaryDataValue = newDataValue;

  //If a state change hasn't existed in its new state for more than the debounce delay time yet, return.
  if (millis() - lastPowerDebounceTime <= POWER_DEBOUNCE_DELAY) {
    Serial.println("Run mode rotary debounce");
    return;
  }

  if (newClockValue == rotaryClockValue && newDataValue != rotaryDataValue) {
    //mode rotated clockwise
    currentModeIndex = (currentModeIndex + 1) % sizeof(*MODES);
    Serial.println("Run mode change forward. Current run mode: " + String(MODES[currentModeIndex]));
  }
  else if (newClockValue != rotaryClockValue && newDataValue == rotaryDataValue) {
    //mode rotated counter-clockwise
    currentModeIndex = (currentModeIndex - 1) % sizeof(*MODES);
    Serial.println("Run mode change backward. Current run mode: " + String(MODES[currentModeIndex]));
  }
}

Mode getCurrentRunMode() {
  return MODES[currentModeIndex];
}

void checkEditModeState() {
  //TODO
}

bool isInEditMode() {
  return editModeIsActive;
}

void resetUnsavedChanges() {
  //TODO
  if (!pendingChangesExist) {
    return;
  }

  Serial.println("Resetting unsaved changes.");
}

void runSolidMode() {
  //TODO
}

void runGradientLinearMode() {
  //TODO
}

void runGradientCircularMode() {
  //TODO
}

void runGradientRotatingMode() {
  //TODO
}