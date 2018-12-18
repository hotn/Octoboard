#include <Arduino.h>
#include <EEPROM.h>
#include <NeoPixelBus.h>
#include "octoboard.h"
#include "convert.h"

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
const int POWER_MODE_ROT_CLOCK_PIN = 2;
const int POWER_MODE_ROT_DATA_PIN = 4;
const int POWER_MODE_ROT_SWITCH_PIN = 8;
const int LED_STRIP_DATA_PIN = 7;
const int SETTINGS_POT_PIN = 5;
const int EDIT_BUTTON_SWITCH_PIN = 6;
const int EDIT_BUTTON_LIGHT_PIN = 12;

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
int pendingRunModeChange = 0; //-1 for reverse, 1 for forward, 0 for no change

//edit button values
int lastEditButtonState;
bool editModeIsActive;

//current mode values
RgbColor solidRgb;
RgbColor gradientLinearRgb1;
RgbColor gradientLinearRgb2;
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
    resetUnsavedChanges();
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
  gradientRotateSpeed = EEPROM.read(GRADIENT_ROTATE_MODE_ROTATE_SPEED);

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

  pendingRunModeChange = 0;
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

  editModeIsActive = newButtonValue == HIGH;

  Serial.println("Edit mode: " + String(editModeIsActive));

  digitalWrite(EDIT_BUTTON_LIGHT_PIN, editModeIsActive);
}

bool isInEditMode() {
  return editModeIsActive;
}

void saveChanges() {
  switch (getCurrentRunMode()) {
    case Mode::Solid:
      EEPROM.write(SOLID_MODE_RED_ADDRESS, solidRgb.R);
      EEPROM.write(SOLID_MODE_GREEN_ADDRESS, solidRgb.G);
      EEPROM.write(SOLID_MODE_BLUE_ADDRESS, solidRgb.B);
      break;
    case Mode::GradientLinear:
      //TODO
      break;
    case Mode::GradientCircular:
      //TODO
      break;
    case Mode::GradientRotating:
      //TODO
      break;
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
    int potVal = analogRead(SETTINGS_POT_PIN);

    currentColor = Convert::AnalogToColor(potVal);
  }
  else {
    currentColor = solidRgb;
  }

  strip.ClearTo(currentColor);
  strip.Show();
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