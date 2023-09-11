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
int counter = 0;
//button
const int buttonPin = 2;
bool calibrating = false;
bool buttonDown = false;
//status LED
bool LEDstate = false;
int RXLED = 17;
int TXLED = 30;

//mapping ring position to calibration
//and sending MIDI pitch bend changes
void calculateAndSendPitch() {
  if (calibrating == false) {
    // get the value from the pin,
    // map from the center, and scale to the top
    int pitchBottom = pitchCenter - (pitchCenter - pitchTop);
    pitchCalc = 16383 * ((analogRead(A0) - pitchBottom) / (pitchTop - pitchBottom));
    //send pitch bend value to channel 1
    topLevelMidiObject.sendPitchBend(CHANNEL_1, pitchCalc);
  };
};

//calibrating pitch, first press sets the middle C,
//second press sets the top note (assumed symmetrical)
void calibratePitch() {
  //set top (second button press)
  if (calibrating && digitalRead(buttonPin) == HIGH && buttonDown == false) {
    Serial.print("Got button press 2: calibrated!");
    Serial.println();
    pitchTop = analogRead(A0);
    digitalWrite(LED_BUILTIN, LOW);
    calibrating = false;
    buttonDown = true;
    Serial.print("(finished calibration setup, setting pitchCenter to:");
    Serial.println();
    Serial.print(pitchTop);
    Serial.println();
    return;
  };
  //wait for button press, if so get value
  if (calibrating == false && digitalRead(buttonPin) == HIGH && buttonDown == false) {
    Serial.print("Got button press #1: starting calibration");
    Serial.println();
    pitchCenter = analogRead(A0);
    digitalWrite(LED_BUILTIN, HIGH);
    calibrating = true;
    buttonDown = true;
    Serial.print("(finished calibration setup, setting pitchCenter to:");
    Serial.println();
    Serial.print(pitchCenter);
    Serial.println();
  };
};

//if we're in calibration mode, flash the LED
void flashIfCalibrating() {
  if (calibrating) {
    if (counter > 2000) {
      counter = 0;
      if (LEDstate == false) {
        digitalWrite(LED_BUILTIN, HIGH);
        LEDstate = true;
      } else {
        digitalWrite(LED_BUILTIN, LOW);
        LEDstate = false;
      };
    } else {
      counter = counter + 1;
    };
  };
};

void checkButton() {
  if (digitalRead(buttonPin) == LOW && buttonDown == true) {
    buttonDown = false;
  };
};

void setup() {
  //setup pins
  pinMode(buttonPin, INPUT); //calibration button
  pinMode(LED_BUILTIN, OUTPUT); //onboard LED
  digitalWrite(LED_BUILTIN, LOW); //turn off LED
  RelativeCCSender::setMode(REAPER_RELATIVE_1); //https://tttapa.github.io/Control-Surface-doc/Doxygen/d2/d1f/RotaryEncoder_8ino-example.html
  topLevelMidiObject.begin(); //instatiate MIDI
  Serial.begin(9600);
};

void loop() {
  checkButton();
  calibratePitch();
  flashIfCalibrating(); //flashes onboard LED if we're calibrating
  calculateAndSendPitch(); //pitch bend
  Control_Surface.loop(); //other MIDI behaviour
};
