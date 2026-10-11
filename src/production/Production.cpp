#include "Production.h"

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>

#include "../config/Configuration.h"
#include "../display/Display.h"

// ============================================================
// Production configuration
// ============================================================

constexpr unsigned long retryIntervalMs = 30000;
constexpr unsigned long wifiConnectionTimeoutMs = 20000;
constexpr unsigned long httpTimeoutMs = 10000;

// ============================================================
// Backend endpoints
// ============================================================

constexpr char identifyEndpoint[] =
    "/api/v1/terminals/identify";

constexpr char qrcodeEndpoint[] =
    "/api/v1/qrcode/terminal";

// ============================================================
// Runtime
// ============================================================

unsigned long nextRequestAt = 0;
String authenticationCookie = "";

// ============================================================
// Production WiFi
// ============================================================

bool connectToConfiguredWifi() {
    showMessage(
        "CONECTANDO",
        "Rede:",
        terminalConfig.wifiSsid,
        "Aguarde..."
    );

    WiFi.mode(WIFI_STA);

    WiFi.begin(
        terminalConfig.wifiSsid.c_str(),
        terminalConfig.wifiPassword.c_str()
    );

    unsigned long startTime =
        millis();

    while (
        WiFi.status() != WL_CONNECTED &&
        millis() - startTime <
            wifiConnectionTimeoutMs
    ) {
        delay(500);
    }

    if (
        WiFi.status() != WL_CONNECTED
    ) {
        showError(
            "Nao conectou ao WiFi.",
            "Verifique a configuracao."
        );

        nextRequestAt =
            millis() + retryIntervalMs;

        return false;
    }

    showMessage(
        "WIFI CONECTADO",
        "Conectando ao servidor..."
    );

    return true;
}

// ============================================================
// HTTP identification
// ============================================================

int identifyTerminal() {
    String url =
        getServerBaseUrl() +
        identifyEndpoint;

    HTTPClient http;
    WiFiClientSecure client;
    client.setInsecure();

    if (!http.begin(client, url)) {
        showError(
            "Servidor invalido.",
            "Verifique o endereco."
        );

        return -1;
    }

    http.setTimeout(httpTimeoutMs);

    http.addHeader(
        "Content-Type",
        "application/json"
    );

    if (authenticationCookie.length() > 0) {
        http.addHeader(
            "Cookie",
            authenticationCookie
        );
    }

    JsonDocument document;

    document["terminalId"] = terminalConfig.terminalId;

    document["name"] = terminalConfig.friendlyName;

    String requestBody;

    serializeJson(
        document,
        requestBody
    );

    int statusCode = http.POST(requestBody);

    http.end();

    return statusCode;
}

// ============================================================
// Identification flow
// ============================================================

bool registerTerminal() {
    showMessage(
        "IDENTIFICANDO",
        "Terminal:",
        terminalConfig.terminalId,
        "Aguarde..."
    );

    int statusCode = identifyTerminal();

    if (statusCode < 0) {
        showError(
            "Servidor indisponivel",
            "Tentando novamente. "
        );

        nextRequestAt =
            millis() + retryIntervalMs;

        return false;
    }

    if (statusCode == 202) {
        showMessage(
            "AGUARDANDO",
            "CONFIGURACAO",
            "NO SERVIDOR",
            "Aguarde..."
        );

        nextRequestAt = millis() + retryIntervalMs;

        return true;
    }

    if (statusCode == 403) {
        showMessage(
            "NÃO AUTORIZADO",
            "Tentando novamente...",
            "Caso persista:",
            "Verifique credenciais"
        );

        authenticateTerminal();
    }

    if (
        statusCode == 409 ||
        (statusCode >= 200 && statusCode < 300)
    ) {
        showMessage(
            "TERMINAL ATIVO",
            "Buscando codigo..."
        );

        nextRequestAt = 0;

        return true;
    }

    
    showError(
        "Servidor recusou",
        "o terminal."
    );

    nextRequestAt = millis() + retryIntervalMs;

    return false;
}

// ============================================================
// QR code request
// ============================================================

bool requestQrCode() {
    String url =
        getServerBaseUrl() +
        qrcodeEndpoint;

    HTTPClient http;
    WiFiClientSecure client;
    client.setInsecure();

    if (!http.begin(client, url)) {
        showError(
            "Endereco invalido.",
            "Verifique o servidor."
        );

        nextRequestAt =
            millis() + retryIntervalMs;

        return false;
    }

    http.setTimeout(
        httpTimeoutMs
    );

    http.addHeader(
        "X-Terminal-Id",
        terminalConfig.terminalId
    );

    if (authenticationCookie.length() > 0) {
        http.addHeader(
            "Cookie",
            authenticationCookie
        );
    }

    int statusCode =
        http.GET();

    if (statusCode < 0) {
        http.end();

        showError(
            "Servidor indisponivel",
            "Tentando novamente. "
        );

        nextRequestAt =
            millis() + retryIntervalMs;

        return false;
    }

    if (statusCode == 200) {
        String response =
            http.getString();

        http.end();

        JsonDocument document;

        DeserializationError error =
            deserializeJson(
                document,
                response
            );

        if (error) {
            showError(
                "Resposta invalida.",
                "Contate o administrador."
            );

            nextRequestAt =
                millis() + retryIntervalMs;

            return false;
        }

        const char* code = document["code"];

        int waitSeconds =
            document["waitSeconds"] | 30;

        if (
            code == nullptr ||
            strlen(code) == 0
        ) {
            showError(
                "Codigo nao encontrado.",
                "Tentando novamente."
            );

            nextRequestAt =
                millis() + retryIntervalMs;

            return false;
        }

        if (waitSeconds < 1) {
            waitSeconds = 30;
        }

        showQrCode(String(code));

        nextRequestAt =
            millis() +
            static_cast<unsigned long>(
                waitSeconds
            ) * 1000UL;

        return true;
    }

    http.end();

    if (statusCode == 403) {
        showMessage(
            "NÃO AUTORIZADO",
            "Tentando novamente...",
            "Caso persista:",
            "Verifique credenciais"
        );

        authenticateTerminal();
    }

    if (statusCode == 202) {
        showMessage(
            "AGUARDANDO",
            "CONFIGURACAO",
            "NO SERVIDOR"
        );
    } else {
        Serial.println("Inesperada no qrcode");
        showError(
            "Resposta inesperada.",
            "HTTP " + String(statusCode)
        );
    }

    nextRequestAt = millis() + retryIntervalMs;

    return false;
}

// ============================================================
// Production mode
// ============================================================

void enterProductionMode() {
    if (!hasConfiguration()) {
        showError(
            "Terminal nao configurado.",
            "Mude o switch para CONFIG."
        );

        nextRequestAt = millis() + retryIntervalMs;

        return;
    }

    if (!connectToConfiguredWifi()) {
        return;
    }

}

// ============================================================
// Authentication
// ============================================================

bool authenticateTerminal() {
    String url =
        getServerBaseUrl() +
        "/security/v1/login";

    HTTPClient http;
    WiFiClientSecure client;

    client.setInsecure();

    if (!http.begin(client, url)) {
        showError(
            "Servidor invalido.",
            "Verifique o endereco."
        );

        nextRequestAt =
            millis() + retryIntervalMs;

        return false;
    }

    http.setTimeout(httpTimeoutMs);

    const char* responseHeaders[] = {"Set-Cookie"};

    http.collectHeaders(responseHeaders, 1);

    http.addHeader(
        "Content-Type",
        "application/json"
    );

    JsonDocument document;

    document["username"] = "service";
        //terminalConfig.username;

    document["password"] = "service";
        //terminalConfig.password;

    String requestBody;

    serializeJson(
        document,
        requestBody
    );

    int statusCode = http.POST(requestBody);

    if (statusCode < 0) {
        http.end();

        showError(
            "Servidor indisponivel",
            "Tentando novamente."
        );

        nextRequestAt = millis() + retryIntervalMs;

        return false;
    }

    if (statusCode == 200) {

        String setCookie = http.header("Set-Cookie");

        http.end();

        if (setCookie.length() == 0) {
            showError(
                "Cookie nao recebido.",
                "Resposta invalida."
            );

            nextRequestAt =
                millis() + retryIntervalMs;

            return false;
        }

        int semicolon = setCookie.indexOf(';');

        if (semicolon >= 0) {

            authenticationCookie = setCookie.substring(0, semicolon);

        } else {

            authenticationCookie = setCookie;

        }

        showMessage(
            "AUTENTICADO",
            "Sessao iniciada."
        );

        nextRequestAt = 0;

        return true;
    }

    if (statusCode == 401 || statusCode == 403) {

        http.end();

        showError(
            "Login recusado.",
            "Usuario ou senha invalidos."
        );

        nextRequestAt = millis() + retryIntervalMs;

        return false;
    }

    http.end();

    Serial.println("Inesperada na autenticação");
    showError(
        "Resposta inesperada.",
        "HTTP " + String(statusCode)
    );

    nextRequestAt = millis() + retryIntervalMs;

    return false;
}