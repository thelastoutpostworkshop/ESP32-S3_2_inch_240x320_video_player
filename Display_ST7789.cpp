#include "Display_ST7789.h"

namespace
{
constexpr uint8_t ST7789_SWRESET = 0x01;
constexpr uint8_t ST7789_SLPOUT = 0x11;
constexpr uint8_t ST7789_NORON = 0x13;
constexpr uint8_t ST7789_INVON = 0x21;
constexpr uint8_t ST7789_DISPON = 0x29;
constexpr uint8_t ST7789_CASET = 0x2A;
constexpr uint8_t ST7789_RASET = 0x2B;
constexpr uint8_t ST7789_RAMWR = 0x2C;
constexpr uint8_t ST7789_MADCTL = 0x36;
constexpr uint8_t ST7789_COLMOD = 0x3A;

constexpr uint8_t ST7789_MADCTL_MY = 0x80;
constexpr uint8_t ST7789_MADCTL_MX = 0x40;
constexpr uint8_t ST7789_MADCTL_MV = 0x20;
constexpr uint8_t ST7789_MADCTL_RGB = 0x00;

void beginWriteCommand(uint8_t value)
{
  SPI.beginTransaction(SPISettings(DISPLAY_SPI_FREQ, MSBFIRST, DISPLAY_SPI_MODE));
  digitalWrite(LCD_CS, LOW);
  digitalWrite(LCD_DC, LOW);
  SPI.transfer(value);
  digitalWrite(LCD_CS, HIGH);
  SPI.endTransaction();
}

void beginWriteData(uint8_t value)
{
  SPI.beginTransaction(SPISettings(DISPLAY_SPI_FREQ, MSBFIRST, DISPLAY_SPI_MODE));
  digitalWrite(LCD_CS, LOW);
  digitalWrite(LCD_DC, HIGH);
  SPI.transfer(value);
  digitalWrite(LCD_CS, HIGH);
  SPI.endTransaction();
}

void writeDataBytes(const uint8_t *data, size_t size)
{
  SPI.beginTransaction(SPISettings(DISPLAY_SPI_FREQ, MSBFIRST, DISPLAY_SPI_MODE));
  digitalWrite(LCD_CS, LOW);
  digitalWrite(LCD_DC, HIGH);
  SPI.transferBytes(const_cast<uint8_t *>(data), nullptr, size);
  digitalWrite(LCD_CS, HIGH);
  SPI.endTransaction();
}

void LCD_Reset()
{
  if (LCD_RST >= 0)
  {
    digitalWrite(LCD_RST, HIGH);
    delay(100);
    digitalWrite(LCD_RST, LOW);
    delay(120);
    digitalWrite(LCD_RST, HIGH);
    delay(120);
  }

  beginWriteCommand(ST7789_SWRESET);
  delay(120);
}

uint8_t rotationToMadctl(uint8_t rotation)
{
  switch (rotation & 0x03U)
  {
  case 1:
    return ST7789_MADCTL_MX | ST7789_MADCTL_MV | ST7789_MADCTL_RGB;
  case 2:
    return ST7789_MADCTL_MX | ST7789_MADCTL_MY | ST7789_MADCTL_RGB;
  case 3:
    return ST7789_MADCTL_MY | ST7789_MADCTL_MV | ST7789_MADCTL_RGB;
  default:
    return ST7789_MADCTL_RGB;
  }
}
} // namespace

void LCD_Init()
{
  pinMode(LCD_CS, OUTPUT);
  pinMode(LCD_DC, OUTPUT);
  if (LCD_RST >= 0)
  {
    pinMode(LCD_RST, OUTPUT);
  }

  digitalWrite(LCD_CS, HIGH);
  digitalWrite(LCD_DC, HIGH);
  if (LCD_RST >= 0)
  {
    digitalWrite(LCD_RST, HIGH);
  }

  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);

  ledcAttach(GFX_BL, BACKLIGHT_PWM_FREQ, BACKLIGHT_PWM_RESOLUTION);
  ledcWrite(GFX_BL, 0);

  LCD_Reset();

  // Waveshare's ESP32-S3-LCD-2 uses an ST7789T3 over SPI.
  beginWriteCommand(ST7789_SLPOUT);
  delay(120);

  beginWriteCommand(ST7789_COLMOD);
  beginWriteData(0x55);

  beginWriteCommand(ST7789_MADCTL);
  beginWriteData(rotationToMadctl(LCD_ROTATION));

  beginWriteCommand(0xB0);
  beginWriteData(0x00);
  beginWriteData(0xF0);

  beginWriteCommand(0xB2);
  beginWriteData(0x0C);
  beginWriteData(0x0C);
  beginWriteData(0x00);
  beginWriteData(0x33);
  beginWriteData(0x33);

  beginWriteCommand(0xB7);
  beginWriteData(0x35);

  beginWriteCommand(0xBB);
  beginWriteData(0x19);

  beginWriteCommand(0xC0);
  beginWriteData(0x2C);

  beginWriteCommand(0xC2);
  beginWriteData(0x01);

  beginWriteCommand(0xC3);
  beginWriteData(0x12);

  beginWriteCommand(0xC4);
  beginWriteData(0x20);

  beginWriteCommand(0xC6);
  beginWriteData(0x0F);

  beginWriteCommand(0xD0);
  beginWriteData(0xA4);
  beginWriteData(0xA1);

  beginWriteCommand(0xE0);
  beginWriteData(0xF0);
  beginWriteData(0x09);
  beginWriteData(0x13);
  beginWriteData(0x12);
  beginWriteData(0x12);
  beginWriteData(0x2B);
  beginWriteData(0x3C);
  beginWriteData(0x44);
  beginWriteData(0x4B);
  beginWriteData(0x1B);
  beginWriteData(0x18);
  beginWriteData(0x17);
  beginWriteData(0x1D);
  beginWriteData(0x21);

  beginWriteCommand(0xE1);
  beginWriteData(0xF0);
  beginWriteData(0x09);
  beginWriteData(0x13);
  beginWriteData(0x0C);
  beginWriteData(0x0D);
  beginWriteData(0x27);
  beginWriteData(0x3B);
  beginWriteData(0x44);
  beginWriteData(0x4D);
  beginWriteData(0x0B);
  beginWriteData(0x17);
  beginWriteData(0x17);
  beginWriteData(0x1D);
  beginWriteData(0x21);

  beginWriteCommand(ST7789_NORON);
  delay(10);

  beginWriteCommand(ST7789_INVON);
  beginWriteCommand(ST7789_DISPON);
}

void LCD_SetCursor(uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end)
{
  beginWriteCommand(ST7789_CASET);
  beginWriteData(x_start >> 8);
  beginWriteData(x_start & 0xFF);
  beginWriteData(x_end >> 8);
  beginWriteData(x_end & 0xFF);

  beginWriteCommand(ST7789_RASET);
  beginWriteData(y_start >> 8);
  beginWriteData(y_start & 0xFF);
  beginWriteData(y_end >> 8);
  beginWriteData(y_end & 0xFF);

  beginWriteCommand(ST7789_RAMWR);
}

void LCD_addWindow(uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end, const uint16_t *color)
{
  const uint16_t width = x_end - x_start + 1;
  const uint16_t height = y_end - y_start + 1;
  const size_t num_bytes = static_cast<size_t>(width) * height * sizeof(uint16_t);

  LCD_SetCursor(x_start, y_start, x_end, y_end);
  writeDataBytes(reinterpret_cast<const uint8_t *>(color), num_bytes);
}

void LCD_FillScreen(uint16_t color)
{
  static uint16_t color_line[LCD_WIDTH];

  for (uint16_t x = 0; x < LCD_WIDTH; ++x)
  {
    color_line[x] = color;
  }

  for (uint16_t y = 0; y < LCD_HEIGHT; ++y)
  {
    LCD_addWindow(0, y, LCD_WIDTH - 1, y, color_line);
  }
}

void Set_Backlight(uint8_t percent)
{
  percent = constrain(percent, 0, 100);
  const uint32_t duty_max = (1U << BACKLIGHT_PWM_RESOLUTION) - 1U;
  const uint32_t duty = (static_cast<uint32_t>(percent) * duty_max) / 100U;
  ledcWrite(GFX_BL, duty);
}

uint16_t RGB565(uint8_t r, uint8_t g, uint8_t b)
{
  return static_cast<uint16_t>(((r & 0xF8U) << 8) | ((g & 0xFCU) << 3) | (b >> 3));
}
