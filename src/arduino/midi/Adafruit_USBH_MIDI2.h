/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2026 Saulo Verissimo
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 */

#ifndef ADAFRUIT_USBH_MIDI2_H_
#define ADAFRUIT_USBH_MIDI2_H_

#include <stdint.h>

#include "tusb_option.h"

#if CFG_TUH_ENABLED && CFG_TUH_MIDI2
// Pull the typedef so the user can read tuh_midi2_mount_cb_t fields
// from the MountCb signature without including tusb headers manually.
#include "class/midi/midi2_host.h"
#else
// Stub typedef so the header parses even when CFG_TUH_MIDI2 is off
// (the .cpp body is gated identically; class methods compile to
// no-ops or are never called in that configuration).
struct tuh_midi2_mount_cb_t_stub {};
typedef struct tuh_midi2_mount_cb_t_stub tuh_midi2_mount_cb_t;
#endif

class Adafruit_USBH_MIDI2 {
public:
  // Per-driver-index identifiers. TinyUSB hands out an index when a
  // MIDI 2.0 device is mounted; query / read / write all use that idx.
  // Multiple devices can be mounted at the same time via different idx
  // values (CFG_TUH_MIDI2_MAX bounds the count).
  typedef void (*MountCb)(uint8_t idx, const tuh_midi2_mount_cb_t *info);
  typedef void (*UnmountCb)(uint8_t idx);
  typedef void (*RxCb)(uint8_t idx, uint32_t xferred_bytes);

  Adafruit_USBH_MIDI2(void);

  bool begin(void); // currently a no-op; reserved for future per-class init

  // Per-idx queries. Pass the idx value the mount callback delivered.
  bool mounted(uint8_t idx);
  uint8_t protocolVersion(uint8_t idx);  // 0 = MIDI 1.0, 1 = MIDI 2.0
  uint8_t altSettingActive(uint8_t idx); // 0 = MIDI 1.0 alt, 1 = UMP
  uint8_t cableCount(uint8_t idx);

  // 32-bit UMP word IO (native MIDI 2.0)
  uint32_t read(uint8_t idx, uint32_t *words, uint32_t max_words);
  uint32_t write(uint8_t idx, const uint32_t *words, uint32_t count);
  uint32_t flush(uint8_t idx);

  // Optional callback registration. When the user calls these, the
  // wrapper's internal weak overrides of the TinyUSB tuh_midi2_*_cb
  // functions invoke the registered function pointers. Apps that prefer
  // to define their own weak overrides directly can leave these unset
  // and the wrapper's default no-op weak overrides do not compete.
  void setMountCallback(MountCb cb);
  void setUnmountCallback(UnmountCb cb);
  void setRxCallback(RxCb cb);
};

#endif /* ADAFRUIT_USBH_MIDI2_H_ */
