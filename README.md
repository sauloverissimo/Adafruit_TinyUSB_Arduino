# Adafruit TinyUSB Library for Arduino

[![Build Status](https://github.com/adafruit/Adafruit_TinyUSB_Arduino/workflows/Build/badge.svg)](https://github.com/adafruit/Adafruit_TinyUSB_Arduino/actions) [![License](https://img.shields.io/badge/license-MIT-brightgreen.svg)](https://opensource.org/licenses/MIT)

This library is a Arduino-friendly version of [TinyUSB](https://github.com/hathach/tinyusb) stack.
It is designed with structure and APIs that are easily integrated to an Arduino Core.

## Features

### Device Stack

Supported device class drivers are:

- Communication (CDC): which is used to implement `Serial` monitor
- Human Interface Device (HID): Generic (In & Out), Keyboard, Mouse, Gamepad etc ...
- Mass Storage Class (MSC): with multiple LUNs
- Musical Instrument Digital Interface (MIDI)
- Video (UVC): work in progress
- WebUSB with vendor specific class

### Host Stack

Host stack is available with either addition of MAX3421E hardware (e.g [Host FeatherWing](https://www.adafruit.com/product/5858)) or  rp2040 core (thanks to [Pico-PIO-USB](https://github.com/sekigon-gonnoc/Pico-PIO-USB)). Supported class driver are:

- Communication (CDC): including vendor usb2uart such as FTDI, CP210x, CH34x
- MassStorage class

Note: Host stack is still work-in-progress

## MIDI 2.0 (override)

> ⚠️ **TinyUSB override, not yet upstream.** This branch (`feat/midi2`) carries the TinyUSB [PR #3571](https://github.com/hathach/tinyusb/pull/3571) tree vendored at SHA `31d730d8bb0b5c0832c5490378a2a2dd60ab72aa`, plus two new Arduino classes (`Adafruit_USBD_MIDI2` and `Adafruit_USBH_MIDI2`) wrapping the new `tud_midi2_n_*` and `tuh_midi2_*` C APIs. Until PR #3571 merges into `hathach/tinyusb` and Adafruit upstream bumps its bundled TinyUSB to a version that includes it, this fork stays internal. Treat it as **beta**.

### What changes vs upstream

- `src/{class,device,host,portable,common,osal}` and the three root tusb files are wholesale-synced to `sauloverissimo/tinyusb` at the pinned SHA above. `tools/sync_tinyusb_fork.sh` is idempotent; bump the SHA and re-run when the PR #3571 fork advances.
- Two new Arduino classes:
  - `Adafruit_USBD_MIDI2` (device): wraps `tud_midi2_n_*` (32-bit UMP read/write, 4-byte cable event read/write, mounted/altSetting/negotiated/protocol accessors). Emits `TUD_MIDI2_DESCRIPTOR` with dual-alt (alt 0 MIDI 1.0 fallback + alt 1 UMP).
  - `Adafruit_USBH_MIDI2` (host): wraps `tuh_midi2_*` (per-idx queries + UMP read/write/flush + MountCb/UnmountCb/RxCb registration).

  Both classes do NOT inherit from `Stream` (MIDI 2.0 native is UMP word-oriented, not byte-oriented). Coexist with the legacy `Adafruit_USBD_MIDI` MIDI 1.0 class in the same firmware.
- Per-chip `tusb_config_<chip>.h` files gain `CFG_TUD_MIDI2` and `CFG_TUH_MIDI2` toggles (default 0). User enables per-sketch via `-DCFG_TUD_MIDI2=1` build flag or `#define` before `#include <Adafruit_TinyUSB.h>`.

### Supported boards (Phase 1, v0.1-midi2)

| Chip | Boards | Status |
|---|---|---|
| SAMD21 | XIAO SAMD21, generic SAMD21 | device validated on hardware |
| nRF52840 | Nice!Nano, Adafruit Feather nRF52840 Express, BlueMicro840 | device validated on hardware |
| RP2040 | Pico, Adafruit Feather RP2040 (Earle Philhower core, `usbstack=tinyusb`) | device + host (PIO-USB) validated on hardware |

Host class code ships present on all 3 chip families, but **only the RP2040 PIO-USB path is bench-validated** in v0.1-midi2. SAMD21 / nRF52840 host requires an external MAX3421E shield; bench validation deferred to v0.2-midi2 or later.

### Supported boards (Phase 2, v0.2-midi2, planned)

SAMD51, ESP32-S2, ESP32-S3.

### Install

```bash
cd ~/Arduino/libraries
git clone -b feat/midi2 https://github.com/sauloverissimo/Adafruit_TinyUSB_Arduino.git
```

### Smoke test

Open `examples/MIDI2/midi2_test/midi2_test.ino`, pick your board's FQBN, add `-DCFG_TUD_MIDI2=1` to the build flags, compile, flash. The sketch sends a JR Timestamp heartbeat every 500 ms and a NoteOn/Off C4 every 2 s.

For full demos with chromatic walk, Per-Note expression, and MIDI-CI Discovery, pair the fork with the [`midi2_cpp`](https://github.com/sauloverissimo/midi2_cpp) recipes (`xiao-samd21-midi2`, `nrf52840-promicro-midi2`).

### Upstream merge plan

This fork stays internal until TinyUSB PR #3571 merges into `hathach/tinyusb`. The merge sequence then follows the project-wide GATED order: TinyUSB merge, MIDI Association communication, ESP32 PRs, libDaisy PR, Teensy PR, and finally this fork's PR to `adafruit/Adafruit_TinyUSB_Arduino`. Until that window opens, no PR submission to Adafruit upstream.

## Supported Cores

There are 2 type of supported cores: with and without built-in support for TinyUSB. Built-in support provide seamless integration but requires extra code added to core's source code. Unfortunately it is not always easy or possible to make those modification.

### Cores with built-in support

Following core has TinyUSB as either the primary usb stack or selectable via menu `Tools->USB Stack`. You only need to include `<Adafruit_TinyUSB.h>` in your sketch to use.

- [adafruit/Adafruit_nRF52_Arduino](https://github.com/adafruit/Adafruit_nRF52_Arduino)
- [adafruit/ArduinoCore-samd](https://github.com/adafruit/ArduinoCore-samd)
- [earlephilhower/arduino-pico](https://github.com/earlephilhower/arduino-pico)
- [espressif/arduino-esp32](https://github.com/espressif/arduino-esp32) Host mode using MAX3421E controller should work with all chips. Device mode only support S2/S3/P4 and  additional Tools menu are needed 
    - `USB Mode=USB-OTG (TinyUSB)` for S3 and P4
    - `USB CDC On Boot=Enabled`, `USB Firmware MSC On Boot=Disabled`, `USB DFU On Boot=Disabled`
- [openwch/arduino_core_ch32](https://github.com/openwch/arduino_core_ch32)

Note: For ESP32 port, version before v3.0 requires all descriptors must be specified in usb objects declaration i.e constructors. Therefore all descriptor-related fields must be part of object declaration and descriptor-related API have no effect afterwards. This limitation is not the case for version from v3.0. 

### Cores without built-in support

Following is cores without built-in support

- **mbed_rp2040**
- **[stm32duino/Arduino_Core_STM32](https://github.com/stm32duino/Arduino_Core_STM32)**
  - Still WIP, only support/tested with F4, only support OTG_FS  

It is still possible to use TinyUSB but with some limits such as:

- `TinyUSB_Device_Init()` need to be manually called in setup()
- `TinyUSB_Device_Task()` and/or `TinyUSB_Device_FlushCDC()` may (or not) need to be manually called in loop()
- Use `SerialTinyUSB` name instead of Serial for serial monitor
- And there could be more other issues, using on these cores should be considered as experimental

## Class Driver API

More document to write ... 

## Porting Guide

To integrate TinyUSB library to a Arduino core, you will need to make changes to the core for built-in support and library for porting the mcu/platform.

### Arduino Core Changes

If possible, making changes to core will allow it to have built-in which make it almost transparent to user sketch 

1. Add this repo as submodule (or have local copy) at your ArduioCore/libraries/Adafruit_TinyUSB_Arduino (much like SPI).
2. Since Serial as CDC is considered as part of the core, we need to have `#include "Adafruit_USBD_CDC.h"` within your `Arduino.h`. For this to work, your `platform.txt` include path need to have `"-I{runtime.platform.path}/libraries/Adafruit_TinyUSB_Arduino/src/arduino"`.
3. In your `main.cpp` before setup() invoke the `TinyUSB_Device_Init(rhport)`. This will initialize usb device hardware and tinyusb stack and also include Serial as an instance of CDC class.
4. `TinyUSB_Device_Task()` must be called whenever there is new USB event. Depending on your core and MCU with or without RTOS. There are many ways to run the task. For example:
  - Use USB IRQn to set flag then invoke function later on after exiting IRQ.
  - Just invoke function after the loop(), within yield(), and delay()
5. `TinyUSB_Device_FlushCDC()` should also be called often to send out Serial data as well.
6. Note: For low power platform that make use of WFI()/WFE(), extra care is required before mcu go into low power mode. Check out my PR to circuipython for reference https://github.com/adafruit/circuitpython/pull/2956

### Library Changes

In addition to core changes, library need to be ported to your platform. Don't worry, tinyusb stack has already done most of heavy-lifting. You only need to write a few APIs

1. `TinyUSB_Port_InitDevice()` hardware specific (clock, phy) to enable usb hardware then call tud_init(). This API is called as part of TinyUSB_Device_Init() invocation.
2. `TinyUSB_Port_EnterDFU()` which is called when device need to enter DFU mode, usually by touch1200 feature
3. `TinyUSB_Port_GetSerialNumber()` which is called to get unique MCU Serial ID to used as Serial string descriptor.
