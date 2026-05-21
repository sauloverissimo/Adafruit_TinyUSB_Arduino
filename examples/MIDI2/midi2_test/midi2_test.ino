/*
 * midi2_test.ino, smoke test for Adafruit_USBD_MIDI2.
 *
 * Minimum sketch demonstrating that the fork compiles and enumerates
 * as a USB MIDI 2.0 device. Real recipes (XIAO SAMD21, nRF52840
 * Pro Micro, etc.) live in midi2_cpp/examples/.
 *
 * Identity: VID 0xCAFE, PID 0x40FF (Tier 3 / experimental window).
 *
 * After flash, host sees the device as MIDI 2.0; LED on LED_BUILTIN
 * lights when mounted; emits a NoteOn/Off C4 every 2 seconds and a
 * JR Timestamp heartbeat every 500 ms.
 *
 * Build flag required: -DCFG_TUD_MIDI2=1
 *
 * Tested FQBNs (compile only):
 *   adafruit:samd:adafruit_metro_m0:usbstack=tinyusb
 *   adafruit:samd:adafruit_metro_m4:speed=120,usbstack=tinyusb
 *   adafruit:nrf52:feather52840
 *   adafruit:nrf52:nice_nano
 *   rp2040:rp2040:rpipico:usbstack=tinyusb
 *   rp2040:rp2040:adafruit_feather:usbstack=tinyusb
 */

#include <Adafruit_TinyUSB.h>

// Self-gate: when the platform does not enable USB device MIDI 2.0
// (CFG_TUD_ENABLED off, or CFG_TUD_MIDI2 explicitly off, or chip family
// without MIDI 2.0 support yet like ESP32 / CH32V), this sketch
// compiles to empty setup()/loop() stubs so it does not break upstream
// CI matrix rows that include all examples for all platforms.
#if CFG_TUD_ENABLED && CFG_TUD_MIDI2

Adafruit_USBD_MIDI2 usb_midi2;

static const uint32_t HEARTBEAT_MS    = 500;
static const uint32_t NOTE_PERIOD_MS  = 2000;
static const uint32_t NOTE_HOLD_MS    = 200;
static const uint8_t  NOTE            = 60;   // C4

static uint32_t last_heartbeat = 0;
static uint32_t last_note_on   = 0;
static bool     note_on        = false;

// Build a JR Timestamp UMP (MT 0x0, status 0x2) with a fixed timestamp
// body; the host uses it to keep its polling loop alive on idle
// endpoints. 1 tick = 1/31250 s per spec; the value used here is just
// a placeholder.
static uint32_t jr_timestamp_ump(void) {
  return ((uint32_t)0x0 << 28) | ((uint32_t)0x2 << 20) | 0x0001u;
}

// Build a MIDI 2.0 NoteOn UMP (MT 0x4, status 0x9) on group 0
// channel 0, with a 16-bit velocity in the low half of word 1.
static void note_on_ump(uint32_t *w, uint8_t note, uint16_t vel) {
  w[0] = ((uint32_t)0x4 << 28) |
         ((uint32_t)0x0 << 24) |
         ((uint32_t)0x9 << 20) |
         ((uint32_t)0x0 << 16) |
         ((uint32_t)note << 8);
  w[1] = ((uint32_t)vel << 16);
}

static void note_off_ump(uint32_t *w, uint8_t note) {
  w[0] = ((uint32_t)0x4 << 28) |
         ((uint32_t)0x0 << 24) |
         ((uint32_t)0x8 << 20) |
         ((uint32_t)0x0 << 16) |
         ((uint32_t)note << 8);
  w[1] = 0;
}

void setup() {
  // Identity: educational VID + Tier 3 PID. Replace both before
  // shipping a real product (pid.codes, V-USB, or USB-IF VID).
  TinyUSBDevice.setID(0xCAFE, 0x40FF);
  TinyUSBDevice.setManufacturerDescriptor("github.com/sauloverissimo");
  TinyUSBDevice.setProductDescriptor("MIDI2_TEST");

  usb_midi2.setStringDescriptor("Main");
  usb_midi2.begin();

  // Force re-enumeration so the host sees the MIDI 2.0 interface that
  // was added AFTER the auto-init done by cores that bring up TinyUSB
  // before setup() runs (Earle Philhower's arduino-pico is the
  // canonical case). 200 ms is enough for Linux to process the
  // disconnect cleanly; shorter delays leave the host in an
  // inconsistent state. On cores that init TinyUSB lazily (Adafruit
  // SAMD/nRF52), this cycle is a no-op since mounted() is false here.
  if (TinyUSBDevice.mounted()) {
    TinyUSBDevice.detach();
    delay(200);
    TinyUSBDevice.attach();
  }

  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  bool mounted = usb_midi2.mounted();
  digitalWrite(LED_BUILTIN, mounted);

  // Only emit when mounted on the UMP alt setting (1).
  if (!mounted || usb_midi2.altSetting() != 1) return;

  uint32_t now = millis();

  if (now - last_heartbeat >= HEARTBEAT_MS) {
    uint32_t ump = jr_timestamp_ump();
    usb_midi2.write(&ump, 1);
    last_heartbeat = now;
  }

  if (!note_on && (now - last_note_on >= NOTE_PERIOD_MS)) {
    uint32_t w[2];
    note_on_ump(w, NOTE, 0xC000);
    usb_midi2.write(w, 2);
    last_note_on = now;
    note_on = true;
  } else if (note_on && (now - last_note_on >= NOTE_HOLD_MS)) {
    uint32_t w[2];
    note_off_ump(w, NOTE);
    usb_midi2.write(w, 2);
    note_on = false;
  }
}

#else // CFG_TUD_ENABLED && CFG_TUD_MIDI2

// No-op stubs so this sketch compiles to nothing on platforms without
// USB device MIDI 2.0 support. Lets upstream CI run the example through
// every chip family without breaking on chips where CFG_TUD_MIDI2 is 0
// (ESP32, CH32V, host-only RP2040 mode).
void setup() {}
void loop() {}

#endif // CFG_TUD_ENABLED && CFG_TUD_MIDI2
