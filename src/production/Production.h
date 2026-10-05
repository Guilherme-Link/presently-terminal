#pragma once

#include <Arduino.h>

extern unsigned long nextRequestAt;

bool connectToConfiguredWifi();
bool registerTerminal();
bool requestQrCode();
bool authenticateTerminal();
void enterProductionMode();
