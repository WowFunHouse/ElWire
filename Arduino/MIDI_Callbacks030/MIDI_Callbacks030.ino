#include <MIDI.h>

// This function will be automatically called when a NoteOn is received.
// It must be a void-returning function with the correct parameters,
// see documentation here: 
// http://arduinomidilib.sourceforge.net/class_m_i_d_i___class.html

/* Midi Note definition:
  Dancer01: C2 (24) - Eb (27)        Blue | Green | Warm | Glass
  Dancer02: F2 (29) - Ab (32)        Blue | Green | Warm | Glass
  Dancer03: G2 (33) - C3 (36)        Blue | Green | Warm | Glass
  Dancer04: D3 (38) - F3 (41)        Blue | Green | Warm | Glass
  Dancer05: G3 (43) - Bb (46)        Blue | Green | Warm | Glass
  
  Note: E2 (28), D3 (37), Gb3 (42) are not in used
*/  
  
#define  STMAN01_NOTE  24
#define  STMAN02_NOTE  29
#define  STMAN03_NOTE  33
#define  STMAN04_NOTE  38
#define  STMAN05_NOTE  43

#define  STMAN_LASTNOTE  46

/* El Wire color definitation:
  0: Blue
  1: Green
  2: Warm color
  3: Eye glasses
*/

#define  NOTE_OFFSET_BLU  0
#define  NOTE_OFFSET_GRN  1
#define  NOTE_OFFSET_WRM  2
#define  NOTE_OFFSET_GLS  3

/* Data Packet definition: 
    |    b7    | b6 | b5 b4 | b3 |    b2 - b0     |
    |- ON/OFF -| NC | COLOR | NC |---Dancer ID ---|
*/
  
#define  DANCER01    00
#define  DANCER02    01
#define  DANCER03    02
#define  DANCER04    03
#define  DANCER05    04
#define  DANCERALL   07

#define  ELWIRE_BLU  0
#define  ELWIRE_GRN  1
#define  ELWIRE_WRM  2
#define  ELWIRE_GLS  3

#define  ELWIRE_ON   1
#define  ELWIRE_OFF  0

#define  PIN_DANCER_B0  2
#define  PIN_DANCER_B1  3
#define  PIN_DANCER_B2  4

#define  PIN_COLOR_B0   5
#define  PIN_COLOR_B1   6
#define  PIN_COLOR_B2   7
#define  PIN_COLOR_MODE   8

#define  PIN_DATA_RDY   9       // 1: Ready
#define  PIN_DATA_ACK   10      // 0: Acknowledged

unsigned char led=13;

void dataSetReady()
{
  digitalWrite(PIN_DATA_RDY, HIGH);
} /* dataSetReady */

void dataNotReady()
{
  digitalWrite(PIN_DATA_RDY, LOW);
} /* dataNotReady */

void dataSendTo8051()
{
  dataSetReady();                  // Data Ready for 8051 wireless transmittion
  while (digitalRead(PIN_DATA_ACK));    // Wait for acknowledgment from 8051
  dataNotReady();                  // Reset Data Ready flag  
} /* dataAckRxd */

void elwireSetDancer(byte elDancer)
{
  int i=1;
  
  digitalWrite(PIN_DANCER_B0,  (elDancer & 1));
  digitalWrite(PIN_DANCER_B1, ((elDancer >> i++) & 1));
  digitalWrite(PIN_DANCER_B2, ((elDancer >> i++) & 1));
  
} /* elwireSetDancer */

void elwireSetColor(byte elIndex, byte elMode)
{
  int i=1;
  
  digitalWrite(PIN_COLOR_B0,  (elIndex & 1));
  digitalWrite(PIN_COLOR_B1, ((elIndex >> i++) & 1));
  digitalWrite(PIN_COLOR_B2, ((elIndex >> i++) & 1));
  
  digitalWrite(PIN_COLOR_MODE, elMode);

} /* elwireSetColor */

void elwireSetColorDefault()
{
  elwireSetColor(ELWIRE_BLU, ELWIRE_ON);
} /* elwireSetColorDefault */

void HandleNoteOn(byte channel, byte pitch, byte velocity) {
  byte elDancer, elIndex, elMode;
  
  // Do whatever you want when you receive a Note On
  if ((pitch >= STMAN01_NOTE) && (pitch <= STMAN_LASTNOTE))
  {      
      if (velocity == 0) {
            // This acts like a NoteOff
            elMode = ELWIRE_OFF;
            digitalWrite(led, LOW);
      }
      else {
        // A key is pressed
        elMode = ELWIRE_ON;
        digitalWrite(led, HIGH);
      }
      
      if (pitch < STMAN02_NOTE) {          // Dancer 1
        elDancer = 0;
        elIndex  = pitch - STMAN01_NOTE;   
      }
      else if (pitch <= STMAN03_NOTE) {    // Dancer 2
        elDancer = 1;
        elIndex  = pitch - STMAN02_NOTE;    
      } 
      else if (pitch <= STMAN04_NOTE) {    // Dancer 3
        elDancer = 2;
        elIndex  = pitch - STMAN03_NOTE;
      }
      else if (pitch <= STMAN05_NOTE) {    // Dancer 4
        elDancer = 3;
        elIndex  = pitch - STMAN04_NOTE;
      }
      else {                               // Dancer 5
        elDancer = 4;
        elIndex  = pitch - STMAN05_NOTE;
      }
      
      elwireSetDancer(elDancer);
      
      if (elIndex < 3) {
          elwireSetColor(elIndex, elMode);
      }
      
      dataSendTo8051();                    // Send to 8051 wireless
    }
  
  // Try to keep your callbacks short (no delays ect) as the contrary would slow down the loop()
  // and have a bad impact on real-time performance.
} /* HandleNoteOn */

void setup() {
  // Initiate MIDI communications, listen to all channels
  MIDI.begin(MIDI_CHANNEL_OMNI);    
  
  // Connect the HandleNoteOn function to the library, so it is called upon reception of a NoteOn.
  MIDI.setHandleNoteOn(HandleNoteOn);  // Put only the name of the function
  
  pinMode(led, OUTPUT);
  
  pinMode(PIN_DANCER_B0, OUTPUT);
  pinMode(PIN_DANCER_B1, OUTPUT);
  pinMode(PIN_DANCER_B2, OUTPUT);
  
  pinMode(PIN_COLOR_B0, OUTPUT);
  pinMode(PIN_COLOR_B1, OUTPUT);
  pinMode(PIN_COLOR_B2, OUTPUT);
  
  pinMode(PIN_COLOR_MODE, OUTPUT);
  
  pinMode(PIN_DATA_RDY, OUTPUT);
  pinMode(PIN_DATA_ACK, INPUT);

} /* setup */


void loop() {

  
  // Call MIDI.read the fastest you can for real-time performance
  MIDI.read();
  
  // There is no need to check if there are messages incoming if they are bound to a Callback function.
} /* loop */
