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

#if CFG_TUD_ENABLED && CFG_TUD_MIDI2

#include "Adafruit_USBD_MIDI2.h"
#include "tusb.h"
#include <string.h>

// Compile-time guard so the user sees a clear error if they instantiate
// Adafruit_USBD_MIDI2 without enabling CFG_TUD_MIDI2 in their tusb_config.
// The #if above gates the implementation block; this static_assert backs
// it up with a readable message at compile time.
static_assert(CFG_TUD_MIDI2 == 1,
              "Adafruit_USBD_MIDI2 requires CFG_TUD_MIDI2=1. "
              "Add -DCFG_TUD_MIDI2=1 to your build flags or define it in "
              "tusb_config_<chip>.h before including Adafruit_TinyUSB.h.");

Adafruit_USBD_MIDI2::Adafruit_USBD_MIDI2(void) : _begun(false) {}

bool Adafruit_USBD_MIDI2::begin(void) {
  if (_begun)
    return false;
  if (!TinyUSBDevice.addInterface(*this))
    return false;
  _begun = true;
  return true;
}

bool Adafruit_USBD_MIDI2::mounted(void) { return tud_midi2_n_mounted(0); }
uint8_t Adafruit_USBD_MIDI2::altSetting(void) {
  return tud_midi2_n_alt_setting(0);
}
bool Adafruit_USBD_MIDI2::negotiated(void) { return tud_midi2_n_negotiated(0); }
uint8_t Adafruit_USBD_MIDI2::protocol(void) { return tud_midi2_n_protocol(0); }

uint32_t Adafruit_USBD_MIDI2::write(const uint32_t *words, uint32_t count) {
  return tud_midi2_n_ump_write(0, words, count);
}

uint32_t Adafruit_USBD_MIDI2::read(uint32_t *buf, uint32_t max_words) {
  return tud_midi2_n_ump_read(0, buf, max_words);
}

uint32_t Adafruit_USBD_MIDI2::available(void) {
  return tud_midi2_n_available(0);
}

bool Adafruit_USBD_MIDI2::writePacket(const uint8_t packet[4]) {
  return tud_midi2_n_packet_write(0, packet);
}

bool Adafruit_USBD_MIDI2::readPacket(uint8_t packet[4]) {
  return tud_midi2_n_packet_read(0, packet);
}

uint16_t Adafruit_USBD_MIDI2::getInterfaceDescriptor(uint8_t itfnum_deprecated,
                                                     uint8_t *buf,
                                                     uint16_t bufsize) {
  (void)itfnum_deprecated;

  uint16_t const desc_len = TUD_MIDI2_DESC_LEN;

  // null buffer is used to query descriptor length only
  if (!buf)
    return desc_len;

  if (bufsize < desc_len)
    return 0;

  // MIDI 2.0 uses 2 interfaces (Audio Control + MIDI Streaming) just like
  // legacy MIDI 1.0; TUD_MIDI2_DESCRIPTOR adds the alt 1 (UMP) descriptors
  // on top of the alt 0 (MIDI 1.0 compat) ones. Allocate accordingly.
  uint8_t itfnum = TinyUSBDevice.allocInterface(2);
  uint8_t ep_in = TinyUSBDevice.allocEndpoint(TUSB_DIR_IN);
  uint8_t ep_out = TinyUSBDevice.allocEndpoint(TUSB_DIR_OUT);

  uint8_t const desc[] = {
      TUD_MIDI2_DESCRIPTOR(itfnum, _strid, ep_out, ep_in, 64)};
  static_assert(
      sizeof(desc) == TUD_MIDI2_DESC_LEN,
      "TUD_MIDI2_DESC_LEN does not match TUD_MIDI2_DESCRIPTOR expansion");

  memcpy(buf, desc, sizeof(desc));
  return sizeof(desc);
}

#endif /* CFG_TUD_ENABLED && CFG_TUD_MIDI2 */
