// relay_toggle_task.cpp
#include "relay_toggle_task.h"
#include <Preferences.h>

// Optional: if you want to wipe LittleFS files on reset
#include <LittleFS.h>

// If you already have these in your project, keep using them
extern void startAP();                   // your AP-mode function
static Preferences prefs;                // for storing relays (namespace "relays")

static const int kRelayPins[4] = {RELAY1_GPIO, RELAY2_GPIO, RELAY3_GPIO, RELAY4_GPIO};
static const int kButtonPins[4] = {KEY1_GPIO,  KEY2_GPIO,  KEY3_GPIO,  KEY4_GPIO};

static bool relayState[4] = {false,false,false,false};  // OFF at boot; you can load from NVS if you want

static inline void writeRelayPin(int gpio, bool on) {
#if ACTIVE_LOW
  digitalWrite(gpio, on ? LOW : HIGH);
#else
  digitalWrite(gpio, on ? HIGH : LOW);
#endif
}

void relays_write_all(bool r1, bool r2, bool r3, bool r4) {
  bool v[4] = {r1,r2,r3,r4};
  for (int i=0;i<4;i++) {
    relayState[i] = v[i];
    writeRelayPin(kRelayPins[i], relayState[i]);
  }
}

void relays_toggle_by_index(uint8_t idx) {
  if (idx > 3) return;
  relayState[idx] = !relayState[idx];
  writeRelayPin(kRelayPins[idx], relayState[idx]);
}

// quick NVS erase for relays
static void eraseRelaysFromNVS() {
  if (prefs.begin("relays", false)) {
    prefs.remove("list");     // your code stores JSON list under key "list"
    prefs.end();
  }
  // Optional: also clear a file you created to avoid VFS error
  if (LittleFS.begin()) {
    if (LittleFS.exists("/info.dat")) LittleFS.remove("/info.dat");
    // You can also remove other app files if you wish
  }
}

// Debounce & long-press handling
// We poll buttons every 10 ms, debounce ~ 40 ms, long-press for KEY2 >= 5s.
void relay_toggle_task(void *pvParameters) {
  // Pins
  for (int i=0;i<4;i++) {
    pinMode(kRelayPins[i], OUTPUT);
    writeRelayPin(kRelayPins[i], relayState[i]);
    pinMode(kButtonPins[i], INPUT_PULLUP);
  }

  // --- Long press KEY2 detection ---
  const uint32_t LONG_PRESS_MS = 5000;
  bool lastBtn[4] = {true,true,true,true};           // true = released (because PULLUP)
  uint32_t lastChange[4] = {0,0,0,0};
  uint32_t pressStart[4] = {0,0,0,0};

  Serial.println("[relay_toggle_task] running");

  for (;;) {
    uint32_t now = millis();
    for (int i=0;i<4;i++) {
      bool level = digitalRead(kButtonPins[i]);  // HIGH=released, LOW=pressed
      if (level != lastBtn[i]) {
        lastChange[i] = now;
        lastBtn[i] = level;
      }

      // basic debounce 40ms
      if (now - lastChange[i] > 40) {
        if (level == LOW) {
          // pressed
          if (pressStart[i] == 0) pressStart[i] = now;

          // Only KEY2 has long-press action (factory reset)
          if (i == 1 && (now - pressStart[i]) >= LONG_PRESS_MS) {
            Serial.println("KEY2 long-press detected -> Factory Reset (erase relays)");
            eraseRelaysFromNVS();
            // also turn everything off for safety
            relays_write_all(false,false,false,false);
            // simple feedback (and avoid re-trigger until released)
            vTaskDelay(pdMS_TO_TICKS(1000));
            pressStart[i] = now + 0x3FFFFFFF;  // block until released
          }
        } else {
          // released
          if (pressStart[i] != 0) {
            // short-press action: toggle corresponding relay
            if (i >= 0 && i <= 3) {
              relays_toggle_by_index(i);
              Serial.printf("KEY%d short-press -> Toggle Relay %d (state=%s)\n",
                            i+1, i+1, relayState[i] ? "ON":"OFF");
            }
          }
          pressStart[i] = 0;
        }
      }
    }
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}
