#pragma once

#include <Arduino.h>
#include <SPI.h>

constexpr uint16_t LCD_PANEL_WIDTH = 240;
constexpr uint16_t LCD_PANEL_HEIGHT = 320;
constexpr uint8_t LCD_ROTATION = 0;
constexpr uint16_t LCD_WIDTH = (LCD_ROTATION & 0x01U) ? LCD_PANEL_HEIGHT : LCD_PANEL_WIDTH;
constexpr uint16_t LCD_HEIGHT = (LCD_ROTATION & 0x01U) ? LCD_PANEL_WIDTH : LCD_PANEL_HEIGHT;

constexpr int SPI_MISO = 40;
constexpr int SPI_MOSI = 38;
constexpr int SPI_SCK = 39;

constexpr int SD_CS = 41;
constexpr int BTN_A = 0;

constexpr int LCD_CS = 45;
constexpr int LCD_DC = 42;
constexpr int LCD_RST = -1;
constexpr int GFX_BL = 1;

constexpr uint32_t DISPLAY_SPI_FREQ = 80000000UL;
constexpr uint32_t SD_SPI_FREQ = 25000000UL;
constexpr uint8_t DISPLAY_SPI_MODE = SPI_MODE3;
constexpr uint32_t BACKLIGHT_PWM_FREQ = 1000;
constexpr uint8_t BACKLIGHT_PWM_RESOLUTION = 10;

// Keep the SD card deselected while the shared SPI bus is configured.
#define DEV_DEVICE_INIT()      \
  do                           \
  {                            \
    pinMode(LCD_CS, OUTPUT);   \
    digitalWrite(LCD_CS, HIGH);\
    pinMode(SD_CS, OUTPUT);    \
    digitalWrite(SD_CS, HIGH); \
  } while (0)

void LCD_Init();
void LCD_SetCursor(uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end);
void LCD_addWindow(uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end, const uint16_t *color);
void LCD_FillScreen(uint16_t color);
void Set_Backlight(uint8_t percent);
uint16_t RGB565(uint8_t r, uint8_t g, uint8_t b);
