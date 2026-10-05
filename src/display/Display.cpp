#include "Display.h"

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <qrcode.h>

// ============================================================
// Hardware
// ============================================================

constexpr uint8_t oledSdaPin = 21;
constexpr uint8_t oledSclPin = 22;

constexpr uint8_t screenWidth = 128;
constexpr uint8_t screenHeight = 64;

constexpr uint8_t oledAddress = 0x3C;

constexpr uint8_t qrcodeVersion = 3;

Adafruit_SSD1306 display(
    screenWidth,
    screenHeight,
    &Wire,
    -1
);

// ============================================================
// Initialization
// ============================================================

void initializeDisplay() {
    Wire.begin(
        oledSdaPin,
        oledSclPin
    );

    if (
        !display.begin(
            SSD1306_SWITCHCAPVCC,
            oledAddress
        )
    ) {
        while (true) {
            delay(1000);
        }
    }

    display.ssd1306_command(SSD1306_SETCONTRAST);
    display.ssd1306_command(0x10);

    display.ssd1306_command(SSD1306_SETDISPLAYCLOCKDIV);
    display.ssd1306_command(0xF0);


}

// ============================================================
// Display
// ============================================================

void clearDisplay() {
    display.clearDisplay();

    display.setTextColor(
        SSD1306_WHITE
    );

    display.setTextSize(1);

    display.setCursor(0, 0);
}

void showMessage(
    const String& line1,
    const String& line2,
    const String& line3,
    const String& line4,
    const String& line5
) {
    clearDisplay();

    display.println(line1);

    if (line2.length() > 0) {
        display.println(line2);
    }

    if (line3.length() > 0) {
        display.println(line3);
    }

    if (line4.length() > 0) {
        display.println(line4);
    }

    if (line5.length() > 0) {
        display.println(line5);
    }

    display.display();
}

void showError(
    const String& message,
    const String& instruction
) {
    clearDisplay();

    display.println("ERRO");
    display.println();

    display.println(message);

    if (instruction.length() > 0) {
        display.println();
        display.println(instruction);
    }

    display.display();
}

void showQrCode(
    const String& code
) {
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

    uint8_t moduleCount = qrcode.size;

    constexpr uint8_t quietZone = 1;

    uint8_t scale = min(
        screenWidth / (moduleCount + 2 * quietZone),
        screenHeight / (moduleCount + 2 * quietZone)
    );

    if (scale == 0) {
        showError(
            "Codigo muito grande.",
            "Contate o administrador."
        );

        return;
    }

    uint16_t qrSize =
        moduleCount * scale;

    uint16_t quietZoneSize =
        quietZone * scale;

    uint16_t totalSize =
        qrSize + 2 * quietZoneSize;

    uint16_t offsetX =
        (screenWidth - totalSize) / 2
        + quietZoneSize;

    uint16_t offsetY =
        (screenHeight - totalSize) / 2
        + quietZoneSize;

    display.clearDisplay();

    // Fundo branco + quiet zone
    display.fillRect(
        offsetX - quietZoneSize,
        offsetY - quietZoneSize,
        totalSize,
        totalSize,
        SSD1306_WHITE
    );

    // QR Code preto
    for (
        uint8_t y = 0;
        y < moduleCount;
        y++
    ) {
        for (
            uint8_t x = 0;
            x < moduleCount;
            x++
        ) {
            if (
                qrcode_getModule(
                    &qrcode,
                    x,
                    y
                )
            ) {
                display.fillRect(
                    offsetX + x * scale,
                    offsetY + y * scale,
                    scale,
                    scale,
                    SSD1306_BLACK
                );
            }
        }
    }

    display.display();
}
