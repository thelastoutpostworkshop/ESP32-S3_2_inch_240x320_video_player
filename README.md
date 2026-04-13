# ESP32-S3 2" 240x320 MJPEG Video Player

This project turns the Waveshare `ESP32-S3-LCD-2` / `ESP32-S3-LCD-2-C` board into a simple MJPEG player that reads video files from a microSD card and displays them on the built-in `240x320 ST7789T3` LCD.

The hardware mapping was adapted for the board sold as:

- Waveshare `ESP32-S3 2inch IPS Display Development Board 240x320`
- Optional `OV5640` camera variant `ESP32-S3-LCD-2-C`

## Pinout Used By This Sketch

- `SPI_MOSI = GPIO38`
- `SPI_MISO = GPIO40`
- `SPI_SCK = GPIO39`
- `SD_CS = GPIO41`
- `BOOT button = GPIO0`
- `LCD_CS = GPIO45`
- `LCD_DC = GPIO42`
- `LCD_BL = GPIO1`
- `LCD_RST = not used` (`software reset` is used instead)

## Features

- Plays `.mjpeg` files from `/mjpeg` on the SD card
- Automatically scans and indexes up to `20` video files
- Loops through all discovered clips continuously
- Uses the onboard `BOOT` button to skip to the next clip
- Prints playback timing and FPS statistics to Serial
- Uses a built-in ST7789 driver, so no external display driver library is required

## Requirements

### Hardware

- Waveshare `ESP32-S3-LCD-2` or `ESP32-S3-LCD-2-C`
- microSD card formatted as `FAT32`
- USB-C cable

### Arduino Core

Tested against:

- `esp32` by Espressif Systems `3.3.8`

### Required Libraries

Install:

- `JPEGDEC` `1.8.4`

Used from the ESP32 core:

- `SD`
- `SPI`
- `FS`

## Arduino IDE / CLI Settings

The included [sketch.yaml](sketch.yaml) is set up for:

- `Board`: `ESP32S3 Dev Module`
- `Upload Speed`: `921600`
- `USB Mode`: `Hardware CDC and JTAG`
- `USB CDC On Boot`: `Enabled`
- `CPU Frequency`: `240MHz (WiFi)`
- `Flash Mode`: `QIO 80MHz`
- `Flash Size`: `16MB`
- `PSRAM`: `OPI PSRAM`

## SD Card Layout

Format the microSD card as FAT32 and create this folder structure:

```text
SD Card
└── mjpeg
    ├── clip1.mjpeg
    ├── clip2.mjpeg
    └── clip3.mjpeg
```

Important details:

- The folder must be named exactly `/mjpeg`
- Files must use the `.mjpeg` extension
- The sketch indexes up to `20` files
- The player scans only that folder, not subfolders

## Tunable Parameters

Adjust these values in `ESP32-S3_2_inch_240x320_video_player.ino`:

- `MJPEG_FOLDER`
- `MAX_FILES`
- `DISPLAY_BRIGHTNESS_PERCENT`
- `MJPEG_BUFFER_SIZE`

Display orientation and low-level panel settings are defined in `Display_ST7789.h`.
If you want landscape playback, change `LCD_ROTATION`.

## Troubleshooting

- `ERROR: File system mount failed!`
  Lower `SD_SPI_FREQ` in `Display_ST7789.h` if your card is marginal.
- `ERROR: No SD card detected!`
  Re-seat the card and confirm it is FAT32.
- `ERROR: No .mjpeg files found in /mjpeg`
  Create the `/mjpeg` folder and place valid `.mjpeg` files inside it.
- Display stays black
  Confirm the board is really the Waveshare `ESP32-S3-LCD-2` family and not the older C6 board.
- Image is rotated the wrong way
  Change `LCD_ROTATION` in `Display_ST7789.h`.
