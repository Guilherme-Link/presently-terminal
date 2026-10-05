#pragma once

#include <Arduino.h>

void initializeDisplay();

void clearDisplay();

void showMessage(
    const String& line1,
    const String& line2 = "",
    const String& line3 = "",
    const String& line4 = "",
    const String& line5 = ""
);

void showError(
    const String& message,
    const String& instruction = ""
);

void showQrCode(
    const String& code
);