#include "pitches.h"
#include <Arduino.h>
#define BUZZZER_PIN 2

// Megalovania - Extended version with intro and main riff
int melody[] = {
  // Intro
  NOTE_D4, NOTE_D4, NOTE_D5, 0, NOTE_A4, 0, NOTE_GS4, 0,
  NOTE_G4, 0, NOTE_F4, 0, NOTE_D4, NOTE_F4, NOTE_G4,
  
  NOTE_C4, NOTE_C4, NOTE_D5, 0, NOTE_A4, 0, NOTE_GS4, 0,
  NOTE_G4, 0, NOTE_F4, 0, NOTE_D4, NOTE_F4, NOTE_G4,
  
  NOTE_B3, NOTE_B3, NOTE_D5, 0, NOTE_A4, 0, NOTE_GS4, 0,
  NOTE_G4, 0, NOTE_F4, 0, NOTE_D4, NOTE_F4, NOTE_G4,
  
  NOTE_AS3, NOTE_AS3, NOTE_D5, 0, NOTE_A4, 0, NOTE_GS4, 0,
  NOTE_G4, 0, NOTE_F4, 0, NOTE_D4, NOTE_F4, NOTE_G4,
  
  // Main Riff
  NOTE_D4, NOTE_D4, NOTE_D5, 0, NOTE_A4, 0, NOTE_GS4, 0,
  NOTE_G4, 0, NOTE_F4, 0, NOTE_D4, NOTE_F4, NOTE_G4,
  
  NOTE_C4, NOTE_C4, NOTE_D5, 0, NOTE_A4, 0, NOTE_GS4, 0,
  NOTE_G4, 0, NOTE_F4, 0, NOTE_D4, NOTE_F4, NOTE_G4,
  
  NOTE_B3, NOTE_B3, NOTE_D5, 0, NOTE_A4, 0, NOTE_GS4, 0,
  NOTE_G4, 0, NOTE_F4, 0, NOTE_D4, NOTE_F4, NOTE_G4,
  
  NOTE_AS3, NOTE_AS3, NOTE_D5, 0, NOTE_A4, 0, NOTE_GS4, 0,
  NOTE_G4, 0, NOTE_F4, 0, NOTE_D4, NOTE_F4, NOTE_G4,
  
  // Melody section
  NOTE_D5, NOTE_D5, NOTE_D5, NOTE_D5, NOTE_D5, NOTE_D5, NOTE_D5, NOTE_A4,
  NOTE_GS4, NOTE_G4, NOTE_F4, NOTE_D4, NOTE_F4, NOTE_G4, 0, 0,
  
  NOTE_C5, NOTE_C5, NOTE_C5, NOTE_C5, NOTE_C5, NOTE_C5, NOTE_D5, 0,
  NOTE_C5, 0, NOTE_AS4, 0, NOTE_A4, 0, 0, 0,
  
  NOTE_B4, NOTE_B4, NOTE_B4, NOTE_B4, NOTE_B4, NOTE_B4, NOTE_B4, NOTE_A4,
  NOTE_GS4, NOTE_G4, NOTE_F4, NOTE_D4, NOTE_F4, NOTE_G4, 0, 0,
  
  NOTE_AS4, NOTE_AS4, NOTE_AS4, NOTE_AS4, NOTE_AS4, NOTE_AS4, NOTE_AS4, NOTE_A4,
  NOTE_GS4, NOTE_G4, NOTE_F4, NOTE_D4, NOTE_F4, NOTE_G4, 0, 0
};

int noteDurations[] = {
  // Intro
  16, 16, 8, 16, 8, 16, 8, 16,
  8, 16, 8, 16, 8, 8, 8,
  
  16, 16, 8, 16, 8, 16, 8, 16,
  8, 16, 8, 16, 8, 8, 8,
  
  16, 16, 8, 16, 8, 16, 8, 16,
  8, 16, 8, 16, 8, 8, 8,
  
  16, 16, 8, 16, 8, 16, 8, 16,
  8, 16, 8, 16, 8, 8, 8,
  
  // Main Riff
  16, 16, 8, 16, 8, 16, 8, 16,
  8, 16, 8, 16, 8, 8, 8,
  
  16, 16, 8, 16, 8, 16, 8, 16,
  8, 16, 8, 16, 8, 8, 8,
  
  16, 16, 8, 16, 8, 16, 8, 16,
  8, 16, 8, 16, 8, 8, 8,
  
  16, 16, 8, 16, 8, 16, 8, 16,
  8, 16, 8, 16, 8, 8, 8,
  
  // Melody section
  16, 16, 16, 16, 16, 16, 8, 8,
  8, 8, 8, 8, 8, 8, 8, 8,
  
  16, 16, 16, 16, 16, 16, 8, 8,
  8, 8, 8, 8, 8, 8, 8, 8,
  
  16, 16, 16, 16, 16, 16, 8, 8,
  8, 8, 8, 8, 8, 8, 8, 8,
  
  16, 16, 16, 16, 16, 16, 8, 8,
  8, 8, 8, 8, 8, 8, 8, 8
};

void setup() {
  // Play the entire melody
  for (int thisNote = 0; thisNote < sizeof(melody) / sizeof(melody[0]); thisNote++) {
    int noteDuration = 1000 / noteDurations[thisNote];
    
    if (melody[thisNote] == 0) {
      // Rest
      delay(noteDuration);
    } else {
      tone(BUZZZER_PIN, melody[thisNote], noteDuration);
      int pauseBetweenNotes = noteDuration * 1.30;
      delay(pauseBetweenNotes);
      noTone(BUZZZER_PIN);
    }
  }
}

void loop() {
  // Empty - plays once on startup
  // Move setup() code here if you want it to loop continuously
}