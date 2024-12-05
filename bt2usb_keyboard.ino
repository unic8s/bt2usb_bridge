// KeyboardPiano example - Released to the public domain in 2024 by Earle F. Philhower, III
//
// Demonstrates using the BluetoothHIDMaster class to use a Bluetooth keyboard as a
// piano keyboard on the PicoW
//
// Hook up a phono plug to GP0 and GP1 (and GND of course...the 1st 3 pins on the PCB)
// Connect wired earbuds up and connect over BT from your keyboard and play some music.


#include <BluetoothHCI.h>
#include <BluetoothHIDMaster.h>
#include "Keyboard.h"

// We need the inverse map, borrow from the Keyboard library
#include <HID_Keyboard.h>
extern const uint8_t KeyboardLayout_en_US[128];

BluetoothHIDMaster hid;
HIDKeyStream keystream; 


// We get make/break for every key which lets us hold notes while a key is depressed
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

  // Setup the HID key to ASCII conversion
  keystream.begin();

  // We can use the cbData as a flag to see if we're making or breaking a key
  hid.onKeyDown(kb, (void *)true);
  hid.onKeyUp(kb, (void *)false);

  hid.begin(true);

  hid.connectBLE();
  // or hid.connectMouse();

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
