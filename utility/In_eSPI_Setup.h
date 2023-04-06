#define ILI9341_DRIVER
#include "ILI9341_Defines.h"
#define  TFT_DRIVER 0x9341
#define TFT_SDA_READ      // This option is for ESP32 ONLY, tested with ST7789 display only
//用户根据自己的硬件修改以下管脚定义 和字库 
#define TFT_MISO -1
#define TFT_MOSI 33
#define TFT_SCLK 32
#define TFT_CS   13  // Chip select control pin
#define TFT_DC   4  // Data Command control pin
#define TFT_RST  -1  // Reset pin (could connect to Arduino RESET pin)
#define TFT_BL   -1  // LED back-light (required for M5Stack)

#define LOAD_FONT2  // Font 2. Small 16 pixel high font, needs ~3534 bytes in FLASH, 96 characters
#define LOAD_FONT4  // Font 4. Medium 26 pixel high font, needs ~5848 bytes in FLASH, 96 characters
#define LOAD_FONT6  // Font 6. Large 48 pixel font, needs ~2666 bytes in FLASH, only characters 1234567890:-.apm
#define LOAD_FONT7  // Font 7. 7 segment 48 pixel font, needs ~2438 bytes in FLASH, only characters 1234567890:-.
#define LOAD_FONT8  // Font 8. Large 75 pixel font needs ~3256 bytes in FLASH, only characters 1234567890:-.
#define SPI_FREQUENCY  40000000 // Maximum to use SPIFFS

#define SPI_READ_FREQUENCY  16000000

