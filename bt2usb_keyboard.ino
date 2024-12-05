#include <BluetoothHCI.h>
#include <BluetoothHIDMaster.h>
#include "Keyboard.h"
#include <HID_Keyboard.h>

extern const uint8_t KeyboardLayout_en_US[128];

BluetoothHIDMaster hid;
HIDKeyStream keystream; 


void kb(void *cbdata, int key) {
  bool state = (bool)cbdata;

  if(key >= 0xe0 && key <= 0xe8){
    if(state){
      Keyboard.press(KEY_LEFT_CTRL);
    }else{
      Keyboard.release(KEY_LEFT_CTRL);
    }

    Serial.printf("Modifier: %02x %s'\n", key, state ? "DOWN" : "UP");
  }else if(state){
    keystream.write((uint8_t)key);
    keystream.write((uint8_t)state);

    char keyChar = keystream.read();
    Keyboard.print(keyChar);

    Serial.printf("Keyboard: %02x %s = '%c'\n", key, state ? "DOWN" : "UP", state ? keyChar : '-');
  }
}

void setup() {
  Serial.begin();
  delay(3000);

  Serial.printf("Starting HID master, put your device in pairing mode now.\n");

  keystream.begin();

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
