#include "modes.h"

/**
 * Load any setting changes that have been written to EEPROM for persistence.
 */
void loadSavedModeSettings();

/**
 * Determine the current state of the power button and update the power state of the device accordingly.
 */
void checkPowerState();

/**
 * Get the value for whether or not the device power state is currently on.
 */
bool isPowerOn();

/**
 * Determine whether or not the current device display mode is changing and update the current mode accordingly.
 */
void checkRunModeState();

/**
 * Get the value for which mode the device is currently set to be running on.
 */
Mode getCurrentRunMode();

/**
 * Determine the current state of the edit mode button and update the edit mode state of the device accordingly.
 */
void checkEditModeState();

/**
 * Get the value for whether or not the device is currently in edit mode for the current run mode and current setting.
 */
bool isInEditMode();

/**
 * Save any changes made to the current mode.
 */
void saveChanges();

/**
 * Remove from memory any pending but unsaved settings changes and turn off edit mode.
 */
void resetUnsavedChanges();

/**
 * Run the device in solid color mode.
 */
void runSolidMode();

/**
 * Run the device in linear gradient color mode.
 */
void runGradientLinearMode();

/**
 * Run the device in circular gradient color mode.
 */
void runGradientCircularMode();

/**
 * Run the device in rotating gradient color mode.
 */
void runGradientRotatingMode();