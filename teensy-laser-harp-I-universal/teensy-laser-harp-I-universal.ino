/////////////////////////////////////////////////////////////////////////
//                                                                     //
//   Laser Harp I [universal] by Johan Berglund, March 2017            //
//                                                                     //
//   a 16 note laser harp with simple frame construction               //
//   using 16 cheap red laser modules detected by photo resistors      //
//   potentiometers for scale, sensitivity, octave and transposition   //
//   omnichord style keyboard for chord selection                      //
//   switch for choosing chord or scale mode                           //
//   code written for PJRC Teensy 3.2                                  //
//                                                                     //
/////////////////////////////////////////////////////////////////////////


#define MIDI_CH 0            // MIDI channel (0 is MIDI channel 1 on DIN MIDI) 
#define VELOCITY 64          // MIDI note velocity (64 for medium velocity, 127 for maximum)
#define START_NOTE 60        // MIDI start note (middle C)
#define BEAMS 16             // number of laser beams (up to 16)

#define THR_SET_PIN A10      // pin for sensitivity adjustment potentiometer
#define SCALE_SET_PIN A11    // pin for scale setting potentiometer (32 scales available)
#define OCTAVE_SET_PIN A12   // pin for octave setting potentiometer (-3 to +3 octaves)
#define TRANSP_SET_PIN A13   // pin for transposition setting potentiometer (-12 to +12 semitones)
#define MODE_SW_PIN 2        // pin for mode switch (chord keyboard play/scale play)
#define LED_PIN 13           // pin for LED indicating 0 transpose

#define CHECK_INTERVAL 5     // interval in ms for sensor check

unsigned long currentMillis = 0L;
unsigned long statusPreviousMillis = 0L;

byte colPin[12]          = {3,4,5,6,7,8,9,10,11,12,32,0};  // teensy digital input pins for keyboard columns
                                                            // (not using pin 2 so you can use same hardware as original version)
                                                          
                                                            // column setup for omnichord style (circle of fifths)
                                                            // chord    Db, Ab, Eb, Bb,  F,  C,  G,  D,  A,  E,  B, F#
                                                            // col/note  1,  8,  3, 10,  5,  0,  7,  2,  9,  4, 11,  6


byte rowPin[3]           = {33,25,24};                      // teensy output pins for keyboard rows

                                                            // chord type   maj, min, 7th
                                                            // row            0,   1,   2
                                                            
                                                            // chordType 0 to 7, from binary row combinations
                                                            // 0 0 0 silent
                                                            // 0 0 1 maj
                                                            // 0 1 0 min
                                                            // 0 1 1 dim  (maj+min keys)
                                                            // 1 0 0 7th
                                                            // 1 0 1 maj7 (maj+7th keys)
                                                            // 1 1 0 m7   (min+7th keys)
                                                            // 1 1 1 aug  (maj+min+7th)

byte sensorPin[16]       = {14,15,16,17,18,19,20,21,22,23,26,27,28,29,30,31}; // teensy analog input pins

byte activeNote[16]      = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}; // keeps track of active notes
byte sensedNote;            // current reading
byte mode;                  // mode (1 is chord play, 0 is scale play)
byte readMode;
int noteNumber;             // calculated midi note number
int scale = 0;              // scale setting
int chord = 0;              // chord setting (base note of chord)
int chordType = 0;          // chord type (maj, min, 7th...)
int octave = 0;             // octave setting
int transposition = 0;      // transposition setting 

int thrValue = 120;         // sensitivity value
int offThr;

int chordNote[8][16] = {
  {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1 },  //silent
  { 0, 4, 7,12,16,19,24,28,31,36,40,43,48,52,55,60 },  //maj 
  { 0, 3, 7,12,15,19,24,27,31,36,39,43,48,51,55,60 },  //min 
  { 0, 3, 6, 9,12,15,18,21,24,27,30,33,36,39,42,45 },  //dim 
  { 0, 4, 7,10,12,16,19,22,24,28,31,34,36,40,43,46 },  //7th 
  { 0, 4, 7,11,12,16,19,23,24,28,31,35,36,40,43,47 },  //maj7
  { 0, 3, 7,10,12,15,19,22,24,27,31,34,36,39,43,46 },  //m7  
  { 0, 4, 8,12,16,20,24,28,32,36,40,44,48,52,56,60 }   //aug  
};

byte scaleNote[32][16] = {
  { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15 },  //chromatic
  { 0, 2, 4, 5, 7, 9,11,12,14,16,17,19,21,23,24,26 },  //major/ionian
  { 0, 2, 3, 5, 7, 9,10,12,14,15,17,19,21,22,24,26 },  //dorian
  { 0, 1, 3, 5, 7, 8,10,12,13,15,17,19,20,22,24,25 },  //phrygian
  { 0, 2, 4, 6, 7, 9,11,12,14,16,18,19,21,23,24,26 },  //lydian
  { 0, 2, 4, 5, 7, 9,10,12,14,16,17,19,21,22,24,26 },  //mixolydian
  { 0, 2, 3, 5, 7, 8,10,12,14,15,17,19,20,22,24,26 },  //natural minor/aeolian
  { 0, 1, 3, 5, 6, 8,10,12,13,15,17,18,20,22,24,25 },  //locrian
  { 0, 1, 4, 5, 7, 8,10,12,13,16,17,19,20,22,24,25 },  //freygish            
  { 0, 2, 3, 5, 7, 9,11,12,14,15,17,19,21,23,24,26 },  //melodic minor
  { 0, 2, 3, 5, 7, 8,11,12,14,15,17,19,20,23,24,26 },  //harmonic minor
  { 0, 3, 4, 7, 9,10,12,15,16,19,21,22,24,27,28,31 },  //major blues
  { 0, 3, 5, 6, 7,10,12,15,17,18,19,22,24,27,29,30 },  //minor blues      
  { 0, 2, 4, 7, 9,12,14,16,19,21,24,26,28,31,33,36 },  //major pentatonic
  { 0, 3, 5, 7,10,12,15,17,19,22,24,27,29,31,34,36 },  //minor pentatonic
  { 0, 2, 3, 5, 6, 8, 9,11,12,14,15,17,18,20,23,24 },  //diminish
  { 0, 1, 3, 4, 6, 7, 9,10,12,13,15,16,18,19,21,22 },  //combi diminish    
  { 0, 1, 4, 5, 7, 8,11,12,13,16,17,19,20,23,24,25 },  //raga bhairav
  { 0, 1, 4, 6, 7, 9,11,12,13,16,18,19,21,23,24,25 },  //raga gamansrama
  { 0, 1, 3, 6, 7, 8,11,12,13,15,18,19,20,23,24,25 },  //raga todi
  { 0, 1, 3, 4, 5, 7, 8,10,12,13,15,16,17,19,20,22 },  //spanish
  { 0, 2, 3, 6, 7, 8,11,12,14,15,18,19,20,23,24,26 },  //gypsy
  { 0, 2, 4, 5, 6, 8,10,12,14,16,17,18,20,22,24,26 },  //arabian
  { 0, 2, 5, 7,10,12,14,17,19,22,24,26,29,31,34,36 },  //egyptian
  { 0, 2, 3, 7, 9,12,14,15,19,21,24,26,27,31,33,36 },  //hawaiian
  { 0, 1, 3, 7, 8,12,13,15,19,20,24,25,27,31,32,36 },  //bali pelog
  { 0, 1, 5, 7, 8,12,13,17,19,20,24,25,29,31,32,36 },  //japan
  { 0, 4, 5, 7,11,12,16,17,19,23,24,28,29,31,35,36 },  //ryukyu
  { 0, 2, 4, 6, 8,10,12,14,16,18,20,22,24,26,28,30 },  //wholetone
  { 0, 3, 4, 7, 8,11,12,15,16,19,20,23,24,27,28,31 },  //augmented
  { 0, 2, 4, 6,10,12,14,16,18,22,24,26,28,30,34,36 },  //prometheus
  { 0, 1, 4, 6, 7,10,12,13,16,18,19,22,24,25,28,30 }   //tritone                                
};

// SETUP

void setup() {
  pinMode(MODE_SW_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  for (int i = 0; i < 12; i++) {
     pinMode(colPin[i], INPUT_PULLUP);
  }
    for (int i = 0; i < 3; i++) {
     pinMode(rowPin[i], OUTPUT);
     digitalWrite(rowPin[i], LOW);
  }
}

// MAIN LOOP
void loop() {
  currentMillis = millis();
  if ((unsigned long)(currentMillis - statusPreviousMillis) >= CHECK_INTERVAL) {
    thrValue = map(analogRead(THR_SET_PIN),0,1023,60,600);                // set sensitivity for light sensors
    offThr = thrValue - 50;
    readMode = digitalRead(MODE_SW_PIN);
    if (readMode != mode) {
      allNotesOff();
      mode = readMode;
    }
    if (mode) {
      setNoteParamsChord();                                               // adjust note selection parameters with note-offs for previous params and note-ons for new
      playChords();                                                       // read sensors, calculate notes and send midi data (chords)
    } else {
      setNoteParamsScale();                                               // adjust note selection parameters with note-offs for previous params and note-ons for new
      playScales();                                                       // read sensors, calculate notes and send midi data (scales)
    }
    statusPreviousMillis = currentMillis;                                 // reset interval timing
  }
}
// END MAIN LOOP

void playChords() {
  for (int scanSensors = 0; scanSensors < BEAMS; scanSensors++) {       // scan sensors for changes and send note on/off accordingly
    if (!activeNote[scanSensors]){
      sensedNote = (analogRead(sensorPin[scanSensors]) > thrValue);     // if note is off, sensedNote gets high if sensor value is higher than thrValue
    } else {
      sensedNote = (analogRead(sensorPin[scanSensors]) > offThr);       // if note is on, sensedNote only goes low if sensor value goes below offThr
    }
    if (sensedNote != activeNote[scanSensors]) {
      noteNumber = START_NOTE + chord + chordNote[chordType][scanSensors] + octave;
      if ((noteNumber < 128) && (noteNumber > -1) && (chordNote[chordType][scanSensors] > -1)) {    // we don't want to send midi out of range or play silent notes
        if (sensedNote){
            usbMIDI.sendNoteOn(noteNumber, VELOCITY, MIDI_CH + 1);      // send Note On, USB MIDI
        } else {
            usbMIDI.sendNoteOff(noteNumber, VELOCITY, MIDI_CH + 1);     // send note Off, USB MIDI
        }
      }  
      activeNote[scanSensors] = sensedNote;         
    }  
  }
}

void playScales() {
  for (int scanSensors = 0; scanSensors < BEAMS; scanSensors++) {     // scan sensors for changes and send note on/off accordingly
    if (!activeNote[scanSensors]){
      sensedNote = (analogRead(sensorPin[scanSensors]) > thrValue);   // if note is off, sensedNote gets high if sensor value is higher than thrValue
    } else {
      sensedNote = (analogRead(sensorPin[scanSensors]) > offThr);     // if note is on, sensedNote only goes low if sensor value goes below offThr
    }
    if (sensedNote != activeNote[scanSensors]) {
      noteNumber = START_NOTE + scaleNote[scale][scanSensors] + octave + transposition;
      if ((noteNumber < 128) && (noteNumber > -1)) {                  // we don't want to send midi out of range
        if (sensedNote){
            usbMIDI.sendNoteOn(noteNumber, VELOCITY, MIDI_CH + 1);    // send Note On, USB MIDI
        } else {
            usbMIDI.sendNoteOff(noteNumber, VELOCITY, MIDI_CH + 1);   // send note Off, USB MIDI
        }
      }  
      activeNote[scanSensors] = sensedNote;         
    } 
  }
}

// Check chord keyboard and potentiometers, if changed shut off any active notes and replay with new settings
void setNoteParamsChord() {
  int rePlay = 0;
  int readChord = 0;
  int readChordType = 0;
  int readTransposition = map(analogRead(TRANSP_SET_PIN), 0, 1023, 6, 19); // get positive transposition values with C (12) in the middle
  if (readTransposition != transposition) {
    rePlay = 1;
    transposition = readTransposition;
  }
  for (int row = 0; row < 3; row++) {         // scan keyboard rows from (7th) row to (maj) row
    enableRow(row);                           // set current row low
    for (int col = 0; col < 12; col++) {      // scan keyboard columns from lowest note to highest
      if (!digitalRead(colPin[col])) {        // is scanned pin low (active)?
        readChord = (col+transposition*7)%12; // set chord base note, high note gets priority, transpose in fifths
        readChordType |= (1 << row);          // set row bit in chord type
      }
    }
  }
  if ((readChord != chord) || (readChordType != chordType)) {   // have the values changed since last scan?
    rePlay = 1;
  }  
  int readOctave = map(analogRead(OCTAVE_SET_PIN), 0, 1023, -4, 3) * 12;
  if (readOctave != octave) {
    rePlay = 1;
  }
  if (rePlay) {
    if (transposition%12 == 0) digitalWrite(LED_PIN, HIGH); else digitalWrite(LED_PIN, LOW); // LED lit if no transposition
    for (int i = 0; i < BEAMS; i++) {
       noteNumber = START_NOTE + chord + chordNote[chordType][i] + octave;
       if ((noteNumber < 128) && (noteNumber > -1) && (chordNote[chordType][i] > -1)) {      // we don't want to send midi out of range or play silent notes
         if (activeNote[i]) {
          usbMIDI.sendNoteOff(noteNumber, VELOCITY, MIDI_CH + 1); // send Note Off, USB MIDI
         }
       }
    }
    for (int i = 0; i < BEAMS; i++) {
      noteNumber = START_NOTE + readChord + chordNote[readChordType][i] + readOctave;
      if ((noteNumber < 128) && (noteNumber > -1) && (chordNote[readChordType][i] > -1)) {    // we don't want to send midi out of range or play silent notes
        if (activeNote[i]) {
          usbMIDI.sendNoteOn(noteNumber, VELOCITY, MIDI_CH + 1);  // send Note On, USB MIDI
        }
      }
    }
    chord = readChord;
    chordType = readChordType;
    octave = readOctave;
  }
}

// Check potentiometers, if changed shut off any active notes and replay with new settings
void setNoteParamsScale() {
  int rePlay = 0;
  int readScale = (analogRead(SCALE_SET_PIN) / 32);
  if (readScale != scale) {
    rePlay = 1;
  }
  int readOctave = map(analogRead(OCTAVE_SET_PIN), 0, 1023, -4, 3) * 12;
  if (readOctave != octave) {
    rePlay = 1;
  }
  int readTransposition = map(analogRead(TRANSP_SET_PIN), 0, 1023, -12, 12);
  if (readTransposition != transposition) {
    rePlay = 1;
  }
  if (rePlay) {
    if (readTransposition == 0) digitalWrite(LED_PIN, HIGH); else digitalWrite(LED_PIN, LOW); // LED lit if no transposition
    for (int i = 0; i < BEAMS; i++) {
       noteNumber = START_NOTE + scaleNote[scale][i] + octave + transposition;
       if ((noteNumber < 128) && (noteNumber > -1)) {             // we don't want to send midi out of range
         if (activeNote[i]) {
          usbMIDI.sendNoteOff(noteNumber, VELOCITY, MIDI_CH + 1); // send Note Off, USB MIDI
         }
       }
    }
    for (int i = 0; i < BEAMS; i++) {
      noteNumber = START_NOTE + scaleNote[readScale][i] + readOctave + readTransposition;
      if ((noteNumber < 128) && (noteNumber > -1)) {              // we don't want to send midi out of range
        if (activeNote[i]) {
          usbMIDI.sendNoteOn(noteNumber, VELOCITY, MIDI_CH + 1);  // send Note On, USB MIDI
        }
      }
    }
    scale = readScale;
    octave = readOctave;
    transposition = readTransposition;
  }
}


// Set selected row low (active), others to Hi-Z
void enableRow(int row) {
  for (int rc = 0; rc < 3; rc++) {
    if (row == rc) {
      pinMode(rowPin[rc], OUTPUT);
      digitalWrite(rowPin[rc], LOW);
    } else {
      digitalWrite(rowPin[rc], HIGH);
      pinMode(rowPin[rc], INPUT); // Put to Hi-Z for safety against shorts
    }
  }
  delayMicroseconds(30); // wait before reading ports (let ports settle after changing)
}

// Send note off for all midi notes, reset active notes
void allNotesOff() {
 for (int i = 0; i <128; i++) {
  usbMIDI.sendNoteOff(i, VELOCITY, MIDI_CH + 1); // send Note Off, USB MIDI
 }
 for (int i = 0; i <BEAMS; i++) {
  activeNote[i] = 0;
 }
  
}

