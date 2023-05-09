/*
  by Adam Staff
  Onde Marinot controller

  Pitch on A0
  Volume on A1
  Plus rotary encoder for other stuff across

  Uses Control Surface library
*/

//libraries
#include <Control_Surface.h>

//init midi interface
USBMIDI_Interface topLevelMidiObject;
//initiate the pressure sensor
CCPotentiometer pressure {
  A1, CHANNEL_1,
};
//init rotary encoder
//TODO
CCRotaryEncoder encoder {
  {3, 4}, //pins
  {MIDI_CC::General_Purpose_Controller_1, CHANNEL_1}, //MIDI address
  1, // optional multiplier
};
// init vars
int analogPin = A3;
int pitchCenter = 512;
int pitchTop = 1023;
int pitchCalc = 0;
//button
const int buttonPin = 2;
int calibrating = false;
//status LED
int LEDstate = false;

//mapping ring position to calibration
//and sending MIDI pitch bend changes
void calculateAndSendPitch() {
  // get the value from the pin,
  // map from the center, and scale to the top
  pitchCalc = pitchCenter - analogRead(A0) / pitchTop;
  //send pitch bend value to channel 1
  topLevelMidiObject.sendPitchBend(CHANNEL_1, pitchCalc);
};

//calibrating pitch, first press sets the middle C,
//second press sets the top note (assumed symmetrical)
void calibratePitch() {
  //set top (second button press)
  if (calibrating && digitalRead(buttonPin) == HIGH) {
    pitchTop = analogRead(A0);
    calibrating = false;
    digitalWrite(LED_BUILTIN, LOW);
    return;
  };
  //wait for button press, if so get value
  if (digitalRead(buttonPin) == HIGH) {
    pitchCenter = analogRead(A0);
    calibrating = true;
    digitalWrite(LED_BUILTIN, HIGH);
  };
};

//if we're in calibration mode, flash the LED
void flashIfCalibrating() {
  if (calibrating) {
    if (LEDstate) {
      digitalWrite(LED_BUILTIN, HIGH);
      LEDstate = true;
    } else {
      digitalWrite(LED_BUILTIN, LOW);
      LEDstate = false;
    }
  };
};

void setup() {
  //setup pins
  pinMode(buttonPin, INPUT); //calibration button
  pinMode(LED_BUILTIN, OUTPUT); //onboard LED
  digitalWrite(LED_BUILTIN, LOW); //turn off LED
  RelativeCCSender::setMode(REAPER_RELATIVE_1); //https://tttapa.github.io/Control-Surface-doc/Doxygen/d2/d1f/RotaryEncoder_8ino-example.html
  topLevelMidiObject.begin(); //instatiate MIDI
};

void loop() {
  flashIfCalibrating(); //flashes onboard LED if we're calibrating
  calculateAndSendPitch(); //pitch bend
  Control_Surface.loop(); //other MIDI behaviour
};