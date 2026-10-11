#include "Display.h"

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>

#include <qrcode.h>

// ============================================================
// Hardware
// ============================================================

constexpr int8_t tftSclkPin = 18;
constexpr int8_t tftSdaPin = 23;
constexpr int8_t tftCsPin   = -1; // A placa não tem pino de Cs
constexpr int8_t tftDcPin   = 4;
constexpr int8_t tftResPin  = 25;
constexpr int8_t tftBlPin   = 32;

constexpr uint16_t screenWidth  = 240;
constexpr uint16_t screenHeight = 240;

constexpr uint8_t qrcodeVersion = 3;

// Cores RGB565
constexpr uint16_t COLOR_BG      = ST77XX_WHITE;
constexpr uint16_t COLOR_TEXT    = ST77XX_BLACK;
constexpr uint16_t COLOR_QR_BG   = ST77XX_WHITE;
constexpr uint16_t COLOR_QR_DARK = ST77XX_BLACK;

Adafruit_ST7789 display(
    tftCsPin,
    tftDcPin,
    tftResPin
);

// ============================================================
// Initialization
// ============================================================

void initializeDisplay() {
   
    SPI.begin(
        tftSclkPin,
        -1,           // MISO não é necessário para este display
        tftSdaPin,
        tftCsPin
    );

    pinMode(tftBlPin, OUTPUT);
    digitalWrite(tftBlPin, HIGH);

    // Inicialização do ST7789 de 240 x 240.
    display.init(screenWidth, screenHeight, SPI_MODE3);
    display.setRotation(2);

    display.fillScreen(COLOR_BG);

    display.setTextWrap(true);
    display.setTextSize(2);
    display.setTextColor(COLOR_TEXT, COLOR_BG);
}

// ============================================================
// Display
// ============================================================

void clearDisplay() {
    display.fillScreen(COLOR_BG);

    display.setTextColor(COLOR_TEXT, COLOR_BG);
    display.setTextSize(2);
    display.setTextWrap(true);
    display.setCursor(10, 10);
}

void showMessage(
    const String& line1,
    const String& line2,
    const String& line3,
    const String& line4,
    const String& line5
) {
    clearDisplay();

    display.println(" " + line1);

    if (line2.length() > 0) {
        display.println(" " + line2);
    }

    if (line3.length() > 0) {
        display.println(" " + line3);
    }

    if (line4.length() > 0) {
        display.println(" " + line4);
    }

    if (line5.length() > 0) {
        display.println(" " + line5);
    }
}

void showError(
    const String& message,
    const String& instruction
) {
    clearDisplay();

    display.println("ERRO");
    display.println();

    display.println(" " + message);

    if (instruction.length() > 0) {
        display.println();
        display.println(" " + instruction);
    }
}

void showQrCode(const String& code) {
    QRCode qrcode;

    uint8_t qrcodeData[
        qrcode_getBufferSize(qrcodeVersion)
    ];

    qrcode_initText(
        &qrcode,
        qrcodeData,
        qrcodeVersion,
        ECC_LOW,
        code.c_str()
    );

    const uint16_t moduleCount = qrcode.size;
    constexpr uint16_t quietZone = 4;

    // O QR Code deve caber na menor dimensão da tela.
    const uint16_t availableSize =
        min(screenWidth, screenHeight);

    const uint16_t scale =
        availableSize / (moduleCount + 2 * quietZone);

    if (scale == 0) {
        showError(
            "Codigo muito grande.",
            "Contate o administrador."
        );
        return;
    }

    const uint16_t qrSize = moduleCount * scale;
    const uint16_t quietZoneSize = quietZone * scale;
    const uint16_t totalSize = qrSize + 2 * quietZoneSize;

    // Centraliza o QR Code e sua margem.
    const int16_t startX =
        (screenWidth - totalSize) / 2;

    const int16_t startY =
        (screenHeight - totalSize) / 2;

    // Fundo branco, incluindo a quiet zone.
    display.fillScreen(COLOR_BG);

    // Desenha apenas os módulos escuros do QR.
    for (uint16_t y = 0; y < moduleCount; y++) {
        for (uint16_t x = 0; x < moduleCount; x++) {
            if (qrcode_getModule(&qrcode, x, y)) {
                display.fillRect(
                    startX + quietZoneSize + x * scale,
                    startY + quietZoneSize + y * scale,
                    scale,
                    scale,
                    COLOR_QR_DARK
                );
            }
        }
    }
}