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

#ifndef ADAFRUIT_USBD_MIDI2_H_
#define ADAFRUIT_USBD_MIDI2_H_

#include "arduino/Adafruit_USBD_Device.h"

class Adafruit_USBD_MIDI2 : public Adafruit_USBD_Interface {
public:
  Adafruit_USBD_MIDI2(void);

  // String descriptor index for the Function Block / cable name. Use the
  // setStringDescriptor inherited from Adafruit_USBD_Interface, OR call
  // setStringDescriptor() before begin().
  bool begin(void);

  // Lifecycle / negotiation
  bool mounted(void);
  uint8_t altSetting(void); // 0 = MIDI 1.0 fallback, 1 = UMP MIDI 2.0
  bool negotiated(void);    // UMP Stream Configuration acknowledged
  uint8_t protocol(void);   // current protocol per Stream Configuration

  // Native MIDI 2.0: 32-bit UMP words
  uint32_t write(const uint32_t *words, uint32_t count);
  uint32_t read(uint32_t *buf, uint32_t max_words);
  uint32_t available(void);

  // Legacy 4-byte cable event compat (alt 0 fallback or apps that do not
  // assemble UMPs themselves)
  bool writePacket(const uint8_t packet[4]);
  bool readPacket(uint8_t packet[4]);

  // From Adafruit_USBD_Interface
  virtual uint16_t getInterfaceDescriptor(uint8_t itfnum_deprecated,
                                          uint8_t *buf, uint16_t bufsize);

private:
  bool _begun;
};

#endif /* ADAFRUIT_USBD_MIDI2_H_ */
