#include <IRremote.h>

const int IR_RECEIVE_PIN = 15; // G15 IR_Receive Pin
const int BUZZER_PIN = 13; // G13 Passive Buzzer Pin 

// Frequenzen von C4 bis D5 (C-D-E-F-G-A-H-C-D)
int notes[] = {261, 294, 330, 349, 392, 440, 494, 523, 587}; // C4–D5

// LED-Pins passend zu den 8 Tönen
const int NUM_LEDS = 9;
int ledPins[NUM_LEDS] = {16, 17, 18, 19, 21, 22, 23, 25, 26}; // LED Pins G16, G17, G18, G19, G21, G22, G23, G25, G26 

// IR-Codes (Taste 0–9)
uint64_t irCodes[10] = {
  0xE916FF00, // Taste 0 Happy Birthday
  0xF30CFF00, // Taste 1 C4
  0xE718FF00, // Taste 2 D4
  0xA15EFF00, // Taste 3 E4
  0xF708FF00, // Taste 4 F4
  0xE31CFF00, // Taste 5 G4
  0xA55AFF00, // Taste 6 A4
  0xBD42FF00, // Taste 7 H4
  0xAD52FF00, // Taste 8 C5
  0xB54AFF00  // Taste 9 D5
};

int currentNote = -1;
unsigned long lastReceiveTime = 0;

void setup() {
  Serial.begin(115200);
  IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);

  // Buzzer konfigurieren
  ledcAttach(BUZZER_PIN, 1000, 8);

  // LEDs konfigurieren
  for (int i = 0; i < NUM_LEDS; i++) {
    pinMode(ledPins[i], OUTPUT);
    digitalWrite(ledPins[i], LOW);
  }
}

void playTone(int frequency, int ledIndex) {
  ledcWriteTone(BUZZER_PIN, frequency);
  if (ledIndex >= 0 && ledIndex < NUM_LEDS) {
    digitalWrite(ledPins[ledIndex], HIGH);
  }
}

void stopTone() {
  ledcWriteTone(BUZZER_PIN, 0);
  if (currentNote >= 0 && currentNote < NUM_LEDS) {
    digitalWrite(ledPins[currentNote], LOW);
  }
  currentNote = -1;
}

void playHappyBirthday() {
int melody[] = {
  261, 261, 294, 261, 349, 330,     // Happy birthday to you
  261, 261, 294, 261, 392, 349,     // Happy birthday to you
  261, 261, 523, 440, 349, 330,     // Happy birthday dear [Name]
  294, 349, 349, 330, 261, 294, 261   // Happy birthday to you
}; 

  int durations[] = {
    250, 250, 500, 500, 500, 1000,
    250, 250, 500, 500, 500, 1000,
    250, 250, 500, 500, 500, 500, 1000,
    250, 250, 500, 500, 500, 1000
  };

  for (int i = 0; i < sizeof(melody) / sizeof(int); i++) {
    int freq = melody[i];
    int ledIndex = -1;

    // Finde passende LED zur Frequenz
    for (int j = 0; j < NUM_LEDS; j++) {
      if (notes[j] == freq) {
        ledIndex = j;
        break;
      }
    }

    if (ledIndex >= 0) {
      digitalWrite(ledPins[ledIndex], HIGH);
    }

    playTone(freq, -1); // -1 = keine automatische LED
    delay(durations[i]);

    stopTone();

    if (ledIndex >= 0) {
      digitalWrite(ledPins[ledIndex], LOW);
    }

    delay(50);
  }
}

void loop() {
  if (IrReceiver.decode()) {
    uint64_t code = IrReceiver.decodedIRData.decodedRawData;
    Serial.print("Empfangen: ");
    Serial.println(code, HEX);

    // Taste 0 = Happy Birthday
    if (code == irCodes[0]) {
      Serial.println("→ Happy Birthday");
      playHappyBirthday();
    } else {
      // Taste 1–9 = Einzelton
      for (int i = 1; i <= 9; i++) {
        if (code == irCodes[i]) {
          Serial.print("Treffer bei Taste: ");
          Serial.println(i);

          if (currentNote != i - 1) {
            stopTone(); // Vorherige LED ausschalten
            playTone(notes[i - 1], i - 1);
            currentNote = i - 1;
          }
          break;
        }
      }
      lastReceiveTime = millis();
    }

    IrReceiver.resume();
  }

  // Timeout: Ton stoppen, wenn Taste losgelassen wurde (>200ms)
  if (currentNote != -1 && millis() - lastReceiveTime > 200) {
    stopTone();
  }
}