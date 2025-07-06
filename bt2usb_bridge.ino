#include <BluetoothHCI.h>
#include <BluetoothHIDMaster.h>
#include "Keyboard.h"
//#include "WiFiManager.h"


#define BT_USB_OFFSET     61
#define KEY_ASCII_OFFET   13

#define SCAN_NAME         "HD2 Macropad BT"
#define DEVICE_NAME       "BT2USB Keyoard"
//#define WIFI_PASSPHRASE   "superearth"


BluetoothHIDMaster hid;
BluetoothHCI hci;


//WiFiManager captivePortal(DEVICE_NAME, WIFI_PASSPHRASE);


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

    if(deviceName == SCAN_NAME){
      bleFound = true;
      bleRescan = false;

      hci.scanFree();
      hci.uninstall();

      delay(1000);

      hid.begin(true);
      hid.connectBLE(e.address(), e.addressType());

      Serial.println("Device connected");
      
      bleRescan = false;
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
  }

  if(!hid.connected() || bleRescan){
    delay(1000);

    Serial.println("Rescanning");

    bleFound = false;

    connectMacropad();
  }

  yield();
  delay(1);
}


unsigned long blinkTimer = 0;
unsigned int blinkInterval = 500;
bool blinkState = false;

void setup1(){
  //bool success = captivePortal.autoConnect();
}

void loop1(){
  if(hid.connected()){
    digitalWrite(LED_BUILTIN, HIGH);
  }else{
    unsigned long now = millis();

    if(now - blinkTimer > blinkInterval){
      blinkState = !blinkState;
      blinkTimer = now;

      digitalWrite(LED_BUILTIN, blinkState ? HIGH : LOW);
    }
  }

  yield();
  delay(1);
}