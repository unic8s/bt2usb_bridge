#include <BluetoothHCI.h>
#include <BluetoothHIDMaster.h>
#include "Keyboard.h"


#define BT_USB_OFFSET 61
#define KEY_ASCII_OFFET 13


BluetoothHIDMaster hid;


uint8_t modifierMap[128] = {};

void setupModifierMap() {
  modifierMap[0xe0] = KEY_LEFT_CTRL;
}

void kb(void *cbdata, int key) {
  bool state = (bool)cbdata;
  int mapKey = key;
  uint8_t modifierKey = modifierMap[key];

  if (modifierKey) {
    mapKey = modifierKey;
  } else {
    mapKey = key + BT_USB_OFFSET;
  }

  if (state) {
    Keyboard.press(mapKey);
  } else {
    Keyboard.release(mapKey);
  }

  char keyChar = (key + KEY_ASCII_OFFET) + '0';

  Serial.printf("Keyboard: %d %s = '%c'\n", key, state ? "DOWN" : "UP", state ? keyChar : ' ');
}

void setup() {
  setupModifierMap();

  Serial.begin();
  delay(3000);

  Serial.printf("Starting HID master, put your device in pairing mode now.\n");

  hid.onKeyDown(kb, (void *)true);
  hid.onKeyUp(kb, (void *)false);

  hid.begin(true);
  hid.connectBLE();

  Keyboard.begin();
}

void loop() {
  if (BOOTSEL) {
    while (BOOTSEL) {
      delay(1);
    }

    hid.disconnect();
    hid.clearPairing();

    Serial.printf("Restarting HID master, put your device in pairing mode now.\n");

    hid.connectBLE();
  }
}

void setup1() {
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop1() {
  digitalWrite(LED_BUILTIN, hid.connected() ? HIGH : LOW);

  yield();
  delay(1);
}
