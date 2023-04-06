#include "HZK_ILI9341.h"

TFTLCD::TFTLCD() : TFT_eSPI() {}
File  file24;
File  file16;

void TFTLCD::begin() {
  TFT_eSPI::begin();
  setRotation(3);
  fillScreen(0);
  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS Mount Failed");
    return;
  }
  HZK_Init();
}

void TFTLCD::sleep() {
  startWrite();
  writecommand(ILI9341_SLPIN); // Software reset
  endWrite();
}

void TFTLCD::wakeup() {
  startWrite();
  writecommand(ILI9341_SLPOUT);
  endWrite();
}

void TFTLCD::setBrightness(uint8_t brightness) {
  //ledcWrite(BLK_PWM_CHANNEL, brightness);
}

void TFTLCD::drawBitmap(int16_t x0, int16_t y0, int16_t w, int16_t h, const uint16_t *data) {
  bool swap = getSwapBytes();
  setSwapBytes(true);
  pushImage((int32_t)x0, (int32_t)y0, (uint32_t)w, (uint32_t)h, data);
  setSwapBytes(swap);
}

void TFTLCD::drawBitmap(int16_t x0, int16_t y0, int16_t w, int16_t h, uint16_t *data) {
  bool swap = getSwapBytes();  
  setSwapBytes(true);
  pushImage((int32_t)x0, (int32_t)y0, (uint32_t)w, (uint32_t)h, data);
  setSwapBytes(swap);
}

void TFTLCD::drawBitmap(int16_t x0, int16_t y0, int16_t w, int16_t h, const uint16_t *data, uint16_t transparent) {
  bool swap = getSwapBytes();
  setSwapBytes(true);
  pushImage((int32_t)x0, (int32_t)y0, (uint32_t)w, (uint32_t)h, data, transparent);
  setSwapBytes(swap);
}

void TFTLCD::drawBitmap(int16_t x0, int16_t y0, int16_t w, int16_t h, const uint8_t *data) {
  bool swap = getSwapBytes();
  setSwapBytes(true);
  pushImage((int32_t)x0, (int32_t)y0, (uint32_t)w, (uint32_t)h, (const uint16_t*)data);
  setSwapBytes(swap);
}

void TFTLCD::drawBitmap(int16_t x0, int16_t y0, int16_t w, int16_t h, uint8_t *data) {
  bool swap = getSwapBytes();
  setSwapBytes(true);
  pushImage((int32_t)x0, (int32_t)y0, (uint32_t)w, (uint32_t)h, (uint16_t*)data);
  setSwapBytes(swap);
}

void TFTLCD::progressBar(int x, int y, int w, int h, uint8_t val) {
  drawRect(x, y, w, h, 0x09F1);
  fillRect(x + 1, y + 1, w * (((float)val) / 100.0), h - 1, 0x09F1);
}

#include "utility/qrcode.h"
void TFTLCD::qrcode(const char *string, uint16_t x, uint16_t y, uint8_t width, uint8_t version) {

  // Create the QR code
  QRCode qrcode;
  uint8_t qrcodeData[qrcode_getBufferSize(version)];
  qrcode_initText(&qrcode, qrcodeData, version, 0, string);

  // Top quiet zone
  uint8_t thickness = width / qrcode.size;
  uint16_t lineLength = qrcode.size * thickness;
  uint8_t xOffset = x + (width-lineLength)/2;
  uint8_t yOffset = y + (width-lineLength)/2;
  fillRect(x, y, width, width, TFT_WHITE);

  for (uint8_t y = 0; y < qrcode.size; y++) {
    for (uint8_t x = 0; x < qrcode.size; x++) {
      uint8_t q = qrcode_getModule(&qrcode, x, y);
      if (q) fillRect(x * thickness + xOffset, y * thickness + yOffset, thickness, thickness, TFT_BLACK);
    }
  }
}

void TFTLCD::qrcode(const String &string, uint16_t x, uint16_t y, uint8_t width, uint8_t version) {
  int16_t len = string.length() + 2;
  char buffer[len];
  string.toCharArray(buffer, len);
  qrcode(buffer, x, y, width, version);
}

// These read 16- and 32-bit types from the SD card file.
// BMP data is stored little-endian, Arduino is little-endian too.
// May need to reverse subscript order if porting elsewhere.

uint16_t read16(fs::File &f) {
  uint16_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read(); // MSB
  return result;
}

uint32_t read32(fs::File &f) {
  uint32_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read();
  ((uint8_t *)&result)[2] = f.read();
  ((uint8_t *)&result)[3] = f.read(); // MSB
  return result;
}

// Bodmers BMP image rendering function
void TFTLCD::drawBmpFile(fs::FS &fs, const char *path, uint16_t x, uint16_t y) {
  if ((x >= width()) || (y >= height())) return;

  // Open requested file on SD card
  File bmpFS = fs.open(path, "r");

  if (!bmpFS) {
    Serial.print("File not found");
    return;
  }

  uint32_t seekOffset;
  uint16_t w, h, row, col;
  uint8_t  r, g, b;

  uint32_t startTime = millis();

  if (read16(bmpFS) == 0x4D42) {
    read32(bmpFS);
    read32(bmpFS);
    seekOffset = read32(bmpFS);
    read32(bmpFS);
    w = read32(bmpFS);
    h = read32(bmpFS);

    if ((read16(bmpFS) == 1) && (read16(bmpFS) == 24) && (read32(bmpFS) == 0)) {
      y += h - 1;

      setSwapBytes(true);
      bmpFS.seek(seekOffset);

      uint16_t padding = (4 - ((w * 3) & 3)) & 3;
      uint8_t lineBuffer[w * 3 + padding];

      for (row = 0; row < h; row++) {
        bmpFS.read(lineBuffer, sizeof(lineBuffer));
        uint8_t*  bptr = lineBuffer;
        uint16_t* tptr = (uint16_t*)lineBuffer;
        // Convert 24 to 16 bit colours
        for (col = 0; col < w; col++) {
          b = *bptr++;
          g = *bptr++;
          r = *bptr++;
          *tptr++ = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
        }

        // Push the pixel row to screen, pushImage will crop the line if needed
        // y is decremented as the BMP image is drawn bottom up
        pushImage(x, y--, w, 1, (uint16_t*)lineBuffer);
      }
      Serial.print("Loaded in "); Serial.print(millis() - startTime);
      Serial.println(" ms");
    }
    else Serial.println("BMP format not recognized.");
  }
  bmpFS.close();
}

// void TFTLCD::drawBmp(fs::FS &fs, const char *path, uint16_t x, uint16_t y) {
//   drawBmpFile(fs, path, x, y);
// }
/***************************************************
  This library is written to be compatible with Adafruit's ILI9341
  library and automatically detects the display type on ESP_WROVER_KITs
  Earlier WROVERs had ILI9341, while newer releases have ST7789V

  MIT license, all text above must be included in any redistribution
 ****************************************************/

/*
 * JPEG
 */

#include "rom/tjpgd.h"

#define jpgColor(c)                                                            \
  (((uint16_t)(((uint8_t *)(c))[0] & 0xF8) << 8) |                             \
   ((uint16_t)(((uint8_t *)(c))[1] & 0xFC) << 3) |                             \
   ((((uint8_t *)(c))[2] & 0xF8) >> 3))

#if ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_ERROR
const char *jd_errors[] = {"Succeeded",
                           "Interrupted by output function",
                           "Device error or wrong termination of input stream",
                           "Insufficient memory pool for the image",
                           "Insufficient stream input buffer",
                           "Parameter error",
                           "Data format error",
                           "Right format but not supported",
                           "Not supported JPEG standard"};
#endif

typedef struct {
  uint16_t x;
  uint16_t y;
  uint16_t maxWidth;
  uint16_t maxHeight;
  uint16_t offX;
  uint16_t offY;
  jpeg_div_t scale;
  const void *src;
  size_t len;
  size_t index;
  TFTLCD *tft;
  uint16_t outWidth;
  uint16_t outHeight;
} jpg_file_decoder_t;

static uint32_t jpgReadFile(JDEC *decoder, uint8_t *buf, uint32_t len) {
  jpg_file_decoder_t *jpeg = (jpg_file_decoder_t *)decoder->device;
  File *file = (File *)jpeg->src;
  if (buf) {
    return file->read(buf, len);
  } else {
    file->seek(len, SeekCur);
  }
  return len;
}

static uint32_t jpgRead(JDEC *decoder, uint8_t *buf, uint32_t len) {
  jpg_file_decoder_t *jpeg = (jpg_file_decoder_t *)decoder->device;
  if (buf) {
    memcpy(buf, (const uint8_t *)jpeg->src + jpeg->index, len);
  }
  jpeg->index += len;
  return len;
}

static uint32_t jpgWrite(JDEC *decoder, void *bitmap, JRECT *rect) {
  jpg_file_decoder_t *jpeg = (jpg_file_decoder_t *)decoder->device;
  uint16_t x = rect->left;
  uint16_t y = rect->top;
  uint16_t w = rect->right + 1 - x;
  uint16_t h = rect->bottom + 1 - y;
  uint16_t oL = 0, oR = 0;
  uint8_t *data = (uint8_t *)bitmap;

  if (rect->right < jpeg->offX) {
    return 1;
  }
  if (rect->left >= (jpeg->offX + jpeg->outWidth)) {
    return 1;
  }
  if (rect->bottom < jpeg->offY) {
    return 1;
  }
  if (rect->top >= (jpeg->offY + jpeg->outHeight)) {
    return 1;
  }
  if (rect->top < jpeg->offY) {
    uint16_t linesToSkip = jpeg->offY - rect->top;
    data += linesToSkip * w * 3;
    h -= linesToSkip;
    y += linesToSkip;
  }
  if (rect->bottom >= (jpeg->offY + jpeg->outHeight)) {
    uint16_t linesToSkip = (rect->bottom + 1) - (jpeg->offY + jpeg->outHeight);
    h -= linesToSkip;
  }
  if (rect->left < jpeg->offX) {
    oL = jpeg->offX - rect->left;
  }
  if (rect->right >= (jpeg->offX + jpeg->outWidth)) {
    oR = (rect->right + 1) - (jpeg->offX + jpeg->outWidth);
  }

  uint16_t pixBuf[32];
  uint8_t pixIndex = 0;
  uint16_t line;

  jpeg->tft->startWrite();
  // jpeg->tft->setAddrWindow(x - jpeg->offX + jpeg->x + oL, y - jpeg->offY +
  // jpeg->y, w - (oL + oR), h);
  jpeg->tft->setWindow(x - jpeg->offX + jpeg->x + oL,
                       y - jpeg->offY + jpeg->y,
                       x - jpeg->offX + jpeg->x + oL + w - (oL + oR) - 1,
                       y - jpeg->offY + jpeg->y + h - 1);

  while (h--) {
    data += 3 * oL;
    line = w - (oL + oR);
    while (line--) {
      pixBuf[pixIndex++] = jpgColor(data);
      data += 3;
      if (pixIndex == 32) {
        jpeg->tft->writePixels(pixBuf, 32);
        // SPI.writePixels((uint8_t *)pixBuf, 64);
        pixIndex = 0;
      }
    }
    data += 3 * oR;
  }
  if (pixIndex) {
    jpeg->tft->writePixels(pixBuf, pixIndex);
    // SPI.writePixels((uint8_t *)pixBuf, pixIndex * 2);
  }
  jpeg->tft->endWrite();
  return 1;
}

static bool jpgDecode(jpg_file_decoder_t *jpeg,
                      uint32_t (*reader)(JDEC *, uint8_t *, uint32_t)) {
  static uint8_t work[3100];
  JDEC decoder;

  JRESULT jres = jd_prepare(&decoder, reader, work, 3100, jpeg);
  if (jres != JDR_OK) {
    log_e("jd_prepare failed! %s", jd_errors[jres]);
    return false;
  }

  uint16_t jpgWidth = decoder.width / (1 << (uint8_t)(jpeg->scale));
  uint16_t jpgHeight = decoder.height / (1 << (uint8_t)(jpeg->scale));

  if (jpeg->offX >= jpgWidth || jpeg->offY >= jpgHeight) {
    log_e("Offset Outside of JPEG size");
    return false;
  }

  size_t jpgMaxWidth = jpgWidth - jpeg->offX;
  size_t jpgMaxHeight = jpgHeight - jpeg->offY;

  jpeg->outWidth =
      (jpgMaxWidth > jpeg->maxWidth) ? jpeg->maxWidth : jpgMaxWidth;
  jpeg->outHeight =
      (jpgMaxHeight > jpeg->maxHeight) ? jpeg->maxHeight : jpgMaxHeight;

  jres = jd_decomp(&decoder, jpgWrite, (uint8_t)jpeg->scale);
  if (jres != JDR_OK) {
    log_e("jd_decomp failed! %s", jd_errors[jres]);
    return false;
  }

  return true;
}

void TFTLCD::drawJpg(const uint8_t *jpg_data, size_t jpg_len, uint16_t x,
                        uint16_t y, uint16_t maxWidth, uint16_t maxHeight,
                        uint16_t offX, uint16_t offY, jpeg_div_t scale) {
  if ((x + maxWidth) > width() || (y + maxHeight) > height()) {
    log_e("Bad dimensions given");
    return;
  }

  jpg_file_decoder_t jpeg;

  if (!maxWidth) {
    maxWidth = width() - x;
  }
  if (!maxHeight) {
    maxHeight = height() - y;
  }

  jpeg.src = jpg_data;
  jpeg.len = jpg_len;
  jpeg.index = 0;
  jpeg.x = x;
  jpeg.y = y;
  jpeg.maxWidth = maxWidth;
  jpeg.maxHeight = maxHeight;
  jpeg.offX = offX;
  jpeg.offY = offY;
  jpeg.scale = scale;
  jpeg.tft = this;

  jpgDecode(&jpeg, jpgRead);
}

void TFTLCD::drawJpgFile(fs::FS &fs, const char *path, uint16_t x, uint16_t y,
                            uint16_t maxWidth, uint16_t maxHeight, uint16_t offX,
                            uint16_t offY, jpeg_div_t scale) {
  if ((x + maxWidth) > width() || (y + maxHeight) > height()) {
    log_e("Bad dimensions given");
    return;
  }

  File file = fs.open(path);
  if (!file) {
    log_e("Failed to open file for reading");
    return;
  }

  jpg_file_decoder_t jpeg;

  if (!maxWidth) {
    maxWidth = width() - x;
  }
  if (!maxHeight) {
    maxHeight = height() - y;
  }

  jpeg.src = &file;
  jpeg.len = file.size();
  jpeg.index = 0;
  jpeg.x = x;
  jpeg.y = y;
  jpeg.maxWidth = maxWidth;
  jpeg.maxHeight = maxHeight;
  jpeg.offX = offX;
  jpeg.offY = offY;
  jpeg.scale = scale;
  jpeg.tft = this;

  jpgDecode(&jpeg, jpgReadFile);

  file.close();
}


/*
 * PNG
 */

#include "utility/pngle.h"
#include <HTTPClient.h>

typedef struct _png_draw_params {
  uint16_t x;
  uint16_t y;
  uint16_t maxWidth;
  uint16_t maxHeight;
  uint16_t offX;
  uint16_t offY;
  double scale;
  uint8_t alphaThreshold;

  TFTLCD *tft;
} png_file_decoder_t;

static void pngle_draw_callback(pngle_t *pngle, uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint8_t rgba[4])
{
  png_file_decoder_t *p = (png_file_decoder_t *)pngle_get_user_data(pngle);
  uint16_t color = jpgColor(rgba); // XXX: It's PNG ;)

  if (x < p->offX || y < p->offY) return ;
  x -= p->offX;
  y -= p->offY;

  // An interlaced file with alpha channel causes disaster, so use 1 here for simplicity
  w = 1;
  h = 1;

  if (p->scale != 1.0) {
    x = (uint32_t)ceil(x * p->scale);
    y = (uint32_t)ceil(y * p->scale);
    w = (uint32_t)ceil(w * p->scale);
    h = (uint32_t)ceil(h * p->scale);
  }

  if (x >= p->maxWidth || y >= p->maxHeight) return ;
  if (x + w >= p->maxWidth) w = p->maxWidth - x;
  if (y + h >= p->maxHeight) h = p->maxHeight - y;

  x += p->x;
  y += p->y;

  if (rgba[3] >= p->alphaThreshold) {
    p->tft->fillRect(x, y, w, h, color);
  }
}

void TFTLCD::drawPngFile(fs::FS &fs, const char *path, uint16_t x, uint16_t y,
                            uint16_t maxWidth, uint16_t maxHeight, uint16_t offX,
                            uint16_t offY, double scale, uint8_t alphaThreshold)
{
  File file = fs.open(path);
  if (!file) {
    log_e("Failed to open file for reading");
    return ;
  }

  pngle_t *pngle = pngle_new();

  png_file_decoder_t png;

  if (!maxWidth) {
    maxWidth = width() - x;
  }
  if (!maxHeight) {
    maxHeight = height() - y;
  }

  png.x = x;
  png.y = y;
  png.maxWidth = maxWidth;
  png.maxHeight = maxHeight;
  png.offX = offX;
  png.offY = offY;
  png.scale = scale;
  png.alphaThreshold = alphaThreshold;
  png.tft = this;

  pngle_set_user_data(pngle, &png);
  pngle_set_draw_callback(pngle, pngle_draw_callback);

  // Feed data to pngle
  uint8_t buf[1024];
  int remain = 0;
  int len;
  while ((len = file.read(buf + remain, sizeof(buf) - remain)) > 0) {
    int fed = pngle_feed(pngle, buf, remain + len);
    if (fed < 0) {
      log_e("[pngle error] %s", pngle_error(pngle));
      break;
    }

    remain = remain + len - fed;
    if (remain > 0) memmove(buf, buf + fed, remain);
  }

  pngle_destroy(pngle);
  file.close();
}

void TFTLCD::drawPngUrl(const char *url, uint16_t x, uint16_t y,
                            uint16_t maxWidth, uint16_t maxHeight, uint16_t offX,
                            uint16_t offY, double scale, uint8_t alphaThreshold)
{
  HTTPClient http;

  if (WiFi.status() != WL_CONNECTED) {
    log_e("Not connected");
    return ;
  }

  http.begin(url);

  int httpCode = http.GET();
  if (httpCode != HTTP_CODE_OK) {
    log_e("HTTP ERROR: %d\n", httpCode);
    http.end();
    return ;
  }

  WiFiClient *stream = http.getStreamPtr();

  pngle_t *pngle = pngle_new();

  png_file_decoder_t png;

  if (!maxWidth) {
    maxWidth = width() - x;
  }
  if (!maxHeight) {
    maxHeight = height() - y;
  }


  png.x = x;
  png.y = y;
  png.maxWidth = maxWidth;
  png.maxHeight = maxHeight;
  png.offX = offX;
  png.offY = offY;
  png.scale = scale;
  png.alphaThreshold = alphaThreshold;
  png.tft = this;

  pngle_set_user_data(pngle, &png);
  pngle_set_draw_callback(pngle, pngle_draw_callback);

  // Feed data to pngle
  uint8_t buf[1024];
  int remain = 0;
  int len;
  while (http.connected()) {
    size_t size = stream->available();
    if (!size) { delay(1); continue; }

    if (size > sizeof(buf) - remain) size = sizeof(buf) - remain;
    if ((len = stream->readBytes(buf + remain, size)) > 0) {
      int fed = pngle_feed(pngle, buf, remain + len);
      if (fed < 0) {
        log_e("[pngle error] %s", pngle_error(pngle));
        break;
      }

      remain = remain + len - fed;
      if (remain > 0) memmove(buf, buf + fed, remain);
    }
  }

  pngle_destroy(pngle);
  http.end();
}

//“‘œ¬ «¡ı»´¿ÒÃÌº”µƒ≤ø∑÷ 

//ll
bool TFTLCD::loadhzlib(String hz, uint8_t *libdata , uint8_t dot) {
  uint16_t offset = 0, unicode;
  if (isChinese(hz.c_str())) {
    unicode = utf8_to_unicode(hz.c_str());
    offset = unicode_to_offset(unicode);
  } else if (isGBK(hz.c_str())) {
    offset = gbk_to_offset(hz.c_str());
  }
  else
    return false;
  bool res = readHzlib(offset, libdata, dot);   //Êó∂Èó¥30ÔΩû70ÊØ´ÁßíÔºåÊÑÅ‰∫∫ÔºÅÔºÅÔºÅ
  return res;
}
uint16_t TFTLCD::utf8_to_unicode(const char *input) {
  uint16_t i = 0;
  uint8_t c1 = input[i++];
  uint8_t c2 = input[i++];
  uint8_t c3 = input[i++];
  uint16_t u = ((c1 & 0x0F) << 12) | ((c2 & 0x3F) << 6) | (c3 & 0x3F);
  return u;
}
uint16_t TFTLCD::gbk_to_offset(const char *input) {
  return (input[0] - 0xA1) * 94 + (input[1] - 0xA1);
}
bool TFTLCD::isChinese(const char* bytes) {
  if (bytes[0] < 0xE4 || bytes[0] > 0xEF) {
    return false;
  }
  if (bytes[1] < 0x80 || bytes[1] > 0xBF) {
    return false;
  }
  if (bytes[2] < 0x80 || bytes[2] > 0xBF) {
    return false;
  }
  return true;
}

bool TFTLCD::isASCII(const char* bytes) {
  if (bytes[0] < 0x20 || bytes[0] > 0x7F) {
    return false;
  }
  return true;
}
bool TFTLCD::isGBK(const char* bytes) {
  if (bytes[0] < 0x81 || bytes[0] > 0xFE) {
    return false;
  }
  if (bytes[1] < 0x40 || bytes[1] > 0xFE) {
    return false;
  }
  if (bytes[2] && (bytes[2] < 0x40 || bytes[2] > 0xFE)) {
    return false;
  }
  return true;
}

void TFTLCD::HZK_Init() {
  File file = SPIFFS.open("/U2offset.bin");
  if (!file || file.isDirectory()) {
    Serial.println("- failed to open U2offset.bin");
    return;
  }
  uint8_t tmpuni[2], tmpoffset[2];
  for (int i = 0; i < HZNUMS; i++) {
    file.read(tmpuni, 2);
    preload[i][0] = tmpuni[1] * 256 + tmpuni[0];
    file.read(tmpoffset, 2);
    preload[i][1] = tmpoffset[1] * 256 + tmpoffset[0];;
  }
  file.close();
  file24 = SPIFFS.open("/HZK24M");
  file16 = SPIFFS.open("/HZK16M");
  return;
}
uint16_t TFTLCD::unicode_to_offset(uint16_t uni) {
  uint16_t start = 0, end = HZNUMS - 1, mid;
  while (start <= end) {
    mid = (start + end) / 2;
    if (preload[mid][0] == uni) {
      return preload[mid][1];
      break;
    } else if (preload[mid][0] > uni) {
      end = mid - 1;
    } else {
      start = mid + 1;
    }
  }
  return 0;
}
bool TFTLCD::readHzlib(uint16_t offset, uint8_t *libdata, uint8_t dot) {
  switch (dot) {
    case 24:
      memset(libdata, 0, 72);
      file24.seek(offset * 72);
      file24.read(libdata, 72);
      for (int i = 0; i < 72; i++)
        libdata[i] ^= USERID[i % 8];
      break;
    case 16:
      memset(libdata, 0, 32);
      file16.seek(offset * 32);
      file16.read(libdata, 32);
      for (int i = 0; i < 32; i++)
        libdata[i] ^= USERID[i % 8];
      break;
  }
  return true;
}

void TFTLCD::printHZ16(uint8_t *pdata, int x, int y, uint16_t c) {
  int i, j, k;
  for (i = 0; i < 16; i++)
  {
    for (j = 0; j < 2; j++)
    {
      for (k = 0; k < 8; k++)
      {
        if (bitRead(pdata[i * 2 + j], 7 - k)) drawPixel(x + (j * 8 + k), y + i, c);
      }
    }
  }
}
void TFTLCD::printHZ24(uint8_t *pdata, int x, int y, uint16_t c) {
  int i, j, k;
  for (i = 0; i < 24; i++)
  {
    for (j = 0; j < 3; j++)
    {
      for (k = 0; k < 8; k++)
      {
        if (bitRead(pdata[i * 3 + j], 7 - k)) drawPixel(x + i, y + (j * 8 + k), c);
      }
    }
  }
}
void TFTLCD::printStr24(String dstr, int x, int y, uint16_t c) {
  uint16_t xnow = x, ynow = y, ccount = 0;
  uint8_t libdata[72];
  setTextColor(c);
  while (ccount < dstr.length())
    if (isChinese(dstr.c_str() + ccount)) {
      loadhzlib(dstr.substring(ccount, ccount + 3), libdata , 24);
      printHZ24(libdata, xnow , ynow, c);
      xnow += 24;
      ccount += 3;
    }
    else if (isASCII(dstr.c_str() + ccount)) {
      drawString(dstr.substring(ccount, ccount + 1), xnow, ynow, 4);
      xnow += textWidth(dstr.substring(ccount, ccount + 1), 4);
      ccount += 1;
    }
  return;
}

void TFTLCD::printStr16(String dstr, int x, int y, uint16_t c) {
  uint16_t xnow = x, ynow = y, ccount = 0;
  uint8_t libdata[32];
  setTextColor(c);
  while (ccount < dstr.length())
    if (isChinese(dstr.c_str() + ccount)) {
      loadhzlib(dstr.substring(ccount, ccount + 3), libdata , 16);
      printHZ16(libdata, xnow , ynow, c);
      xnow += 16;
      ccount += 3;
    }
    else if (isASCII(dstr.c_str() + ccount)) {
      drawString(dstr.substring(ccount, ccount + 1), xnow, ynow, 2);
      xnow += textWidth(dstr.substring(ccount, ccount + 1), 2);
      ccount += 1;
    }
  return;
}
uint16_t TFTLCD::width16(String msg) {
  uint16_t xnow = 0, ccount = 0;
  while (ccount < msg.length()) {
    if (isChinese(msg.c_str() + ccount)) {
      xnow += 16;
      ccount += 3;
    }
    else if (isASCII(msg.c_str() + ccount)) {
      xnow += textWidth(msg.substring(ccount, ccount + 1), 2);
      ccount += 1;
    }
  }
  return xnow;
}
uint16_t TFTLCD::width24(String msg) {
  uint16_t xnow = 0, ccount = 0;
  while (ccount < msg.length()) {
    if (isChinese(msg.c_str() + ccount)) {
      xnow += 24;
      ccount += 3;
    }
    else if (isASCII(msg.c_str() + ccount)) {
      xnow += textWidth(msg.substring(ccount, ccount + 1), 4);
      ccount += 1;
    }
  }
  return xnow;
}
void TFTLCD::drawBox(int boxtype, int cnen) {
  uint16_t shadow1 = color565(230, 190, 132);
  fillRect(32, 56, 256, 128, TFT_BLACK);
  fillRect(28, 52, 256, 128, color565(218, 136, 75));
  fillRect(36, 92 , 240, 80, color565(133, 100, 87));
  drawFastHLine(28, 52, 256, shadow1);
  drawFastVLine(28, 52, 128, shadow1);
  drawFastHLine(36, 172, 240, shadow1);
  drawFastVLine(276, 92, 80, shadow1);
  drawFastHLine(36, 92, 240, TFT_BLACK);
  drawFastVLine(36, 92, 80, TFT_BLACK);

  switch (boxtype) {
    case 1:
      printStr24("ÊèêÁ§∫", 128, 60, TFT_YELLOW);
      break;
    case 2:
      printStr24("‰ø°ÊÅØ", 128, 60, TFT_YELLOW);
      break;
    case 3:
      printStr24("Ë≠¶Âëä", 128, 60, TFT_RED);
      break;
    case 4:
      printStr24("ÈîôËØØ", 128, 60, TFT_RED);
      break;
    case 5:
      setTextColor(TFT_YELLOW, color565(218, 136, 75));
      drawString("Tips", 128, 60, 4);
      break;
    case 6:
      setTextColor(TFT_YELLOW, color565(218, 136, 75));
     drawString("Info", 128, 60, 4);
      break;
    case 7:
      setTextColor(TFT_RED, color565(218, 136, 75));
      drawString("Alert", 128, 60, 4);
      break;
    case 8:
      setTextColor(TFT_RED, color565(218, 136, 75));
      drawString("Error", 128, 60, 4);
      break;
  }
  return;
}
void TFTLCD::msgbox16(String msg, int boxtype, int waittime) {
  int x1, x2, x3, y1, y2, y3;
  uint16_t textClor = foreColor; 
  uint16_t split = msg.indexOf('/');
  String m1, m2, m3;
  if (split > 0) {
    m1 = msg.substring(0, split);
    msg = msg.substring(split + 1);
    split = msg.indexOf('/');
    if (split > 0) {
      m2 = msg.substring(0, split);
      m3 = msg.substring(split + 1);
    }
    else {
      m2 = msg;
      m3 = "";
    }
  }
  else {
    m1 = msg;
    m2 = "";
    m3 = "";
  }
  drawBox(boxtype);
  if (m2.length() != 0 && m3.length() != 0) {
    x1 = 36 + (240 - width16(m1)) / 2;
    y1 = 98;
    x2 = 36 + (240 - width16(m2)) / 2;
    y2 = 120;
    x3 = 36 + (240 - width16(m3)) / 2;
    y3 = 144;
    printStr16(m1, x1, y1, textClor);
    printStr16(m2, x2, y2, textClor);
    printStr16(m3, x3, y3, textClor);
  }
  else if (m2.length() != 0) {
    x1 = 36 + (240 - width16(m1)) / 2;
    y1 = 108;
    x2 = 36 + (240 - width16(m2)) / 2;
    y2 = 136;
    printStr16(m1, x1, y1, textClor);
    printStr16(m2, x2, y2, textClor);
  }
  else {
    x1 = 36 + (240 - width16(m1)) / 2;
    y1 = 120;
    printStr16(m1, x1, y1, textClor);
  }
  if (waittime != 0) {
    int tmp = millis();
    while (millis() - tmp < waittime) ;
  }
  return;
}
void TFTLCD::msgbox24(String msg, int boxtype, int waittime) {
  int x1, x2, y1, y2;
  uint16_t textClor = foreColor;
  uint16_t split = msg.indexOf('/');
  String m1, m2;
  if (split > 0) {
    m1 = msg.substring(0, split);
    m2 = msg.substring(split + 1);
  }
  else {
    m1 = msg;
    m2 = "";
  }

  drawBox(boxtype);
  if (m2.length() != 0) {
    x1 = 36 + (240 - width24(m1)) / 2;
    y1 = 104;
    x2 = 36 + (240 - width24(m2)) / 2;
    y2 = 136;
    printStr24(m1, x1, y1, textClor);
    printStr24(m2, x2, y2, textClor);
  }
  else {
    x1 = 36 + (240 - width24(m1)) / 2;
    y1 = 120;
    printStr24(m1, x1, y1, textClor);
  }
  if (waittime != 0) {
    int tmp = millis();
    while (millis() - tmp < waittime) ;
  }
  return;
}

void TFTLCD::setBoxTextColor(uint16_t fColor){
	foreColor = fColor;
}
