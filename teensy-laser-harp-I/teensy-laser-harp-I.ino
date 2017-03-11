/////////////////////////////////////////////////////////////////////////
//                                                                     //
//   Laser Harp I by Johan Berglund, January 2017                      //
//                                                                     //
//   a 16 note laser harp with simple frame construction               //
//   using 16 cheap red laser modules detected by photo resistors      //
//   potentiometers for sensitivity, scale, octave and transposition   //
//   code written for PJRC Teensy 3.2                                  //
//                                                                     //
/////////////////////////////////////////////////////////////////////////

#define MIDI_CH 0            // MIDI channel (0 is MIDI channel 1 on DIN MIDI) 
#define VELOCITY 64          // MIDI note velocity (64 for medium velocity, 127 for maximum)
#define START_NOTE 60        // MIDI start note (middle C)
#define BEAMS 16             // number of laser beams (up to 16)

#define THR_SET_PIN A10      // pin for sensitivity adjustment potentiometer
#define SCALE_SET_PIN A11    // pin for scale setting potentiometer (32 scales available)
#define SCALE_SW_PIN 2       // pin for switch dividing scale setting in two (16 upper, 16 lower)
#define OCTAVE_SET_PIN A12   // pin for octave setting potentiometer (-3 to +3 octaves)
#define TRANSP_SET_PIN A13   // pin for transposition setting potentiometer (-12 to +12 semitones)
#define CHECK_INTERVAL 5     // interval in ms for matrix check

unsigned long currentMillis = 0L;
unsigned long statusPreviousMillis = 0L;

byte sensorPin[16]       = {14,15,16,17,18,19,20,21,22,23,26,27,28,29,30,31}; // teensy analog input pins
byte activeNote[16]      = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}; // keeps track of active notes
byte sensedNote;            // current reading
int noteNumber;             // calculated midi note number
byte noActiveNote = 1;      // flag for avoiding note/scale parameter changes during play (eliminate note hangs)
int scale = 1;              // scale setting
int octave = 0;             // octave setting
int transposition = 0;      // transposition setting

int thrValue = 120;         // sensitivity value
int offThr;

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

void setup() {
  pinMode(SCALE_SW_PIN, INPUT_PULLUP);
  Serial1.begin(31250);  // start serial with midi baudrate 31250
  Serial1.flush();
}

void loop() {
  currentMillis = millis();
  if ((unsigned long)(currentMillis - statusPreviousMillis) >= CHECK_INTERVAL) {
    thrValue = map(analogRead(THR_SET_PIN),0,1023,60,600); // set sensitivity for light sensors
    offThr = thrValue - 50;
    if (noActiveNote) {                                   
      setNoteParams();                                    // adjust note selection parameters directly when no notes are playing
    } else {
      setNoteParamsPlay();                                // adjust note selection parameters with note-offs for previous params and note-ons for new
    }
    noActiveNote = 1;                                     // before each scan, assume there's no note playing
    for (int scanSensors = 0; scanSensors < BEAMS; scanSensors++) {   // scan matrix for changes and send note on/off accordingly
      if (!activeNote[scanSensors]){
        sensedNote = (analogRead(sensorPin[scanSensors]) > thrValue);   // if note is off, sensedNote gets high if sensor value is higher than thrValue
      } else {
        sensedNote = (analogRead(sensorPin[scanSensors]) > offThr);   // if note is on, sensedNote only goes low if sensor value goes below offThr
      }
      if (sensedNote != activeNote[scanSensors]) {
        noteNumber = START_NOTE + scaleNote[scale][scanSensors] + octave + transposition;
        if ((noteNumber < 128) && (noteNumber > -1)) {                // we don't want to send midi out of range
          if (sensedNote){
              usbMIDI.sendNoteOn(noteNumber, VELOCITY, MIDI_CH + 1);  // send Note On, USB MIDI
              midiSend((0x90 | MIDI_CH), noteNumber, VELOCITY);       // send Note On, DIN MIDI
          } else {
              usbMIDI.sendNoteOff(noteNumber, VELOCITY, MIDI_CH + 1); // send note Off, USB MIDI
              midiSend((0x80 | MIDI_CH), noteNumber, VELOCITY);       // send Note Off, DIN MIDI
          }
        }  
        activeNote[scanSensors] = sensedNote;         
      } 
    if (activeNote[scanSensors]) noActiveNote = 0;           // if there is a note active, set flag false  
    }
    statusPreviousMillis = currentMillis;                // reset interval timing
  }
}

//  Send a three byte midi message on serial 1 (DIN MIDI) 
  void midiSend(byte midistatus, byte data1, byte data2) {
  Serial1.write(midistatus);
  Serial1.write(data1);
  Serial1.write(data2);
}


void setNoteParams() {
  scale = (analogRead(SCALE_SET_PIN) / 64) + (digitalRead(SCALE_SW_PIN) * 16);
  octave = map(analogRead(OCTAVE_SET_PIN), 0, 1023, -4, 3) * 12;
  transposition = map(analogRead(TRANSP_SET_PIN), 0, 1023, -12, 12);
}

void setNoteParamsPlay() {
  int rePlay = 0;
  int readScale = (analogRead(SCALE_SET_PIN) / 64) + (digitalRead(SCALE_SW_PIN) * 16);
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
    for (int i = 0; i < BEAMS; i++) {
       noteNumber = START_NOTE + scaleNote[scale][i] + octave + transposition;
       if ((noteNumber < 128) && (noteNumber > -1)) {             // we don't want to send midi out of range
         if (activeNote[i]) {
          usbMIDI.sendNoteOff(noteNumber, VELOCITY, MIDI_CH + 1); // send Note Off, USB MIDI
          midiSend((0x80 | MIDI_CH), noteNumber, VELOCITY);       // send Note Off, DIN MIDI
         }
       }
    }
    for (int i = 0; i < BEAMS; i++) {
      noteNumber = START_NOTE + scaleNote[readScale][i] + readOctave + readTransposition;
      if ((noteNumber < 128) && (noteNumber > -1)) {              // we don't want to send midi out of range
        if (activeNote[i]) {
          usbMIDI.sendNoteOn(noteNumber, VELOCITY, MIDI_CH + 1);  // send Note On, USB MIDI
          midiSend((0x90 | MIDI_CH), noteNumber, VELOCITY);       // send Note On, DIN MIDI
        }
      }
    }
    scale = readScale;
    octave = readOctave;
    transposition = readTransposition;
  }
}

