// Use board "ESP32S3 Dev Module".

#include "Display_ST7789.h"
#include "MjpegClass.h"

#include <Arduino.h>
#include <SD.h>
#include <esp_heap_caps.h>

constexpr char MJPEG_FOLDER[] = "/mjpeg";
constexpr size_t MAX_FILES = 20;
constexpr uint8_t DISPLAY_BRIGHTNESS_PERCENT = 100;
constexpr size_t MJPEG_BUFFER_SIZE = 100 * 1024;

String mjpegFileList[MAX_FILES];
uint32_t mjpegFileSizes[MAX_FILES] = {0};
int mjpegCount = 0;
int currentMjpegIndex = 0;

MjpegClass mjpeg;
uint8_t *mjpeg_buf = nullptr;
uint16_t *jpegClipBuffer = nullptr;
size_t jpegClipBufferPixels = 0;

int total_frames = 0;
unsigned long total_read_video = 0;
unsigned long total_decode_video = 0;
unsigned long total_show_video = 0;
unsigned long start_ms = 0;
unsigned long curr_ms = 0;

volatile bool skipRequested = false;
volatile uint32_t lastPress = 0;

void *allocateVideoBuffer(size_t size)
{
  void *buffer = heap_caps_malloc(size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
  return buffer ? buffer : malloc(size);
}

void IRAM_ATTR onButtonPress()
{
  const uint32_t now = millis();
  if (now - lastPress > 300)
  {
    skipRequested = true;
    lastPress = now;
  }
}

void haltWithMessage(const char *message)
{
  Serial.println(message);
  while (true)
  {
    delay(1000);
  }
}

String basenameFromPath(const String &path)
{
  const int slash = path.lastIndexOf('/');
  return (slash >= 0) ? path.substring(slash + 1) : path;
}

void loadMjpegFilesList()
{
  File mjpegDir = SD.open(MJPEG_FOLDER);
  if (!mjpegDir || !mjpegDir.isDirectory())
  {
    haltWithMessage("Failed to open /mjpeg folder");
  }

  mjpegCount = 0;
  while (true)
  {
    File file = mjpegDir.openNextFile();
    if (!file)
    {
      break;
    }

    if (!file.isDirectory())
    {
      String name = basenameFromPath(String(file.name()));
      if (name.endsWith(".mjpeg") || name.endsWith(".MJPEG"))
      {
        mjpegFileList[mjpegCount] = name;
        mjpegFileSizes[mjpegCount] = file.size();
        ++mjpegCount;
        if (mjpegCount >= static_cast<int>(MAX_FILES))
        {
          file.close();
          break;
        }
      }
    }

    file.close();
  }

  mjpegDir.close();

  Serial.printf("%d mjpeg files read\n", mjpegCount);
  for (int i = 0; i < mjpegCount; ++i)
  {
    Serial.printf("File %d: %s, Size: %lu bytes\n", i, mjpegFileList[i].c_str(), mjpegFileSizes[i]);
  }
}

int jpegDrawCallback(JPEGDRAW *pDraw)
{
  const unsigned long started = millis();

  const int16_t x = pDraw->x;
  const int16_t y = pDraw->y;
  const int16_t w = pDraw->iWidth;
  const int16_t h = pDraw->iHeight;

  const int16_t clippedX = max<int16_t>(0, x);
  const int16_t clippedY = max<int16_t>(0, y);
  const int16_t clippedRight = min<int16_t>(LCD_WIDTH, x + w);
  const int16_t clippedBottom = min<int16_t>(LCD_HEIGHT, y + h);
  const int16_t clippedWidth = clippedRight - clippedX;
  const int16_t clippedHeight = clippedBottom - clippedY;

  if (clippedWidth <= 0 || clippedHeight <= 0)
  {
    return 1;
  }

  const size_t neededPixels = static_cast<size_t>(clippedWidth) * clippedHeight;
  if (neededPixels > jpegClipBufferPixels)
  {
    uint16_t *newBuffer = static_cast<uint16_t *>(realloc(jpegClipBuffer, neededPixels * sizeof(uint16_t)));
    if (!newBuffer)
    {
      return 0;
    }
    jpegClipBuffer = newBuffer;
    jpegClipBufferPixels = neededPixels;
  }

  const uint16_t *pixels = reinterpret_cast<const uint16_t *>(pDraw->pPixels);
  const int16_t srcX = clippedX - x;
  const int16_t srcY = clippedY - y;

  for (int16_t row = 0; row < clippedHeight; ++row)
  {
    const uint16_t *src = pixels + ((srcY + row) * w) + srcX;
    uint16_t *dst = jpegClipBuffer + (row * clippedWidth);

    for (int16_t col = 0; col < clippedWidth; ++col)
    {
      const uint16_t pixel = src[col];
      dst[col] = static_cast<uint16_t>((pixel >> 8) | (pixel << 8));
    }
  }

  LCD_addWindow(
      static_cast<uint16_t>(clippedX),
      static_cast<uint16_t>(clippedY),
      static_cast<uint16_t>(clippedRight - 1),
      static_cast<uint16_t>(clippedBottom - 1),
      jpegClipBuffer);

  total_show_video += millis() - started;
  return 1;
}

void mjpegPlayFromSDCard(const char *mjpegFilename)
{
  File mjpegFile = SD.open(mjpegFilename, FILE_READ);
  if (!mjpegFile || mjpegFile.isDirectory())
  {
    Serial.printf("ERROR: Failed to open %s for reading\n", mjpegFilename);
    return;
  }

  Serial.println(F("MJPEG start"));
  LCD_FillScreen(RGB565(0, 0, 0));

  start_ms = millis();
  curr_ms = start_ms;
  total_frames = 0;
  total_read_video = 0;
  total_decode_video = 0;
  total_show_video = 0;

  if (!mjpeg.setup(&mjpegFile, mjpeg_buf, jpegDrawCallback, true, 0, 0, LCD_WIDTH, LCD_HEIGHT))
  {
    mjpegFile.close();
    Serial.println("MJPEG setup failed");
    return;
  }

  while (!skipRequested && mjpegFile.available() && mjpeg.readMjpegBuf())
  {
    total_read_video += millis() - curr_ms;
    curr_ms = millis();

    mjpeg.drawJpg();
    total_decode_video += millis() - curr_ms;

    curr_ms = millis();
    ++total_frames;
  }

  const int time_used = millis() - start_ms;
  mjpegFile.close();

  skipRequested = false;

  Serial.println(F("MJPEG end"));
  if (time_used > 0)
  {
    const float fps = 1000.0f * total_frames / time_used;
    total_decode_video -= total_show_video;
    Serial.printf("Total frames: %d\n", total_frames);
    Serial.printf("Time used: %d ms\n", time_used);
    Serial.printf("Average FPS: %0.1f\n", fps);
    Serial.printf("Read MJPEG: %lu ms (%0.1f %%)\n", total_read_video, 100.0f * total_read_video / time_used);
    Serial.printf("Decode video: %lu ms (%0.1f %%)\n", total_decode_video, 100.0f * total_decode_video / time_used);
    Serial.printf("Show video: %lu ms (%0.1f %%)\n", total_show_video, 100.0f * total_show_video / time_used);
  }
}

void playSelectedMjpeg(int mjpegIndex)
{
  const String fullPath = String(MJPEG_FOLDER) + "/" + mjpegFileList[mjpegIndex];
  Serial.printf("Playing %s\n", fullPath.c_str());
  mjpegPlayFromSDCard(fullPath.c_str());
}

void setup()
{
  Serial.begin(115200);
  delay(1500);

  DEV_DEVICE_INIT();

  LCD_Init();
  Set_Backlight(DISPLAY_BRIGHTNESS_PERCENT);
  LCD_FillScreen(RGB565(0, 0, 0));

  if (!SD.begin(SD_CS, SPI, SD_SPI_FREQ, "/sd", 5, true))
  {
    haltWithMessage("ERROR: File system mount failed!");
  }

  if (SD.cardType() == CARD_NONE)
  {
    haltWithMessage("ERROR: No SD card detected!");
  }

  mjpeg_buf = static_cast<uint8_t *>(allocateVideoBuffer(MJPEG_BUFFER_SIZE));
  if (!mjpeg_buf)
  {
    haltWithMessage("ERROR: MJPEG buffer allocation failed!");
  }

  loadMjpegFilesList();
  if (mjpegCount == 0)
  {
    haltWithMessage("ERROR: No .mjpeg files found in /mjpeg");
  }

  pinMode(BTN_A, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BTN_A), onButtonPress, FALLING);
}

void loop()
{
  if (mjpegCount == 0)
  {
    delay(1000);
    return;
  }

  playSelectedMjpeg(currentMjpegIndex);

  ++currentMjpegIndex;
  if (currentMjpegIndex >= mjpegCount)
  {
    currentMjpegIndex = 0;
  }
}
