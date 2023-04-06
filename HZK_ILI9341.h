#ifndef _TFTLCD_ILI9341_H_
  #define _TFTLCD_ILI9341_H_

  #include <Arduino.h>
  #include <FS.h>
  #include <SPI.h>
  #include <SPIFFS.h>
  #include "utility/In_eSPI.h"
  #include "utility/Sprite.h"
  #define HZNUMS 8042

  typedef enum {
    JPEG_DIV_NONE,
    JPEG_DIV_2,
    JPEG_DIV_4,
    JPEG_DIV_8,
    JPEG_DIV_MAX
  } jpeg_div_t;

  class TFTLCD : public TFT_eSPI {
    public:
      TFTLCD();
      void begin();
      void sleep();
      void wakeup();
      void setBrightness(uint8_t brightness);
      void clearDisplay(uint32_t color=ILI9341_BLACK) { fillScreen(color); }
      void clear(uint32_t color=ILI9341_BLACK) { fillScreen(color); }
      void display() {}

      inline void startWrite(void){
        #if defined (SPI_HAS_TRANSACTION) && defined (SUPPORT_TRANSACTIONS) && !defined(ESP32_PARALLEL)
          if (locked) {
            locked = false; SPI.beginTransaction(SPISettings(SPI_FREQUENCY, MSBFIRST, SPI_MODE0));
          }
        #endif
        CS_L;
      }
      inline void endWrite(void){
        #if defined (SPI_HAS_TRANSACTION) && defined (SUPPORT_TRANSACTIONS) && !defined(ESP32_PARALLEL)
          if(!inTransaction) {
            if (!locked) {
              locked = true; 
              SPI.endTransaction();
            }
          }
        #endif
        CS_H;
      }
      inline void writePixel(uint16_t color) {
        SPI.write16(color);
      }
      inline void writePixels(uint16_t * colors, uint32_t len){
        SPI.writePixels((uint8_t*)colors , len * 2);
      }
      void progressBar(int x, int y, int w, int h, uint8_t val);

      #define setFont setFreeFont

      void qrcode(const char *string, uint16_t x = 50, uint16_t y = 10, uint8_t width = 220, uint8_t version = 6);
      void qrcode(const String &string, uint16_t x = 50, uint16_t y = 10, uint8_t width = 220, uint8_t version = 6);

      void drawBmp(fs::FS &fs, const char *path, uint16_t x, uint16_t y);
      void drawBmpFile(fs::FS &fs, const char *path, uint16_t x, uint16_t y);

      void drawBitmap(int16_t x0, int16_t y0, int16_t w, int16_t h, const uint16_t *data);
      void drawBitmap(int16_t x0, int16_t y0, int16_t w, int16_t h, const uint8_t *data);
      void drawBitmap(int16_t x0, int16_t y0, int16_t w, int16_t h, uint16_t *data);
      void drawBitmap(int16_t x0, int16_t y0, int16_t w, int16_t h, uint8_t *data);
      void drawBitmap(int16_t x0, int16_t y0, int16_t w, int16_t h, const uint16_t *data, uint16_t transparent);

      void drawJpg(const uint8_t *jpg_data, size_t jpg_len, uint16_t x = 0,
                  uint16_t y = 0, uint16_t maxWidth = 0, uint16_t maxHeight = 0,
                  uint16_t offX = 0, uint16_t offY = 0,
                  jpeg_div_t scale = JPEG_DIV_NONE);

      void drawJpg(fs::FS &fs, const char *path, uint16_t x = 0, uint16_t y = 0,
                    uint16_t maxWidth = 0, uint16_t maxHeight = 0,
                    uint16_t offX = 0, uint16_t offY = 0,
                    jpeg_div_t scale = JPEG_DIV_NONE);

      void drawJpgFile(fs::FS &fs, const char *path, uint16_t x = 0, uint16_t y = 0,
                    uint16_t maxWidth = 0, uint16_t maxHeight = 0,
                    uint16_t offX = 0, uint16_t offY = 0,
                    jpeg_div_t scale = JPEG_DIV_NONE);

      void drawPngFile(fs::FS &fs, const char *path, uint16_t x = 0, uint16_t y = 0,
                    uint16_t maxWidth = 0, uint16_t maxHeight = 0,
                    uint16_t offX = 0, uint16_t offY = 0,
                    double scale = 1.0, uint8_t alphaThreshold = 127);

      void drawPngUrl(const char *url, uint16_t x = 0, uint16_t y = 0,
                    uint16_t maxWidth = 0, uint16_t maxHeight = 0,
                    uint16_t offX = 0, uint16_t offY = 0,
                    double scale = 1.0, uint8_t alphaThreshold = 127);
    ////��������ȫ�����ӵĲ��� 
	void HZK_Init();                
    void printStr16(String dstr, int x, int y, uint16_t c);   //中英混合显示 支持UTF8编码
	void printStr24(String dstr, int x, int y, uint16_t c);   //中英混合显示 支持UTF8编码
	void msgbox16(String msg, int boxtype = 1, int waittime = 1);//中英混合显示 支持UTF8编码
	void msgbox24(String msg, int boxtype = 1, int waittime = 1);//中英混合显示 支持UTF8编码                
	void setBoxTextColor(uint16_t fColor = TFT_WHITE);
    private:
   	unsigned char USERID[8] = {0xBC, 0xD1, 0xCC, 0xA9, 0xC8, 0xAB, 0xC0, 0xF1};
	uint16_t preload[HZNUMS][2];
    uint16_t utf8_to_unicode(const char *input);
	uint16_t unicode_to_offset(uint16_t uni);
	uint16_t gbk_to_offset(const char *input);
	uint16_t foreColor = TFT_WHITE;
	bool readHzlib(uint16_t offset, uint8_t *libdata, uint8_t dot);
	bool loadhzlib(String hz, uint8_t *libdata , uint8_t dot);
	bool isChinese(const char *input);
	bool isGBK(const char *input);
	bool isASCII(const char* bytes);
	void drawBox(int boxtype = 1, int cnen = 0);   //画对话框 cnen = 0 中文 cnen = 1 英文
	void printHZ16(uint8_t *pdata, int x, int y, uint16_t c);
	void printHZ24(uint8_t *pdata, int x, int y, uint16_t c);
	uint16_t width16(String msg);
	uint16_t width24(String msg);	
    //ll
  	
    	
  };
#endif
