#pragma once

#include <Arduino.h>

struct TerminalConfig {
    String wifiSsid;
    String wifiPassword;
    String serverAddress;
    String friendlyName;
    String terminalId;
    String username;
    String password;
};

extern TerminalConfig terminalConfig;

void loadConfiguration();
void saveConfiguration();
bool hasConfiguration();

String getServerBaseUrl();