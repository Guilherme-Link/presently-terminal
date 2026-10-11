#include <Arduino.h>
#include <WiFi.h>

#include "config/Configuration.h"
#include "display/Display.h"
#include "wifi/ConfigurationWifi.h"
#include "web/ConfigurationWeb.h"
#include "production/Production.h"

// ============================================================
// Hardware
// ============================================================

constexpr uint8_t modeSwitchPin = 27;

// ============================================================
// Runtime
// ============================================================

enum TerminalMode {
    MODE_CONFIG,
    MODE_PRODUCTION
};

TerminalMode currentMode = MODE_CONFIG;
boolean registered = false;
boolean authenticated = false;

// ============================================================
// Mode switch
// ============================================================

TerminalMode readMode() {
    if (
        digitalRead(modeSwitchPin) == LOW
    ) {
        return MODE_PRODUCTION;
    }

    return MODE_CONFIG;
}

// ============================================================
// Setup
// ============================================================

void setup() {
    Serial.begin(115200);

    pinMode(
        modeSwitchPin,
        INPUT_PULLUP
    );

    initializeDisplay();

    showMessage(
        "INICIANDO",
        "Terminal QR..."
    );

    loadConfiguration();

    currentMode = readMode();

    if (
        currentMode == MODE_CONFIG
    ) {
        startConfigurationAccessPoint();
        startConfigurationServer();

        return;
    }

    enterProductionMode();
    authenticated = authenticateTerminal();
    registered = registerTerminal();
}

// ============================================================
// Loop
// ============================================================

void loop() {
    TerminalMode detectedMode = readMode();

    if (detectedMode != currentMode) {

        currentMode = detectedMode;

        // Reiniciar simplifica a transição:
        // limpa sockets, HTTP, WiFi e AP.
        ESP.restart();
    }

    if (currentMode == MODE_CONFIG) {

        handleConfigurationDns();

        handleConfigurationServer();

        return;
    }

    // --------------------------------------------------------
    // Production
    // --------------------------------------------------------

    if (WiFi.status() != WL_CONNECTED) {

        if (millis() >= nextRequestAt) {

            connectToConfiguredWifi();
        }

        return;
    }

    if (millis() >= nextRequestAt) {
        
        if (!registered) {
            registered = registerTerminal();
        }
        else {
            requestQrCode();
        }
        
    }
}