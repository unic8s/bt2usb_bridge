#include <BluetoothHCI.h>
#include <BluetoothHIDMaster.h>
#include "Keyboard.h"


#define BT_USB_OFFSET     61
#define KEY_ASCII_OFFET   13

#define DEVICE_NAME       "HD2 Macropad"


BluetoothHIDMaster hid;
BluetoothHCI hci;


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
  pinMode(LED_BUILTIN, OUTPUT);

  setupModifierMap();

  Serial.begin();
  delay(3000);

  Keyboard.begin();

  hid.onKeyDown(kb, (void *)true);
  hid.onKeyUp(kb, (void *)false);

  connectMacropad();
}

bool bleScanning = false;
bool bleRescan = false;
bool bleFound = false;
unsigned long lastScan = 0;

void connectMacropad(){
  unsigned long now = millis();

  if(bleFound || bleScanning || now - lastScan < 5000){
    if(!bleFound && !hid.connected()){
      bleRescan = true;
    }

    return;
  }

  bleScanning = true;
  lastScan = now;

  hid.end();

  delay(1000);

  hci.install();
  hci.begin();

  auto l = hci.scanBLE(BluetoothHCI::any_cod);

  Serial.printf("%-8s | %-17s | %-4s | %s\n", "Class", "Address", "RSSI", "Name");
  Serial.printf("%-8s | %-17s | %-4s | %s\n", "--------", "-----------------", "----", "----------------");

  for (auto e : l) {
    Serial.printf("%08lx | %17s | %4d | %s\n", e.deviceClass(), e.addressString(), e.rssi(), e.name());

    String deviceName = e.name();

    if(deviceName == DEVICE_NAME){
      bleFound = true;
      bleRescan = false;

      hci.scanFree();
      hci.uninstall();

      delay(1000);

      hid.begin(true);
      hid.connectBLE(e.address(), e.addressType());

      Serial.println("Device connected");
      
      bleScanning = false;
      return;
    }
  }

  Serial.println("No device found");

  if(!hid.connected()){
    bleRescan = true;
  }
  
  bleScanning = false;
}

void loop() {
  if (BOOTSEL) {
    while (BOOTSEL) {
      delay(1);
    }

    Serial.println("Cleanup after reset");

    bleFound = false;
    digitalWrite(LED_BUILTIN, LOW);

    hid.disconnect();
    hid.clearPairing();

    delay(1000);

    Serial.println("Rescanning");

    connectMacropad();
  }

  if(!hid.connected() || bleRescan){
    connectMacropad();
  }

  digitalWrite(LED_BUILTIN, bleFound ? HIGH : LOW);

  yield();
  delay(1);
}
