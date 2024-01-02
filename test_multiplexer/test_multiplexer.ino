#include "MIDIUSB.h"
#include <CD74HC4067.h>
#include <FastLED.h>
//======================================================================
//LEDs
//======================================================================
FASTLED_USING_NAMESPACE

#define DATA_PIN    3
//#define CLK_PIN   4
#define LED_TYPE    WS2812
#define COLOR_ORDER GRB
#define NUM_LEDS    16
#define HUE_OFFSET 90
CRGB leds[NUM_LEDS];
//byte ledIndex[NUM_LEDS] = {3, 7, 11, 15, 2, 6, 10, 14, 1, 5, 9, 13, 0, 4, 8, 12};
int ledIndex[NUM_LEDS] = {12,13,14,15,11,10,9,8,4,5,6,7,3,2,1,0}; //reindexando os LEDS

#define BRIGHTNESS          96
#define FRAMES_PER_SECOND  120
byte midiChMenuColor = 200;
byte ch1Hue = 135;
byte maxHue = 240;
int dot;
//===================================================================================
const int N_BUTTONS = 16;
const int BUTTON_ARDUINO_PIN[N_BUTTONS] = {};
const int CHANNEL_BUTTON_PIN = 5;  //BOTÃO SEPARADO
//int buttonMuxThreshold = 300;
int buttonCState[N_BUTTONS] = { 0 };
int buttonPState[N_BUTTONS] = { 0 };
int cbuttonCState = 1;
//unsigned long cbuttonTimer = 0;
//unsigned long clastDebounceTime = 0;
unsigned long lastDebounceTime[N_BUTTONS] = { 0 };
unsigned long debounceDelay = 5;
byte velocity[N_BUTTONS] = { 127 };
unsigned long buttonTimer[N_BUTTONS] = { 0 };
int buttonTimeout = 10;
int BUTTON_MIDI_CH;

//=============================================================================================================
const int N_POTS = 8;
byte potPIN[N_POTS] = { A0, A1, A2, A3, 4, 6, 8, 9 };
int potState[N_POTS] = { 0 };   //estado atual do potenciômetro
int potPState[N_POTS] = { 0 };  //estado anterior do potenciômetro
int midiState[N_POTS] = { 0 };
int midiPState[N_POTS] = { 0 };
int CC[N_POTS] = { 1, 2, 3, 4, 5, 6, 7, 8 };  //int CC[N_POTS] = {11,12};
int potThreshold = 35;
unsigned long lastPotTime[N_POTS] = { 0 };
unsigned long potTimer[N_POTS] = { 0 };
int potTimeout = 150;
int potVar = 0;
byte midiCh = 1; //* Canal MIDI a ser usado
byte cc = 1; //* O mais baixo MIDI CC a ser usado

bool channelMenuOn = false;

byte MIDI_CH = 0;  //0 - 15
//byte NN[N_BUTTONS] = { 51, 50, 49, 48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36 };
//byte NN[N_BUTTONS] = { 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51 };
byte NN[N_BUTTONS] = { 36, 40, 44, 48, 37, 41, 45, 49, 38, 42 , 46, 50, 39, 43, 47,51  };

CD74HC4067 my_mux(16, 14, 15, 2);

const int g_common_pin = 10;

void setup() {

  Serial.begin(115200);

  for (int i = 0; i < N_BUTTONS; i++) {
    pinMode(BUTTON_ARDUINO_PIN[i], INPUT_PULLUP);
  }

  pinMode(CHANNEL_BUTTON_PIN, INPUT_PULLUP);
  pinMode(g_common_pin, INPUT_PULLUP);

  //FAST LED
  //FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  setAllLeds(ch1Hue, 30);// set all leds at once with a hue (hue, randomness)

  FastLED.show();

  //////////////////////////////////////
}

void loop() {
  buttons();
  potenciometers();
  channelMenu();

   for(int dot = 0; dot < NUM_LEDS; dot++) { 
     leds[dot] = CRGB::Blue;
     FastLED.show();
   }

}

void buttons() {

  for (int i = 0; i < N_BUTTONS; i++) {

    my_mux.channel(i);
    buttonCState[i] = digitalRead(g_common_pin);
    buttonTimer[i] = millis() - lastDebounceTime[i];  //how much time has passed since we pressed the button

    if (buttonTimer[i] > buttonTimeout) {

      if (buttonCState[i] != buttonPState[i]) {

        lastDebounceTime[i] = millis();

        if (buttonCState[i] == LOW) {
          noteOn(MIDI_CH, NN[i], 127);
          MidiUSB.flush();

          Serial.print("botao ");
          Serial.print(i);
          Serial.println(" ON");
          
          // if (i < 4){
          //   leds[i+12] = CRGB::Red;
          //   FastLED.show();
          //   delay(500);
          // }

           leds[ledIndex[i]] = CRGB::Red;
           FastLED.show();
           delay(500);


        } else {
          noteOn(MIDI_CH, NN[i], 0);
          MidiUSB.flush();
          Serial.print("botao ");
          Serial.print(i);
          Serial.println(" OFF");
        }

        buttonPState[i] = buttonCState[i];
      }
    }
  }
}

void channelMenu() {
  cbuttonCState = digitalRead(CHANNEL_BUTTON_PIN);
  while (cbuttonCState == LOW) {
    channelMenuOn = true;

    setAllLeds(midiChMenuColor, 0); // turn all lights into the menu lights
    leds[ledIndex[BUTTON_MIDI_CH]].setHue(midiChMenuColor + 60);

     for (int dot=0; dot < NUM_LEDS; dot++){
       leds[dot] = CRGB::Purple;
       FastLED.show();
     }

    for (int i = 0; i < N_BUTTONS; i++) {
      my_mux.channel(i);
      buttonCState[i] = digitalRead(g_common_pin);
      buttonTimer[i] = millis() - lastDebounceTime[i];  //how much time has passed since we pressed the button

      if (buttonTimer[i] > buttonTimeout) {

        if (buttonCState[i] != buttonPState[i]) {

          lastDebounceTime[i] = millis();

          if (buttonCState[i] == LOW) {
            BUTTON_MIDI_CH = i;
            Serial.print("CANAL ");
            Serial.println(i);
            //channelMenuOn = false;
            cbuttonCState = HIGH;
            leds[ledIndex[i]] = CRGB::Red;
            FastLED.show();
            delay(200);
          } else {
            noteOn(MIDI_CH, NN[i], 0);
            MidiUSB.flush();
            leds[ledIndex[i]] = CRGB::Blue;
            FastLED.show();
            //Serial.print("botao ");
            //Serial.print(i);
            //Serial.println(" OFF");
          }

          buttonPState[i] = buttonCState[i];
        }
      }
    }
  }
} 

  void potenciometers() {

    for (int i = 0; i < N_POTS; i++) {
      potState[i] = analogRead(potPIN[i]);               //reads the potenciometer
      midiState[i] = map(potState[i], 0, 1023, 0, 127);  //utilizando map para configurar o novo range do potenciômetro, que vai de 0 a 127 [MIDI]

      potVar = abs(potState[i] - potPState[i]);  //abs porque preciso do valor absoluto, não negativo

      if (potVar > potThreshold) {
        lastPotTime[i] = millis();  //"resets" the clock
      }

      potTimer[i] = millis() - lastPotTime[i];  //how much time has passed since we moved the pot

      if (potTimer[i] < potTimeout) {
        if (midiPState[i] != midiState[i]) {

          //controlChange(MIDI_CH, CC[i], midiState[i]);
          controlChange(midiCh, cc + i, midiState[i]);
          MidiUSB.flush();

          Serial.print("Pot");
          Serial.print(i);
          Serial.print(" ");
          Serial.println(midiState[i]);

          midiPState[i] = midiState[i];
          potPState[i] = potState[i];
        }
      }
    }
  }
  

  //Arduino pro micro midi functions MIDIUSB library
  void noteOn(byte channel, byte pitch, byte velocity) {
    midiEventPacket_t noteOn = { 0x09, 0x90 | channel, pitch, velocity };
    MidiUSB.sendMIDI(noteOn);
  }

  void controlChange(byte channel, byte control, byte value) {
    midiEventPacket_t event = { 0x0B, 0xB0 | channel, control, value };
    MidiUSB.sendMIDI(event);
  }

  void setAllLeds(byte hue_, byte randomness_) {

    for (int i = 0; i < NUM_LEDS; i++) {
      byte offset = random(0, randomness_);
      leds[i].setHue(hue_  + offset);
    }
  }



  
