#include "ConfigurationWeb.h"

#include <WebServer.h>

#include "../config/Configuration.h"
#include "../display/Display.h"

// ============================================================
// Configuration HTTP
// ============================================================

WebServer webServer(80);

// ============================================================
// Configuration HTML
// ============================================================

String htmlEscape(
    const String& value
) {
    String result = value;

    result.replace("&", "&amp;");
    result.replace("<", "&lt;");
    result.replace(">", "&gt;");
    result.replace("\"", "&quot;");

    return result;
}

String buildConfigurationPage() {
    String html;

    html.reserve(5000);

    html += R"rawliteral(
<!DOCTYPE html>
<html lang="pt-BR">
<head>
<meta charset="UTF-8">
<meta name="viewport"
      content="width=device-width,initial-scale=1">

<title>Configurar terminal</title>

<style>

* {
    box-sizing: border-box;
}

body {
    margin: 0;
    padding: 20px;
    background: #f2f2f2;
    font-family: Arial, sans-serif;
}

.container {
    max-width: 520px;
    margin: auto;
    background: white;
    padding: 24px;
    border-radius: 12px;
    box-shadow: 0 2px 10px #bbb;
}

h1 {
    margin-top: 0;
}

label {
    display: block;
    margin-top: 18px;
    font-weight: bold;
}

input {
    width: 100%;
    padding: 12px;
    margin-top: 7px;
    font-size: 16px;
    border: 1px solid #bbb;
    border-radius: 6px;
}

button {
    width: 100%;
    margin-top: 24px;
    padding: 14px;
    border: 0;
    border-radius: 6px;
    background: #1976d2;
    color: white;
    font-size: 17px;
}

.info {
    padding: 12px;
    margin-top: 8px;
    background: #eeeeee;
    border-radius: 6px;
}

.help {
    color: #666;
    font-size: 14px;
}

</style>
</head>

<body>

<div class="container">

<h1>Configurar terminal</h1>

<p>
Configure a rede e o servidor que este terminal deve utilizar.
</p>

<label>Identificador do terminal</label>

<div class="info">
)rawliteral";

    html += htmlEscape(
        terminalConfig.terminalId
    );

    html += R"rawliteral(
</div>

<p class="help">
Este identificador é criado automaticamente e não pode ser alterado.
</p>

<form method="POST" action="/save">

<label>Rede Wi-Fi</label>

<input
    type="text"
    name="wifiSsid"
    placeholder="Nome da rede Wi-Fi"
    value=")rawliteral";

    html += htmlEscape(
        terminalConfig.wifiSsid
    );

    html += R"rawliteral("
    required>

<label>Senha do Wi-Fi</label>

<input
    type="password"
    name="wifiPassword"
    placeholder="Senha da rede Wi-Fi"
    value=")rawliteral";

    html += htmlEscape(
        terminalConfig.wifiPassword
    );

    html += R"rawliteral("
    required>

<label>Endereço do servidor</label>

<input
    type="text"
    name="serverAddress"
    placeholder="192.168.1.10:8080"
    value=")rawliteral";

    html += htmlEscape(
        terminalConfig.serverAddress
    );

    html += R"rawliteral("
    required>

<p class="help">
Pode utilizar um endereço IP ou hostname.
Exemplo: 192.168.1.10:8080
ou servidor.local:8080
</p>

<label>Nome do terminal</label>

<input
    type="text"
    name="friendlyName"
    placeholder="Ex.: Recepção"
    value=")rawliteral";

    html += htmlEscape(
        terminalConfig.friendlyName
    );

    html += R"rawliteral("
    required>

<p class="help">
Este é o nome usado para identificar o terminal no sistema.
</p>

<button type="submit">
Salvar configuração
</button>

</form>

</div>

</body>
</html>
)rawliteral";

    return html;
}

// ============================================================
// Configuration HTTP handlers
// ============================================================

void handleConfigurationPage() {
    webServer.send(
        200,
        "text/html; charset=utf-8",
        buildConfigurationPage()
    );
}

String getFormValue(
    const char* name
) {
    if (!webServer.hasArg(name)) {
        return "";
    }

    String value =
        webServer.arg(name);

    value.trim();

    return value;
}

void handleSaveConfiguration() {
    String wifiSsid =
        getFormValue("wifiSsid");

    String wifiPassword =
        getFormValue("wifiPassword");

    String serverAddress =
        getFormValue("serverAddress");

    String friendlyName =
        getFormValue("friendlyName");

    if (
        wifiSsid.isEmpty() ||
        wifiPassword.isEmpty() ||
        serverAddress.isEmpty() ||
        friendlyName.isEmpty()
    ) {
        webServer.send(
            400,
            "text/html; charset=utf-8",
            "<h2>Preencha todos os campos.</h2>"
        );

        showError(
            "Campos incompletos.",
            "Preencha a configuracao."
        );

        return;
    }

    terminalConfig.wifiSsid =
        wifiSsid;

    terminalConfig.wifiPassword =
        wifiPassword;

    terminalConfig.serverAddress =
        serverAddress;

    terminalConfig.friendlyName =
        friendlyName;

    saveConfiguration();

    webServer.send(
        200,
        "text/html; charset=utf-8",
        R"rawliteral(
<!DOCTYPE html>
<html lang="pt-BR">
<head>
<meta charset="UTF-8">
<meta name="viewport"
content="width=device-width,initial-scale=1">
<title>Configuração salva</title>
</head>

<body style="font-family:Arial;padding:30px">

<h2>Configuração salva!</h2>

<p>
Desconecte-se da rede de configuração.
</p>

<p>
Depois mude o switch para
<b>PRODUCTION</b>.
</p>

<p>
O terminal irá se conectar automaticamente
ao Wi-Fi configurado.
</p>

</body>
</html>
)rawliteral"
    );

    showMessage(
        "CONFIGURACAO SALVA",
        "Desconecte o celular",
        "e mude o switch",
        "para PRODUCTION"
    );
}

void handleConfigurationNotFound() {
    webServer.sendHeader(
        "Location",
        "http://192.168.4.1/",
        true
    );

    webServer.send(
        302,
        "text/plain",
        ""
    );
}

void startConfigurationServer() {
    webServer.on(
        "/",
        HTTP_GET,
        handleConfigurationPage
    );

    webServer.on(
        "/save",
        HTTP_POST,
        handleSaveConfiguration
    );

    webServer.on(
        "/generate_204",
        HTTP_GET,
        handleConfigurationPage
    );

    webServer.on(
        "/hotspot-detect.html",
        HTTP_GET,
        handleConfigurationPage
    );

    webServer.on(
        "/connecttest.txt",
        HTTP_GET,
        handleConfigurationPage
    );

    webServer.onNotFound(
        handleConfigurationNotFound
    );

    webServer.begin();
}

void handleConfigurationServer() {
    webServer.handleClient();
}