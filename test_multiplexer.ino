#include "MIDIUSB.h"
#include <CD74HC4067.h>

const int N_BUTTONS = 17;
const int BUTTON_ARDUINO_PIN[N_BUTTONS] = {};
const int CHANNEL_BUTTON_PIN = 3;  //BOTÃO SEPARADO
int buttonMuxThreshold = 300;
int buttonCState[N_BUTTONS] = { 0 };
int buttonPState[N_BUTTONS] = { 0 };
int cbuttonCState = 1;
int cbuttonPState = 0;
unsigned long cbuttonTimer = 0;
unsigned long clastDebounceTime = 0;
unsigned long lastDebounceTime[N_BUTTONS] = { 0 };
unsigned long debounceDelay = 5;
byte velocity[N_BUTTONS] = { 127 };
unsigned long buttonTimer[N_BUTTONS] = { 0 };
int buttonTimeout = 10;
int  BUTTON_MIDI_CH;

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
byte NN[N_BUTTONS] = { 51, 50, 49, 48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36 };

CD74HC4067 my_mux(16, 14, 15, 2);

const int g_common_pin = 10;

void setup() {

  Serial.begin(115200);

  for (int i = 0; i < N_BUTTONS; i++) {
    pinMode(BUTTON_ARDUINO_PIN[i], INPUT_PULLUP);
  }

  pinMode(CHANNEL_BUTTON_PIN, INPUT_PULLUP);
  pinMode(g_common_pin, INPUT_PULLUP);
}

void loop() {
  // put your main code here, to run repeatedly:
  buttons();
  potenciometers();
  channelMenu();
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

    for (int i = 0; i < N_BUTTONS; i++) {
      my_mux.channel(i);
      buttonCState[i] = digitalRead(g_common_pin);
      buttonTimer[i] = millis() - lastDebounceTime[i];  //how much time has passed since we pressed the button

      if (buttonTimer[i] > buttonTimeout) {

        if (buttonCState[i] != buttonPState[i]) {

          lastDebounceTime[i] = millis();

          if (buttonCState[i] == LOW) {
            Serial.print("CANAL ");
            Serial.println(i);
            channelMenuOn = false;
            cbuttonCState = HIGH;
          } else {
            //noteOn(MIDI_CH, NN[i], 0);
            //MidiUSB.flush();
            //Serial.print("botao ");
            //Serial.print(i);
            //Serial.println(" OFF");
          }

          buttonPState[i] = buttonCState[i];
        }
      }
    }
  }

//   while (digitalRead(CHANNEL_BUTTON_PIN) == LOW) {
//     Serial.println("Botão Canal Pressionado");
//     channelMenuOn = true;

//     // read pins from arduino
//     for (int i = 0; i < N_BUTTONS; i++) {
//       //my_mux.channel(i);
//       buttonCState[i] = digitalRead(g_common_pin);
//       //buttonTimer[i] = millis() - lastDebounceTime[i];
//     }

//     int nButtonsPerMuxSum = N_BUTTONS; // offsets the buttonCState at every mux reading

//         for (int i = 0; i < N_BUTTONS; i++) {
//           buttonCState[i + nButtonsPerMuxSum] = digitalRead(g_common_pin);
 
//           if (buttonCState[i + nButtonsPerMuxSum] > buttonMuxThreshold) {
//             buttonCState[i + nButtonsPerMuxSum] = HIGH;
//             Serial.println("Channel Button HIGH");
//           }
//           else {
//             Serial.println("to aqui no else");
//             buttonCState[i + nButtonsPerMuxSum] = LOW;
//              Serial.println("Channel Button LOW");
//           }
//         }
// //nButtonsPerMuxSum += N_BUTTONS_PER_MUX[j];


//       for (int i = 0; i < N_BUTTONS; i++) { // Read the buttons connected to the Arduino
//         Serial.println("to aqui no FOR");
//         if ((millis() - lastDebounceTime[i]) > debounceDelay) {
//             Serial.println("to aqui no IF");
//           if (buttonPState[i] != buttonCState[i]) {
//             lastDebounceTime[i] = millis();

//             if (buttonCState[i] == LOW) {
//               // DO STUFF
//               BUTTON_MIDI_CH = i;
//               Serial.print("Channel ");
//               Serial.println(BUTTON_MIDI_CH);

//             }
//             buttonPState[i] = buttonCState[i];
//           }
//         }
//       }
//     }
//   channelMenuOn = false;
} //end

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
