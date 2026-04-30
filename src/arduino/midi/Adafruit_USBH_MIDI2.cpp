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

#include "tusb_option.h"

#if CFG_TUH_ENABLED && CFG_TUH_MIDI2

#include "Adafruit_USBH_MIDI2.h"
#include "tusb.h"

static_assert(CFG_TUH_MIDI2 == 1,
              "Adafruit_USBH_MIDI2 requires CFG_TUH_MIDI2=1. "
              "Add -DCFG_TUH_MIDI2=1 to your build flags or define it in "
              "tusb_config_<chip>.h before including Adafruit_TinyUSB.h.");

namespace {
Adafruit_USBH_MIDI2::MountCb g_mount_cb = nullptr;
Adafruit_USBH_MIDI2::UnmountCb g_unmount_cb = nullptr;
Adafruit_USBH_MIDI2::RxCb g_rx_cb = nullptr;
} // namespace

Adafruit_USBH_MIDI2::Adafruit_USBH_MIDI2(void) {}

bool Adafruit_USBH_MIDI2::begin(void) { return true; }

bool Adafruit_USBH_MIDI2::mounted(uint8_t idx) {
  return tuh_midi2_mounted(idx);
}
uint8_t Adafruit_USBH_MIDI2::protocolVersion(uint8_t idx) {
  return tuh_midi2_get_protocol_version(idx);
}
uint8_t Adafruit_USBH_MIDI2::altSettingActive(uint8_t idx) {
  return tuh_midi2_get_alt_setting_active(idx);
}
uint8_t Adafruit_USBH_MIDI2::cableCount(uint8_t idx) {
  return tuh_midi2_get_cable_count(idx);
}

uint32_t Adafruit_USBH_MIDI2::read(uint8_t idx, uint32_t *words,
                                   uint32_t max_words) {
  return tuh_midi2_ump_read(idx, words, max_words);
}

uint32_t Adafruit_USBH_MIDI2::write(uint8_t idx, const uint32_t *words,
                                    uint32_t count) {
  return tuh_midi2_ump_write(idx, words, count);
}

uint32_t Adafruit_USBH_MIDI2::flush(uint8_t idx) {
  return tuh_midi2_write_flush(idx);
}

void Adafruit_USBH_MIDI2::setMountCallback(MountCb cb) { g_mount_cb = cb; }
void Adafruit_USBH_MIDI2::setUnmountCallback(UnmountCb cb) {
  g_unmount_cb = cb;
}
void Adafruit_USBH_MIDI2::setRxCallback(RxCb cb) { g_rx_cb = cb; }

// Weak overrides of TinyUSB host-side application callbacks. When the
// user registers a function pointer through setMountCallback /
// setUnmountCallback / setRxCallback, these stubs route the event.
// Apps that want their own weak override can simply leave the wrapper's
// callbacks unset (or supply their own non-weak override that the
// linker prefers).
extern "C" {

void tuh_midi2_mount_cb(uint8_t idx, const tuh_midi2_mount_cb_t *info) {
  if (g_mount_cb)
    g_mount_cb(idx, info);
}

void tuh_midi2_umount_cb(uint8_t idx) {
  if (g_unmount_cb)
    g_unmount_cb(idx);
}

void tuh_midi2_rx_cb(uint8_t idx, uint32_t xferred_bytes) {
  if (g_rx_cb)
    g_rx_cb(idx, xferred_bytes);
}

} // extern "C"

#endif /* CFG_TUH_ENABLED && CFG_TUH_MIDI2 */
