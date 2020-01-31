/////////////////////////////////////////////////////////////////////////
//                                                                     //
//   Laser Harp S by Johan Berglund, January 2018                     //
//                                                                     //
//   a framed 16 notes laser harp with programmable chord buttons      //
//   using cheap red laser modules detected by photo resistors         //
//   code written for PJRC Teensy 3.2                                  //
//                                                                     //
/////////////////////////////////////////////////////////////////////////

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <EEPROM.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define VELOCITY 64          // MIDI note velocity (64 for medium velocity, 127 for maximum)
#define BEAMS 16             // number of laser beams (up to 16)

#define SW0_PIN 2
#define SW0_SNS_PIN 3
#define SW1_PIN 4
#define SW2_PIN 5
#define SW3_PIN 6
#define MPX_EN_PIN 16
#define SENSOR_PIN A3
#define THR_SET_PIN A0       // pin for sensitivity adjustment potentiometer
#define D_PIN 9              // pin for down tact switch
#define E_PIN 10              // pin for enter tact switch
#define U_PIN 11              // pin for up tact switch
#define M_PIN 12              // pin for menu/back tact switch
//#define EN_LO_PIN 31         // gnd for enable sw
//#define EN_READ_PIN 32       // pin for enable, active low
#define LED_PIN 13           // Note on indicator LED
#define CHECK_INTERVAL 10    // interval in ms for matrix check

#define ATTACK 1             // tone attack value in ms
#define RELEASE 1000         // tone release value in ms

// EEPROM addresses for settings
#define VERSION_ADDR 0
#define TRANSP1_ADDR 2
#define TRANSP2_ADDR 4
#define TRANSP3_ADDR 6
#define TRANSP4_ADDR 8
#define SCALE1_ADDR 10
#define SCALE2_ADDR 12
#define SCALE3_ADDR 14
#define SCALE4_ADDR 16
#define DRONE1_ADDR 18
#define DRONE2_ADDR 20
#define FRMIDI_ADDR 22
#define DRMIDI_ADDR 24

//"factory" values for settings
#define VERSION 3
#define TRANSP1_FACTORY 36
#define TRANSP2_FACTORY 41
#define TRANSP3_FACTORY 43
#define TRANSP4_FACTORY 45
#define SCALE1_FACTORY 0
#define SCALE2_FACTORY 0
#define SCALE3_FACTORY 0
#define SCALE4_FACTORY 1
#define DRONE1_FACTORY 0
#define DRONE2_FACTORY 7
#define FRMIDI_FACTORY 0
#define DRMIDI_FACTORY 2

#define TRANSP1 0
#define TRANSP2 1
#define TRANSP3 2
#define TRANSP4 3
#define SCALE1  4
#define SCALE2  5
#define SCALE3  6
#define SCALE4  7


// GUItool: begin automatically generated code
AudioSynthWaveformSine   sine1;          //xy=105,37
AudioSynthWaveformSine   sine2;          //xy=110,86
AudioSynthWaveformSine   sine3;          //xy=117,136
AudioSynthWaveformSine   sine4;          //xy=125,185
AudioSynthWaveformSine   sine5;          //xy=134,233
AudioSynthWaveformSine   sine6;          //xy=147,285
AudioSynthWaveformSine   sine7;          //xy=160,336
AudioSynthWaveformSine   sine8;          //xy=170,385
AudioSynthWaveformSine   sine9;          //xy=174.28571701049805,429.99999713897705
AudioSynthWaveformSine   sine10;         //xy=175.71427154541016,475.7142734527588
AudioSynthWaveformSine   sine14;         //xy=175.7142791748047,648.5713977813721
AudioSynthWaveformSine   sine15;         //xy=175.71427154541016,691.4285669326782
AudioSynthWaveformSine   sine16;         //xy=175.71429061889648,734.2857427597046
AudioSynthWaveformSine   sine12;         //xy=177.1428451538086,559.9999980926514
AudioSynthWaveformSine   sine13;         //xy=177.14286422729492,602.8571224212646
AudioSynthWaveformSine   sine11;         //xy=178.5714340209961,517.1428470611572
AudioEffectFade          fade1;          //xy=303,61
AudioEffectFade          fade2;          //xy=310,104
AudioEffectFade          fade3;          //xy=319,149
AudioEffectFade          fade4;          //xy=337,198
AudioEffectFade          fade5;          //xy=351,243
AudioEffectFade          fade6;          //xy=366,292
AudioEffectFade          fade7;          //xy=379,334
AudioEffectFade          fade8;          //xy=395,380
AudioEffectFade          fade9;          //xy=400,430.0000147819519
AudioEffectFade          fade12;         //xy=405.7142639160156,559.9999904632568
AudioEffectFade          fade14;         //xy=405.7142906188965,642.8571138381958
AudioEffectFade          fade15;         //xy=405.7142677307129,679.9999866485596
AudioEffectFade          fade10;         //xy=407.1428680419922,471.4285888671875
AudioEffectFade          fade16;         //xy=407.1428565979004,725.7142553329468
AudioEffectFade          fade11;         //xy=408.57142639160156,515.7143020629883
AudioEffectFade          fade13;         //xy=409.99999237060547,599.9999980926514
AudioMixer4              mixer1;         //xy=557,137
AudioMixer4              mixer2;         //xy=597,239
AudioMixer4              mixer3;         //xy=620.0000228881836,501.4285583496094
AudioMixer4              mixer4;         //xy=621.4286003112793,618.5713901519775
AudioMixer4              mixer5;         //xy=849.9999923706055,355.7142791748047
AudioOutputAnalog        dac1;           //xy=1097.142857142857,287.1428571428571
AudioOutputUSB           usb1;           //xy=1099.9999618530273,419.99999713897705
AudioConnection          patchCord1(sine1, fade1);
AudioConnection          patchCord2(sine2, fade2);
AudioConnection          patchCord3(sine3, fade3);
AudioConnection          patchCord4(sine4, fade4);
AudioConnection          patchCord5(sine5, fade5);
AudioConnection          patchCord6(sine6, fade6);
AudioConnection          patchCord7(sine7, fade7);
AudioConnection          patchCord8(sine8, fade8);
AudioConnection          patchCord9(sine9, fade9);
AudioConnection          patchCord10(sine10, fade10);
AudioConnection          patchCord11(sine14, fade14);
AudioConnection          patchCord12(sine15, fade15);
AudioConnection          patchCord13(sine16, fade16);
AudioConnection          patchCord14(sine12, fade12);
AudioConnection          patchCord15(sine13, fade13);
AudioConnection          patchCord16(sine11, fade11);
AudioConnection          patchCord17(fade1, 0, mixer1, 0);
AudioConnection          patchCord18(fade2, 0, mixer1, 1);
AudioConnection          patchCord19(fade3, 0, mixer1, 2);
AudioConnection          patchCord20(fade4, 0, mixer1, 3);
AudioConnection          patchCord21(fade5, 0, mixer2, 0);
AudioConnection          patchCord22(fade6, 0, mixer2, 1);
AudioConnection          patchCord23(fade7, 0, mixer2, 2);
AudioConnection          patchCord24(fade8, 0, mixer2, 3);
AudioConnection          patchCord25(fade9, 0, mixer3, 0);
AudioConnection          patchCord26(fade12, 0, mixer3, 3);
AudioConnection          patchCord27(fade14, 0, mixer4, 1);
AudioConnection          patchCord28(fade15, 0, mixer4, 2);
AudioConnection          patchCord29(fade10, 0, mixer3, 1);
AudioConnection          patchCord30(fade16, 0, mixer4, 3);
AudioConnection          patchCord31(fade11, 0, mixer3, 2);
AudioConnection          patchCord32(fade13, 0, mixer4, 0);
AudioConnection          patchCord33(mixer1, 0, mixer5, 0);
AudioConnection          patchCord34(mixer2, 0, mixer5, 1);
AudioConnection          patchCord35(mixer3, 0, mixer5, 2);
AudioConnection          patchCord36(mixer4, 0, mixer5, 3);
AudioConnection          patchCord37(mixer5, 0, usb1, 0);
AudioConnection          patchCord38(mixer5, 0, usb1, 1);
AudioConnection          patchCord39(mixer5, dac1);
// GUItool: end automatically generated code


// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastDebounceTime = 0;         // the last time the output pin was toggled
unsigned long debounceDelay = 20;           // the debounce time; increase if the output flickers
unsigned long buttonRepeatTime = 0;
unsigned long buttonPressedTime = 0;
unsigned long buttonRepeatInterval = 50;
unsigned long buttonRepeatDelay = 400;
unsigned long currentMillis = 0L;
unsigned long statusPreviousMillis = 0L;

byte mpxPin[4]             = {23,22,21,20};                       // multiplexer control pins S0-S3
//byte presencePin[4]      = {24,25,26,27};                       // presence sense pins for laser frames 
//byte sensorPin[4]        = {14,15,16,17};                       // teensy analog input pins
//byte frameAvail[4]       = {1,0,0,0};
byte activeNote[16]        = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};   // keeps track of active notes
byte sensedNote;                                                  // current reading
int noteNumber;                                                   // calculated midi note number
int switchReading;
int oldSwitchReading;

float midiToFreq[128];         // for storing pre calculated frequencies for note numbers

int minVal[8] = {24,24,24,24,0,0,0,0};
int maxVal[8] = {107,107,107,107,38,38,38,38};

unsigned short setting[8];
unsigned short oldSetting[8];
unsigned short drone[2];
unsigned short frameMIDIch;
unsigned short droneMIDIch;

byte item = 0;
byte replay = 0;

int deumButtons = 0;
int lastDeumButtons = 0;
int deumButtonState = 0;
byte buttonPressedAndNotUsed = 0;

int lasersOn;
int lastLasersOn;

int octave = 0;             // octave

int thrValue = 120;         // sensitivity value
int offThr;

int scaleNote[39][16] = {
  { 0, 4, 7,12,16,19,24,28,31,36,40,43,48,52,55,60 },  //maj
  { 0, 3, 7,12,15,19,24,27,31,36,39,43,48,51,55,60 },  //min 
  { 0, 3, 6, 9,12,15,18,21,24,27,30,33,36,39,42,45 },  //dim 
  { 0, 4, 7,10,12,16,19,22,24,28,31,34,36,40,43,46 },  //7th 
  { 0, 4, 7,11,12,16,19,23,24,28,31,35,36,40,43,47 },  //maj7
  { 0, 3, 7,10,12,15,19,22,24,27,31,34,36,39,43,46 },  //m7  
  { 0, 4, 8,12,16,20,24,28,32,36,40,44,48,52,56,60 },  //aug  
  { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15 },  //chromatic - chro
  { 0, 2, 4, 5, 7, 9,11,12,14,16,17,19,21,23,24,26 },  //major/ionian - ioni
  { 0, 2, 3, 5, 7, 9,10,12,14,15,17,19,21,22,24,26 },  //dorian - dori
  { 0, 1, 3, 5, 7, 8,10,12,13,15,17,19,20,22,24,25 },  //phrygian -phry
  { 0, 2, 4, 6, 7, 9,11,12,14,16,18,19,21,23,24,26 },  //lydian -lydi
  { 0, 2, 4, 5, 7, 9,10,12,14,16,17,19,21,22,24,26 },  //mixolydian - mixo
  { 0, 2, 3, 5, 7, 8,10,12,14,15,17,19,20,22,24,26 },  //natural minor/aeolian - aeol
  { 0, 1, 3, 5, 6, 8,10,12,13,15,17,18,20,22,24,25 },  //locrian - locr
  { 0, 1, 4, 5, 7, 8,10,12,13,16,17,19,20,22,24,25 },  //freygish - frey						
  { 0, 2, 3, 5, 7, 9,11,12,14,15,17,19,21,23,24,26 },  //melodic minor - melm
  { 0, 2, 3, 5, 7, 8,11,12,14,15,17,19,20,23,24,26 },  //harmonic minor - harm
  { 0, 3, 4, 7, 9,10,12,15,16,19,21,22,24,27,28,31 },  //major blues - majb
  { 0, 3, 5, 6, 7,10,12,15,17,18,19,22,24,27,29,30 },  //minor blues - minb  		
  { 0, 2, 4, 7, 9,12,14,16,19,21,24,26,28,31,33,36 },  //major pentatonic - majp
  { 0, 3, 5, 7,10,12,15,17,19,22,24,27,29,31,34,36 },  //minor pentatonic - minp
  { 0, 2, 3, 5, 6, 8, 9,11,12,14,15,17,18,20,23,24 },  //diminish - dimi
  { 0, 1, 3, 4, 6, 7, 9,10,12,13,15,16,18,19,21,22 },  //combi diminish - cdim   
  { 0, 1, 4, 5, 7, 8,11,12,13,16,17,19,20,23,24,25 },  //raga bhairav - ragb
  { 0, 1, 4, 6, 7, 9,11,12,13,16,18,19,21,23,24,25 },  //raga gamansrama - ragg
  { 0, 1, 3, 6, 7, 8,11,12,13,15,18,19,20,23,24,25 },  //raga todi - ragt
  { 0, 1, 3, 4, 5, 7, 8,10,12,13,15,16,17,19,20,22 },  //spanish - span
  { 0, 2, 3, 6, 7, 8,11,12,14,15,18,19,20,23,24,26 },  //gypsy - gyps
  { 0, 2, 4, 5, 6, 8,10,12,14,16,17,18,20,22,24,26 },  //arabian - arab
  { 0, 2, 5, 7,10,12,14,17,19,22,24,26,29,31,34,36 },  //egyptian - egyp
  { 0, 2, 3, 7, 9,12,14,15,19,21,24,26,27,31,33,36 },  //hawaiian - hwii
  { 0, 1, 3, 7, 8,12,13,15,19,20,24,25,27,31,32,36 },  //bali pelog - bali
  { 0, 1, 5, 7, 8,12,13,17,19,20,24,25,29,31,32,36 },  //japan - japn
  { 0, 4, 5, 7,11,12,16,17,19,23,24,28,29,31,35,36 },  //ryukyu - ryuk
  { 0, 2, 4, 6, 8,10,12,14,16,18,20,22,24,26,28,30 },  //wholetone - whle
  { 0, 3, 4, 7, 8,11,12,15,16,19,20,23,24,27,28,31 },  //augmented - augm
  { 0, 2, 4, 6,10,12,14,16,18,22,24,26,28,30,34,36 },  //prometheus - prom
  { 0, 1, 4, 6, 7,10,12,13,16,18,19,22,24,25,28,30 }   //tritone - trit																
};

char scaleName[39][4] = {
  {'m', 'a', 'j', ' '},
  {'m', 'i', 'n', ' '},
  {'d', 'i', 'm', ' '},
  {'7', 't', 'h', ' '},
  {'m', 'a', 'j', '7'},
  {'m', '7', ' ', ' '},
  {'a', 'u', 'g', ' '},
  {'c', 'h', 'r', 'o'},
  {'i', 'o', 'n', 'i'},
  {'d', 'o', 'r', 'i'},
  {'p', 'h', 'r', 'y'},
  {'l', 'y', 'd', 'i'},
  {'m', 'i', 'x', 'o'},
  {'a', 'e', 'o', 'l'},
  {'l', 'o', 'c', 'r'},
  {'f', 'r', 'e', 'y'},
  {'m', 'e', 'l', 'm'},
  {'h', 'a', 'r', 'm'},
  {'m', 'a', 'j', 'b'},
  {'m', 'i', 'n', 'b'},
  {'m', 'a', 'j', 'p'},
  {'m', 'i', 'n', 'p'},
  {'d', 'i', 'm', 'i'},
  {'c', 'd', 'i', 'm'},
  {'r', 'a', 'g', 'b'},
  {'r', 'a', 'g', 'g'},
  {'r', 'a', 'g', 't'},
  {'s', 'p', 'a', 'n'},
  {'g', 'y', 'p', 's'},
  {'a', 'r', 'a', 'b'},
  {'e', 'g', 'y', 'p'},
  {'h', 'w', 'i', 'i'},
  {'b', 'a', 'l', 'i'},
  {'j', 'a', 'p', 'n'},
  {'r', 'y', 'u', 'k'},
  {'w', 'h', 'l', 'e'},
  {'a', 'u', 'g', 'm'},
  {'p', 'r', 'o', 'm'},
  {'t', 'r', 'i', 't'}
};

char noteName[12][2] = {
  {'C', ' '},
  {'C', '#'},
  {'D', ' '},
  {'D', '#'},
  {'E', ' '},
  {'F', ' '},
  {'F', '#'},
  {'G', ' '},
  {'G', '#'},
  {'A', ' '},
  {'A', '#'},
  {'B', ' '}
};


#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

AudioSynthWaveformSine*     osc[16]   {&sine1,&sine2,&sine3,&sine4,&sine5,&sine6,&sine7,&sine8,&sine9,&sine10,&sine11,&sine12,&sine13,&sine14,&sine15,&sine16};
AudioEffectFade*            fader[16] {&fade1,&fade2,&fade3,&fade4,&fade5,&fade6,&fade7,&fade8,&fade9,&fade10,&fade11,&fade12,&fade13,&fade14,&fade15,&fade16};

void setup() {
  analogReadResolution(12);   // set resolution of ADCs to 12 bit
    
  for(int i=0;i<128;i++) {  // set up table, midi note number to frequency
      midiToFreq[i] = numToFreq(i);
  }

  
  
  //pinMode(EN_LO_PIN, OUTPUT);

  for (int i = 0; i < 4; i++){
    pinMode(mpxPin[i], OUTPUT);
  }
  pinMode(D_PIN, INPUT_PULLUP);
  pinMode(E_PIN, INPUT_PULLUP);
  pinMode(U_PIN, INPUT_PULLUP);
  pinMode(M_PIN, INPUT_PULLUP);
  pinMode(SW0_PIN, INPUT_PULLUP);
  pinMode(SW0_SNS_PIN, INPUT_PULLUP);
  pinMode(SW1_PIN, INPUT_PULLUP);
  pinMode(SW2_PIN, INPUT_PULLUP);
  pinMode(SW3_PIN, INPUT_PULLUP);

  pinMode(MPX_EN_PIN, OUTPUT);
  digitalWrite(MPX_EN_PIN, LOW);

  pinMode(LED_PIN, OUTPUT);

  //pinMode(EN_READ_PIN, INPUT_PULLUP);
  //digitalWrite(EN_LO_PIN, LOW);

  AudioMemory(50);
  dac1.analogReference(INTERNAL);   // normal volume
  //dac1.analogReference(EXTERNAL); // louder
  mixer1.gain(0, 0.27);
  mixer1.gain(1, 0.27);
  mixer1.gain(2, 0.27);
  mixer1.gain(3, 0.27);
  mixer2.gain(0, 0.27);
  mixer2.gain(1, 0.27);
  mixer2.gain(2, 0.27);
  mixer2.gain(3, 0.27);
  mixer3.gain(0, 0.5);
  mixer3.gain(1, 0.5);
  mixer3.gain(2, 0.5);
  mixer4.gain(0, 0.27);
  mixer4.gain(1, 0.27);
  mixer4.gain(2, 0.27);
  mixer4.gain(3, 0.27);
  mixer5.gain(0, 0.27);
  mixer5.gain(1, 0.27);
  mixer5.gain(2, 0.27);
  mixer5.gain(3, 0.27);

  // if stored settings are not for current version, they are replaced by factory settings
  if (readSetting(VERSION_ADDR) != VERSION){
    writeSetting(VERSION_ADDR,VERSION);
    writeSetting(TRANSP1_ADDR,TRANSP1_FACTORY);
    writeSetting(TRANSP2_ADDR,TRANSP2_FACTORY);
    writeSetting(TRANSP3_ADDR,TRANSP3_FACTORY);
    writeSetting(TRANSP4_ADDR,TRANSP4_FACTORY);
    writeSetting(SCALE1_ADDR,SCALE1_FACTORY);
    writeSetting(SCALE2_ADDR,SCALE2_FACTORY);
    writeSetting(SCALE3_ADDR,SCALE3_FACTORY);
    writeSetting(SCALE4_ADDR,SCALE4_FACTORY);
    writeSetting(DRONE1_ADDR,DRONE1_FACTORY);
    writeSetting(DRONE2_ADDR,DRONE2_FACTORY);
    writeSetting(FRMIDI_ADDR,FRMIDI_FACTORY);
    writeSetting(DRMIDI_ADDR,DRMIDI_FACTORY);
  }
  // read settings from EEPROM
  setting[TRANSP1]    = readSetting(TRANSP1_ADDR);
  setting[TRANSP2]    = readSetting(TRANSP2_ADDR);
  setting[TRANSP3]    = readSetting(TRANSP3_ADDR);
  setting[TRANSP4]    = readSetting(TRANSP4_ADDR);
  setting[SCALE1]     = readSetting(SCALE1_ADDR);
  setting[SCALE2]     = readSetting(SCALE2_ADDR);
  setting[SCALE3]     = readSetting(SCALE3_ADDR);
  setting[SCALE4]     = readSetting(SCALE4_ADDR);
  drone[0]     = readSetting(DRONE1_ADDR);
  drone[1]     = readSetting(DRONE2_ADDR);
  frameMIDIch  = readSetting(FRMIDI_ADDR);
  droneMIDIch  = readSetting(DRMIDI_ADDR);

  for (int i = 0; i < 8; i++){
    oldSetting[i] = setting[i];
  }
  
  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3D (for the 128x64)
  // init done

  //lasersOn = !digitalRead(EN_READ_PIN);
  lasersOn = 1;
  redraw();
  
  
  Serial1.begin(31250);  // start serial with midi baudrate 31250
  Serial1.flush();
}

void loop() {
  /*
  lasersOn = !digitalRead(EN_READ_PIN);
  if (lastLasersOn != lasersOn){
    redraw();
    if (!lasersOn) allNotesOff();
  }
  lastLasersOn = lasersOn;
  */
  menu();
  if (lasersOn){
    currentMillis = millis();
    if ((unsigned long)(currentMillis - statusPreviousMillis) >= CHECK_INTERVAL) {
      thrValue = map(analogRead(THR_SET_PIN),0,4096,100,3800);              // set sensitivity for light sensors
      offThr = thrValue + 200;
      readSwitches();
      //Serial.println();
      for (int scanSensors = 0; scanSensors < BEAMS; scanSensors++) {     // scan matrix for changes and send note on/off accordingly
        
        for (int i = 0; i < 4; i++) digitalWrite(mpxPin[i],bitRead(scanSensors,i)); // select inputs on multiplexer
        delayMicroseconds(50);

            /*
            Serial.print(thrValue);
            Serial.print(" ");
            Serial.print(offThr);
            Serial.print(" ");
            Serial.println(analogRead(SENSOR_PIN));
            */

        
          if (switchReading > -1){ // 0 to 3 is pressed switches
            if (!activeNote[scanSensors]){
              sensedNote = (analogRead(SENSOR_PIN) < thrValue);   // if note is off, sensedNote gets high if sensor value is lower than thrValue
            } else {
              sensedNote = (analogRead(SENSOR_PIN) < offThr);     // if note is on, sensedNote only goes low if sensor value goes over offThr
            }
            if (sensedNote != (activeNote[scanSensors] > 0)) {
              noteNumber = noteValueCheck(setting[switchReading]+scaleNote[setting[switchReading+4]][scanSensors]);
              if (sensedNote){
                  usbMIDI.sendNoteOn(noteNumber, VELOCITY, frameMIDIch + 1);    // send Note On, USB MIDI
                  midiSend((0x90 | frameMIDIch), noteNumber, VELOCITY);         // send Note On, DIN MIDI
                  internalSineNoteOn(noteNumber, scanSensors);                  // play sine note with teensy audio (built in DAC)
                  activeNote[scanSensors] = noteNumber;  
              } else {
                  usbMIDI.sendNoteOff(noteNumber, VELOCITY, frameMIDIch + 1);   // send note Off, USB MIDI
                  midiSend((0x80 | frameMIDIch), noteNumber, VELOCITY);         // send Note Off, DIN MIDI
                  internalSineNoteOff(scanSensors);                             // note off for internal audio (fade out)
                  activeNote[scanSensors] = 0;  
              }        
            }
          }
      }
      statusPreviousMillis = currentMillis;                               // reset interval timing
    }
  }
  if (anyActiveNotes()) digitalWrite(LED_PIN,HIGH); else digitalWrite(LED_PIN,LOW); 
  usbMIDI.read();
} // end of main loop

//***********************************************************

void readSwitches(){
  oldSwitchReading = switchReading;
  switchReading = -1;
  if (!digitalRead(SW3_PIN)) switchReading = 3; else
  if (!digitalRead(SW2_PIN)) switchReading = 2; else 
  if (!digitalRead(SW1_PIN)) switchReading = 1; else
  if (!digitalRead(SW0_PIN) || digitalRead(SW0_SNS_PIN)) switchReading = 0; 

  if ((switchReading != oldSwitchReading) || replay){ // button press change - stop old notes and start new, or menu updated (replay)
    for (int i = 0; i < 16; i++){
      if (activeNote[i]){
        usbMIDI.sendNoteOff(activeNote[i],VELOCITY,frameMIDIch + 1);     // send Note Off, USB MIDI
        midiSend((0x80 | frameMIDIch), activeNote[i], VELOCITY);         // send Note Off, DIN MIDI
        internalSineNoteOff(i);                                          // note off for internal audio (fade out)
      }
    }
    if (switchReading > -1){ // no new notes is -1 (no key pressed)
      for (int i = 0; i < 16; i++){
        if (activeNote[i]){
          noteNumber = noteValueCheck(setting[switchReading]+scaleNote[setting[switchReading+4]][i]);
          usbMIDI.sendNoteOn(noteNumber, VELOCITY, frameMIDIch + 1);    // send Note On, USB MIDI
          midiSend((0x90 | frameMIDIch), noteNumber, VELOCITY);         // send Note On, DIN MIDI
          internalSineNoteOn(noteNumber-12, i);                         // play sine note with teensy audio (built in DAC)
          activeNote[i] = noteNumber;
        }
      }
    } else activeNotesOff();
  }
  replay = 0;
}



//***********************************************************

// MIDI note value check with out of range octave repeat
int noteValueCheck(int note) {
  if (note > 127) {
    note = 115 + (note - 127) % 12;
  } else if (note < 1) {
    note = 12 - abs(note) % 12;
  }
  return note;
}


//***********************************************************

//  Send a three byte midi message on serial 1 (DIN MIDI) 
void midiSend(byte midistatus, byte data1, byte data2) {
  Serial1.write(midistatus);
  Serial1.write(data1);
  Serial1.write(data2);
}


//***********************************************************

void writeSetting(byte address, unsigned short value){
  union {
    byte v[2];
    unsigned short val;
  } data;
  data.val = value;
  EEPROM.write(address, data.v[0]);
  EEPROM.write(address+1, data.v[1]);  
}

//***********************************************************

unsigned short readSetting(byte address){
  union {
    byte v[2];
    unsigned short val;
  } data;  
  data.v[0] = EEPROM.read(address); 
  data.v[1] = EEPROM.read(address+1); 
  return data.val;
}

//***********************************************************


void allNotesOff(){
  for (int i = 0; i < 16; i++){
    activeNote[i] = 0;
  }
  for (int i = 0; i < 128; i++){
    usbMIDI.sendNoteOff(i,VELOCITY,frameMIDIch + 1);     // send Note Off, USB MIDI
    midiSend((0x80 | frameMIDIch), i, VELOCITY);         // send Note Off, DIN MIDI
  }
}

//************************************************************

void activeNotesOff(){
  for (int i = 0; i < 16; i++){
    if (activeNote[i]){
      usbMIDI.sendNoteOff(activeNote[i],VELOCITY,frameMIDIch + 1);     // send Note Off, USB MIDI
      midiSend((0x80 | frameMIDIch), activeNote[i], VELOCITY);         // send Note Off, DIN MIDI
    }
    activeNote[i] = 0;
  }
}

//************************************************************


int anyActiveNotes(){
  int result = 0;
  for (int i = 0; i < 16; i++){    
    if (activeNote[i])  result = 1;
  }
  return result;
}

//************************************************************

void menu() {

  // read the state of the switches
  deumButtons = !digitalRead(D_PIN)+2*!digitalRead(E_PIN)+4*!digitalRead(U_PIN)+8*!digitalRead(M_PIN);

  // check to see if you just pressed the button
  // (i.e. the input went from LOW to HIGH), and you've waited long enough
  // since the last press to ignore any noise:

  // If the switch changed, due to noise or pressing:
  if (deumButtons != lastDeumButtons) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    // if the button state has changed:
    if (deumButtons != deumButtonState) {
      deumButtonState = deumButtons;
      Serial.println(deumButtonState);
      buttonPressedAndNotUsed = 1;
      buttonPressedTime = millis();
    }

    if (((deumButtons == 1) || (deumButtons == 4)) && (millis() - buttonPressedTime > buttonRepeatDelay) && (millis() - buttonRepeatTime > buttonRepeatInterval)){
      buttonPressedAndNotUsed = 1;
      buttonRepeatTime = millis();
    }
    
  }


  // save the reading. Next time through the loop, it'll be the lastButtonState:
  lastDeumButtons = deumButtons;

  if (buttonPressedAndNotUsed){
      buttonPressedAndNotUsed = 0;
      switch (deumButtonState){
        case 1:
          // down
          //if (anyActiveNotes()) allNotesOff();
          if (setting[item] > minVal[item]){
            oldSetting[item] = setting[item];
            setting[item]--;
            replay=1;
            redraw();
          }
          break;
        case 2:
          // enter
          if (readSetting(item*2+2) != setting[item]) writeSetting(item*2+2,setting[item]);
          if (item < 7) item++; else item = 0;
          redraw();
          break;
        case 4:
          // up
          //if (anyActiveNotes()) allNotesOff();
          if (setting[item] < maxVal[item]){
            oldSetting[item] = setting[item];
            setting[item]++;
            replay=1;
            redraw();
          }
          break;
        case 8:
          // menu
          if (readSetting(item*2+2) != setting[item]) writeSetting(item*2+2,setting[item]);
          if (item > 0) item--; else item = 7;
          redraw();
          break;
      }
  }

} // menu function end

/*
void replay(){
  for (int frame = 0; frame < 4; frame++) {
    if (frameAvail[frame] && ((oldSetting[frame] != setting[frame]) || (oldSetting[frame+4] != setting[frame+4]))){
      for (int scan = 0; scan < BEAMS; scan++) {     // scan matrix and send note off for old settings
        if (activeNote[frame][scan]){
          noteNumber = oldSetting[frame]+scaleNote[oldSetting[frame+4]][scan];
          if ((noteNumber < 128) && (noteNumber > -1)) {                  // we don't want to send midi out of range
            usbMIDI.sendNoteOff(noteNumber, VELOCITY, frameMIDIch + 1);   // send note Off, USB MIDI
            midiSend((0x80 | frameMIDIch), noteNumber, VELOCITY);         // send Note Off, DIN MIDI
          }
        }
      }
       for (int scan = 0; scan < BEAMS; scan++) {     // scan matrix and send note on for new settings
        if (activeNote[frame][scan]){
          noteNumber = setting[frame]+scaleNote[setting[frame+4]][scan];
          if ((noteNumber < 128) && (noteNumber > -1)) {                  // we don't want to send midi out of range
            usbMIDI.sendNoteOn(noteNumber, VELOCITY, frameMIDIch + 1);    // send Note On, USB MIDI
            midiSend((0x90 | frameMIDIch), noteNumber, VELOCITY);         // send Note On, DIN MIDI
          }
        }
      }
    }
  } 
  for (int i = 0; i < 8; i++){
    oldSetting[i] = setting[i];
  }     
}
*/

void redraw(){
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  if (item == 0) display.print(">"); else display.print(" ");
  display.print(noteName[setting[TRANSP1]%12][0]);
  display.print(noteName[setting[TRANSP1]%12][1]);
  display.print(setting[TRANSP1]/12-2);
  if (item == 1) display.print(" >"); else display.print("  ");
  display.print(noteName[setting[TRANSP2]%12][0]);
  display.print(noteName[setting[TRANSP2]%12][1]);
  display.print(setting[TRANSP2]/12-2);
  if (item == 2) display.print(" >"); else display.print("  ");
  display.print(noteName[setting[TRANSP3]%12][0]);
  display.print(noteName[setting[TRANSP3]%12][1]);
  display.print(setting[TRANSP3]/12-2);
  if (item == 3) display.print(" >"); else display.print("  ");
  display.print(noteName[setting[TRANSP4]%12][0]);
  display.print(noteName[setting[TRANSP4]%12][1]);
  display.println(setting[TRANSP4]/12-2);
  if (item == 4) display.print(">"); else display.print(" ");
  display.print(scaleName[setting[SCALE1]][0]);
  display.print(scaleName[setting[SCALE1]][1]);
  display.print(scaleName[setting[SCALE1]][2]);
  display.print(scaleName[setting[SCALE1]][3]);
  if (item == 5) display.print(">"); else display.print(" ");
  display.print(scaleName[setting[SCALE2]][0]);
  display.print(scaleName[setting[SCALE2]][1]);
  display.print(scaleName[setting[SCALE2]][2]);
  display.print(scaleName[setting[SCALE2]][3]);
  if (item == 6) display.print(">"); else display.print(" ");
  display.print(scaleName[setting[SCALE3]][0]);
  display.print(scaleName[setting[SCALE3]][1]);
  display.print(scaleName[setting[SCALE3]][2]);
  display.print(scaleName[setting[SCALE3]][3]);
  if (item == 7) display.print(">"); else display.print(" ");
  display.print(scaleName[setting[SCALE4]][0]);
  display.print(scaleName[setting[SCALE4]][1]);
  display.print(scaleName[setting[SCALE4]][2]);
  display.println(scaleName[setting[SCALE4]][3]);
  display.println();
  display.print(" ");
  if (digitalRead(SW0_SNS_PIN)){ // switch 0 not connected, always default chord
    display.println("NRM  SW1  SW2  SW3");
  } else {
    display.println("SW0  SW1  SW2  SW3");
  }
  display.print(" ");
  /*
  display.print("  ");
  if (frameAvail[1]) display.print("HOT"); else display.print("---");
  display.print("  ");
  if (frameAvail[2]) display.print("HOT"); else display.print("---");
  display.print("  ");
  if (frameAvail[3]) display.println("HOT"); else display.println("---");
  */

  display.display();
}

//************************************************************

//Audio functions

// MIDI note number to frequency calculation
float numToFreq(int input) {
    int number = input - 21; // set to midi note numbers = start with 21 at A0 
    number = number - 48; // A0 is 48 steps below A4 = 440hz
    return 440*(pow (1.059463094359,number));
}

// play a sine wave sound using the teensy audio library
void internalSineNoteOn(int note, int num) {
  if (midiToFreq[note] > 20.0) {
    osc[num]->frequency(midiToFreq[note]);
    fader[num]->fadeIn(ATTACK);
  }
}

void internalSineNoteOff(int num) {
  fader[num]->fadeOut(RELEASE);
}

