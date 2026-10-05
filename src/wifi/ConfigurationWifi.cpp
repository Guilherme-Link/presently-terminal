#include "ConfigurationWifi.h"

#include <WiFi.h>
#include <DNSServer.h>

#include "../config/Configuration.h"
#include "../display/Display.h"

// ============================================================
// Configuration AP
// ============================================================

const IPAddress configurationIp(
    192, 168, 4, 1
);

constexpr uint16_t dnsPort = 53;

constexpr char accessPointPassword[] =
    "config123";

DNSServer dnsServer;

// ============================================================
// Configuration WiFi
// ============================================================

void handleConfigurationDns() {
    dnsServer.processNextRequest();
}


String getConfigurationSsid() {
    return "QR-SETUP-" +
           terminalConfig.terminalId.substring(2);
}

void startConfigurationAccessPoint() {
    String ssid =
        getConfigurationSsid();

    WiFi.mode(WIFI_AP);

    WiFi.softAPConfig(
        configurationIp,
        configurationIp,
        IPAddress(255, 255, 255, 0)
    );

    if (
        !WiFi.softAP(
            ssid.c_str(),
            accessPointPassword
        )
    ) {
        showError(
            "Nao foi possivel",
            "criar a rede de config."
        );

        return;
    }

    dnsServer.start(
        dnsPort,
        "*",
        configurationIp
    );

    showMessage(
        "CONFIGURACAO",
        "Conecte-se a:",
        ssid,
        "Senha: config123",
        "Preencha o form"
    );
}