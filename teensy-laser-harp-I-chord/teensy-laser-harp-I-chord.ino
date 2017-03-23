/////////////////////////////////////////////////////////////////////////
//                                                                     //
//   Laser Harp I [chord version] by Johan Berglund, March 2017        //
//                                                                     //
//   a 16 note laser harp with simple frame construction               //
//   using 16 cheap red laser modules detected by photo resistors      //
//   potentiometers for sensitivity, octave and transposition          //
//   omnichord style keyboard for chord selection                      //
//   code written for PJRC Teensy 3.2                                  //
//                                                                     //
/////////////////////////////////////////////////////////////////////////

// STILL UNTESTED VERSION aka just a sketch - need to build keyboard first :)

#define MIDI_CH 0            // MIDI channel (0 is MIDI channel 1 on DIN MIDI) 
#define VELOCITY 64          // MIDI note velocity (64 for medium velocity, 127 for maximum)
#define START_NOTE 60        // MIDI start note (middle C)
#define BEAMS 16             // number of laser beams (up to 16)

#define THR_SET_PIN A10      // pin for sensitivity adjustment potentiometer
#define OCTAVE_SET_PIN A12   // pin for octave setting potentiometer (-3 to +3 octaves)
#define TRANSP_SET_PIN A13   // pin for transposition setting potentiometer (-12 to +12 semitones)
#define CHECK_INTERVAL 5     // interval in ms for matrix check

unsigned long currentMillis = 0L;
unsigned long statusPreviousMillis = 0L;

byte colPin[11]          = {3,4,5,6,7,8,9,10,11,12,33}; // teensy digital input pins for keyboard columns
                                                        // (not using pin 2 so you can use same hardware as original version)

// column setup for omnichord style (circle of fifths)
// chord    Db, Ab, Eb, Bb,  F,  C,  G,  D,  A,  E,  B, F#
// col/note  1,  8,  3, 10,  5,  0,  7,  2,  9,  4, 11,  6
// for chromatic order, C to B, wire to columns in straight order 0 to 11

byte rowPin[3]           = {24,25,32};                  // teensy output pins for keyboard rows, where 0 is bottom row

// chord type   maj, min, 7th
// row            2,   1,   0

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
int noteNumber;             // calculated midi note number
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

void setup() {
  for (int i = 0; i < 11; i++) {
     pinMode(colPin[i],INPUT_PULLUP);
  }
    for (int i = 0; i < 3; i++) {
     pinMode(rowPin[i],OUTPUT);
     digitalWrite(rowPin[i], HIGH);
  }
  Serial1.begin(31250);  // start serial with midi baudrate 31250
  Serial1.flush();
}

void loop() {
  currentMillis = millis();
  if ((unsigned long)(currentMillis - statusPreviousMillis) >= CHECK_INTERVAL) {
    thrValue = map(analogRead(THR_SET_PIN),0,1023,60,600);                // set sensitivity for light sensors
    offThr = thrValue - 50;
    setNoteParamsPlay();                                                  // adjust note selection parameters with note-offs for previous params and note-ons for new
    for (int scanSensors = 0; scanSensors < BEAMS; scanSensors++) {       // scan matrix for changes and send note on/off accordingly
      if (!activeNote[scanSensors]){
        sensedNote = (analogRead(sensorPin[scanSensors]) > thrValue);     // if note is off, sensedNote gets high if sensor value is higher than thrValue
      } else {
        sensedNote = (analogRead(sensorPin[scanSensors]) > offThr);       // if note is on, sensedNote only goes low if sensor value goes below offThr
      }
      if (sensedNote != activeNote[scanSensors]) {
        noteNumber = START_NOTE + chord + chordNote[chordType][scanSensors] + octave + transposition;
        if ((noteNumber < 128) && (noteNumber > -1) && (chordNote[chord][scanSensors] > -1)) {    // we don't want to send midi out of range, or play silent notes
          if (sensedNote){
              usbMIDI.sendNoteOn(noteNumber, VELOCITY, MIDI_CH + 1);      // send Note On, USB MIDI
              midiSend((0x90 | MIDI_CH), noteNumber, VELOCITY);           // send Note On, DIN MIDI
          } else {
              usbMIDI.sendNoteOff(noteNumber, VELOCITY, MIDI_CH + 1);     // send note Off, USB MIDI
              midiSend((0x80 | MIDI_CH), noteNumber, VELOCITY);           // send Note Off, DIN MIDI
          }
        }  
        activeNote[scanSensors] = sensedNote;         
      }  
    }
    statusPreviousMillis = currentMillis;                                 // reset interval timing
  }
}

//  Send a three byte midi message on serial 1 (DIN MIDI) 
void midiSend(byte midistatus, byte data1, byte data2) {
  Serial1.write(midistatus);
  Serial1.write(data1);
  Serial1.write(data2);
}


void setNoteParamsPlay() {
  int rePlay = 0;
  int readChord = 0;
  int type[3] = {0,0,0};
  for (int row = 0; row < 3; row++) {     // scan keyboard rows from lowest (7th) row to highest (maj) row
    enableRow(row);                       // set current row low, others high
    for (int col = 0; col < 11; col++) {  // scan keyboard columns from lowest note to highest
      if (!digitalRead(colPin[col])) {    // is scanned pin low (active)?
        readChord = col;                  // set chord base note, high note gets priority
        type[row] = 1;                    // set row bit high
      }
    }
  }
  int readChordType = type[2]+type[1]*2+type[0]*4;              // combine rows binary to get chord type value
  if ((readChord != chord) || (readChordType != chordType)) {   // have the values changed since last scan?
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
    for (int i = 0; i < BEAMS; i++) {
       noteNumber = START_NOTE + chord + chordNote[chordType][i] + octave + transposition;
       if ((noteNumber < 128) && (noteNumber > -1) && (chordNote[chordType][i] > -1)) {      // we don't want to send midi out of range, or play silent notes
         if (activeNote[i]) {
          usbMIDI.sendNoteOff(noteNumber, VELOCITY, MIDI_CH + 1); // send Note Off, USB MIDI
          midiSend((0x80 | MIDI_CH), noteNumber, VELOCITY);       // send Note Off, DIN MIDI
         }
       }
    }
    for (int i = 0; i < BEAMS; i++) {
      noteNumber = START_NOTE + readChord + chordNote[readChordType][i] + readOctave + readTransposition;
      if ((noteNumber < 128) && (noteNumber > -1) && (chordNote[readChordType][i] > -1)) {    // we don't want to send midi out of range, or play silent notes
        if (activeNote[i]) {
          usbMIDI.sendNoteOn(noteNumber, VELOCITY, MIDI_CH + 1);  // send Note On, USB MIDI
          midiSend((0x90 | MIDI_CH), noteNumber, VELOCITY);       // send Note On, DIN MIDI
        }
      }
    }
    chord = readChord;
    chordType = readChordType;
    octave = readOctave;
    transposition = readTransposition;
  }
}

void enableRow(int row) {
  for (int rc = 0; rc < 3; rc++) {
    if (row == rc) digitalWrite(rowPin[row], LOW); else digitalWrite(rowPin[row], HIGH);
  }
}

