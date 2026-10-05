#include "Configuration.h"

#include <Preferences.h>

// ============================================================
// Configuration
// ============================================================

constexpr char preferencesNamespace[] = "terminal";

constexpr char wifiSsidKey[] = "wifiSsid";
constexpr char wifiPasswordKey[] = "wifiPassword";
constexpr char serverAddressKey[] = "server";
constexpr char friendlyNameKey[] = "name";
constexpr char terminalIdKey[] = "terminalId";

// ============================================================
// Persistent configuration
// ============================================================

TerminalConfig terminalConfig;

Preferences preferences;

// ============================================================
// Terminal identity
// ============================================================

String generateTerminalId() {
    uint64_t chipId =
        ESP.getEfuseMac();

    uint32_t value =
        static_cast<uint32_t>(
            chipId & 0xFFFFFF
        );

    char buffer[16];

    snprintf(
        buffer,
        sizeof(buffer),
        "T-%06lX",
        static_cast<unsigned long>(value)
    );

    return String(buffer);
}

// ============================================================
// Preferences
// ============================================================

void loadConfiguration() {
    preferences.begin(
        preferencesNamespace,
        true
    );

    terminalConfig.wifiSsid =
        preferences.getString(
            wifiSsidKey,
            ""
        );

    terminalConfig.wifiPassword =
        preferences.getString(
            wifiPasswordKey,
            ""
        );

    terminalConfig.serverAddress =
        preferences.getString(
            serverAddressKey,
            ""
        );

    terminalConfig.friendlyName =
        preferences.getString(
            friendlyNameKey,
            ""
        );

    terminalConfig.terminalId =
        preferences.getString(
            terminalIdKey,
            ""
        );

    preferences.end();

    if (
        terminalConfig.terminalId.isEmpty()
    ) {
        terminalConfig.terminalId =
            generateTerminalId();

        preferences.begin(
            preferencesNamespace,
            false
        );

        preferences.putString(
            terminalIdKey,
            terminalConfig.terminalId
        );

        preferences.end();
    }
}

void saveConfiguration() {
    preferences.begin(
        preferencesNamespace,
        false
    );

    preferences.putString(
        wifiSsidKey,
        terminalConfig.wifiSsid
    );

    preferences.putString(
        wifiPasswordKey,
        terminalConfig.wifiPassword
    );

    preferences.putString(
        serverAddressKey,
        terminalConfig.serverAddress
    );

    preferences.putString(
        friendlyNameKey,
        terminalConfig.friendlyName
    );

    preferences.putString(
        terminalIdKey,
        terminalConfig.terminalId
    );

    preferences.end();
}

bool hasConfiguration() {
    return
        !terminalConfig.wifiSsid.isEmpty() &&
        !terminalConfig.serverAddress.isEmpty() &&
        !terminalConfig.friendlyName.isEmpty();
}

// ============================================================
// Server address
// ============================================================

String getServerBaseUrl() {
    String address =
        terminalConfig.serverAddress;

    address.trim();

    if (
        !address.startsWith("http://") &&
        !address.startsWith("https://")
    ) {
        address =
            "http://" + address;
    }

    while (
        address.endsWith("/")
    ) {
        address.remove(
            address.length() - 1
        );
    }

    return address;
}