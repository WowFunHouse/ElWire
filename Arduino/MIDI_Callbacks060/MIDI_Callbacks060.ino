/**************************************************************
 Title:	        Midi to Parallel (for 8051 Wireless Peer)		
 
 File:	        MIDI_Callbacks060.c

 Version:       1.0
 Description:   Arduino will convert Midi Note messages to parllel packet
 		and transmit through WSN-02/03 433MHz transmitter to El-Wire
                dancers by a 8051 Wireless peer.

 Created on:	        2014-01-11
 Created by:    Michael
  
 Board:	        Arduino UNO + Midi Shield

 Connections:	MCS-51 P1.0 <- Arduino Pin 2 (Dancer ID Bit0)
 	        MCS-51 P1.1 <- Arduino Pin 3 (Dancer ID Bit1)
	        MCS-51 P1.2 <- Arduino Pin 4 (Dancer ID Bit2)
		MCS-51 P1.3 <- Arduino Pin 7 (ElWire Mode 1:On 0:Off)

		MCS-51 P1.4 <- Arduino Pin 5 (ElWire Index Bit 0)
	        MCS-51 P1.5 <- Arduino Pin 6 (ElWire Index Bit 1)

		MCS-51 P1.6 <- Arduino Pin 8 (Data Ready  0:Ready)
	        MCS-51 P1.7 -> Arduino Pin 9 (Data Rx Ack 0:Ack   1:Standby)

 **************************************************************/
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

#define  MIDI_NOTE_OFFSET  24

/* El Wire color definitation:
  0: Blue
  1: Green
  2: Warm color
  3: Eye glasses
*/

#define  NOTE_OFFSET_BLU  0
#define  NOTE_OFFSET_GRN  1
#define  NOTE_OFFSET_AUX  2
#define  NOTE_OFFSET_GLS  3

/* Data PACKet definition: 
    | b7 | b6 | b5 b4 | - b3 - |    b2 - b0     |
    | NC | NC | COLOR | ON/OFF |---Dancer ID ---|
*/
  
#define  DANCER01    00
#define  DANCER02    01
#define  DANCER03    02
#define  DANCER04    03
#define  DANCER05    04
#define  DANCERALL   07

#define  ELWIRE_BLU  0
#define  ELWIRE_GRN  1
#define  ELWIRE_AUX  2
#define  ELWIRE_GLS  3

#define  ELWIRE_ON   1
#define  ELWIRE_OFF  0

#define  PIN_DANCER_B0  2
#define  PIN_DANCER_B1  3
#define  PIN_DANCER_B2  4

#define  PIN_WIRE_MODE  5      // El-Wire 1:On 0:Off

#define  PIN_COLOR_B0   6
#define  PIN_COLOR_B1   7

#define  PIN_DATA_RDY   8      // 1: Data ready for peer
#define  PIN_DATA_ACK   9      // 0: Acknowledged Data received from peer

unsigned char led=13;

void ardDancerSelect(unsigned char elDancer)
{
  int i=1;
  
  digitalWrite(PIN_DANCER_B0,  (elDancer & 1));
  digitalWrite(PIN_DANCER_B1, ((elDancer >> i++) & 1));
  digitalWrite(PIN_DANCER_B2, ((elDancer >> i++) & 1));
  
} /* ardDancerSelect */

void ardElWireSelect(unsigned char elIndex, unsigned char elMode)
{
  digitalWrite(PIN_COLOR_B0,  (elIndex & 1));
  digitalWrite(PIN_COLOR_B1, ((elIndex >> 1) & 1));  
  digitalWrite(PIN_WIRE_MODE, elMode);

} /* ardElWireSelect */

void ardElWireSelectDefault()
{
  ardElWireSelect(ELWIRE_BLU, ELWIRE_ON);
} /* ardElWireSelectDefault */

void ardDataReady()
{
  digitalWrite(PIN_DATA_RDY, HIGH);
  
} /* ardDataReady */

void ardDataNotReady()
{
  digitalWrite(PIN_DATA_RDY, LOW);
  
} /* ardDataNotReady */

void ardDataSend(unsigned char dancer, unsigned char elwire, unsigned char mode)
{
  
  ardDancerSelect(dancer);
  ardElWireSelect(elwire, mode);
  
  ardDataReady();                      // Data Ready for peer wireless transmittion
  while (digitalRead(PIN_DATA_ACK));   // Wait for -ACK from peer
  ardDataNotReady();                   // Reset Data Ready flag
  
} /* ardDataSend */

void HandleNoteOn(unsigned char channel, unsigned char pitch, unsigned char velocity) {
  unsigned char elDancer, elIndex, elMode;
  
  // Do whatever you want when you receive a Note On
  if ((pitch >= (STMAN01_NOTE   + MIDI_NOTE_OFFSET)) 
   && (pitch <= (STMAN_LASTNOTE + MIDI_NOTE_OFFSET)))
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
      
      if (pitch < (STMAN02_NOTE + MIDI_NOTE_OFFSET)) {         // Dancer 1
        elDancer = 0;
        elIndex  = pitch - (STMAN01_NOTE + MIDI_NOTE_OFFSET);   
      }
      else if (pitch < (STMAN03_NOTE + MIDI_NOTE_OFFSET)) {    // Dancer 2
        elDancer = 1;
        elIndex  = pitch - (STMAN02_NOTE + MIDI_NOTE_OFFSET);    
      } 
      else if (pitch < (STMAN04_NOTE + MIDI_NOTE_OFFSET)) {    // Dancer 3
        elDancer = 2;
        elIndex  = pitch - (STMAN03_NOTE + MIDI_NOTE_OFFSET);
      }
      else if (pitch < (STMAN05_NOTE + MIDI_NOTE_OFFSET)) {    // Dancer 4
        elDancer = 3;
        elIndex  = pitch - (STMAN04_NOTE + MIDI_NOTE_OFFSET);
      }
      else {                               // Dancer 5
        elDancer = 4;
        elIndex  = pitch - (STMAN05_NOTE + MIDI_NOTE_OFFSET);
      }
       
      if (elIndex < 4) {
          ardDataSend(elDancer, elIndex, elMode);  // Send to Peer
      }
    }
  
  // Try to keep your callbACKs short (no delays ect) as the contrary would slow down the loop()
  // and have a bad impact on real-time performance.
} /* HandleNoteOn */

void HandleNoteOff(unsigned char channel, unsigned char pitch, unsigned char velocity) {
  unsigned char elDancer, elIndex, elMode;
  
  // Do whatever you want when you receive a Note On
  if ((pitch >= (STMAN01_NOTE   + MIDI_NOTE_OFFSET)) 
   && (pitch <= (STMAN_LASTNOTE + MIDI_NOTE_OFFSET)))
  {      
      elMode = ELWIRE_OFF;
      digitalWrite(led, LOW);

      if (pitch < (STMAN02_NOTE + MIDI_NOTE_OFFSET)) {         // Dancer 1
        elDancer = 0;
        elIndex  = pitch - (STMAN01_NOTE + MIDI_NOTE_OFFSET);   
      }
      else if (pitch < (STMAN03_NOTE + MIDI_NOTE_OFFSET)) {    // Dancer 2
        elDancer = 1;
        elIndex  = pitch - (STMAN02_NOTE + MIDI_NOTE_OFFSET);    
      } 
      else if (pitch < (STMAN04_NOTE + MIDI_NOTE_OFFSET)) {    // Dancer 3
        elDancer = 2;
        elIndex  = pitch - (STMAN03_NOTE + MIDI_NOTE_OFFSET);
      }
      else if (pitch < (STMAN05_NOTE + MIDI_NOTE_OFFSET)) {    // Dancer 4
        elDancer = 3;
        elIndex  = pitch - (STMAN04_NOTE + MIDI_NOTE_OFFSET);
      }
      else {                               // Dancer 5
        elDancer = 4;
        elIndex  = pitch - (STMAN05_NOTE + MIDI_NOTE_OFFSET);
      }
       
      if (elIndex < 4) {
          ardDataSend(elDancer, elIndex, elMode);  // Send to Peer
      }
    }
  
  // Try to keep your callbACKs short (no delays ect) as the contrary would slow down the loop()
  // and have a bad impact on real-time performance.
} /* HandleNoteOff */

void ardElWireDiag(void) {
  unsigned char dancer, elwire;
  unsigned char n;
  unsigned int  t;
  
  t = 100;                                             // Light up all El-Wires of all dancers for 100ms      
  
  for (n=0; n<2; n++) {
    for (dancer=0; dancer<5; dancer++) {
      for (elwire=0; elwire<4; elwire++) {
        ardDataSend(dancer, elwire, ELWIRE_OFF);        // Turn off all El-Wires of all dancers
      }
    }
    
    for (dancer=0; dancer<5; dancer++) {
      for(elwire=0; elwire<4; elwire++) {
        ardDataSend(dancer, elwire, ELWIRE_ON);        // Turn on all El-Wires of all dancers      
      }
    }
  }

  delay(t);
  for (n=0; n<2; n++) {
    for (dancer=0; dancer<5; dancer++) {
      for (elwire=0; elwire<4; elwire++) {
        ardDataSend(dancer, elwire, ELWIRE_OFF);        // Turn off all El-Wires of all dancers
      }
    }
  }  
} /* ardElWireDiag */

void setup() {
  // Initiate MIDI communications, listen to all channels
  MIDI.begin(MIDI_CHANNEL_OMNI);    
  
  // Connect the HandleNoteOn function to the library, so it is called upon reception of a NoteOn.
  MIDI.setHandleNoteOn(HandleNoteOn);  // Put only the name of the function
  MIDI.setHandleNoteOff(HandleNoteOff);
  
  pinMode(led, OUTPUT);
  
  pinMode(PIN_DANCER_B0, OUTPUT);
  pinMode(PIN_DANCER_B1, OUTPUT);
  pinMode(PIN_DANCER_B2, OUTPUT);
  
  pinMode(PIN_COLOR_B0, OUTPUT);
  pinMode(PIN_COLOR_B1, OUTPUT);
  
  pinMode(PIN_WIRE_MODE, OUTPUT);
  
  pinMode(PIN_DATA_RDY, OUTPUT);
  pinMode(PIN_DATA_ACK, INPUT);

  ardDataNotReady();            // No data for output
  
  ardElWireDiag();              // Execute system diagnostic
  
} /* setup */


void loop() {

  
  // Call MIDI.read the fastest you can for real-time performance
  MIDI.read();
  
  // There is no need to check if there are messages incoming if they are bound to a Callb function.
} /* loop */
