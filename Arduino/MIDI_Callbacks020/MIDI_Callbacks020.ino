#include <MIDI.h>
#include <SoftwareSerial.h>    x

// This function will be automatically called when a NoteOn is received.
// It must be a void-returning function with the correct parameters,
// see documentation here: 
// http://arduinomidilib.sourceforge.net/class_m_i_d_i___class.html

/* Midi Note definition:
  Dancer01: C2 (24) - Eb (27)
  Dancer02: F2 (29) - Ab (32)
  Dancer03: G2 (33) - C3 (36)
  Dancer04: D3 (38) - F3 (41)
  Dancer05: G3 (43) - Bb (46)
*/  
  
#define  STMAN01_NOTE  24
#define  STMAN02_NOTE  29
#define  STMAN03_NOTE  33
#define  STMAN03_NOTE  38
#define  STMAN04_NOTE  43

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
  
#define  DANCER01    00;
#define  DANCER02    01;
#define  DANCER03    02;
#define  DANCER04    03;
#define  DANCER05    04;
#define  DANCERALL   07;

#define  ELWIRE_BLU  0x00
#define  ELWIRE_GRN  0x08
#define  ELWIRE_WRM  0x10
#define  ELWIRE_GLS  0x18

#define  ELWIRE_ON   0x80;
#define  ELWIRE_OFF  0x00;

unsigned char led=13;
  
unsigned char msgPacket=0;
unsigned char msgReady=0;

unsigned char msgDATA[]={0, 1, 2, 3,          // Note 24 - 27 [Dancer 1]
                           0,                   // Note 28 (not in used)
                           0, 1, 2, 3,          // Note 29 - 32 [Dancer 2]       
                           0, 1, 2, 3,          // Note 33 - 36 [Dancer 3]
                           0,                   // Note 37 (not in used)
                           0, 1, 2, 3,          // Note 38 - 41 [Dancer 3]
                           0,                   // Note 42 (not in used)                     
                           0, 1, 2, 3           // Note 43 - 46 [Dancer 3]
                          };


void HandleNoteOn(byte channel, byte pitch, byte velocity) {
  int msgIndex, elStatus;
  
  // Do whatever you want when you receive a Note On.
  if ((pitch >= STMAN01_NOTE) && (pitch <= STMAN_LASTNOTE))
  {
      msgIndex = pitch - STMAN01_NOTE;
      
      if (velocity == 0) {
            // This acts like a NoteOff.
            elStatus = ELWIRE_OFF;
            digitalWrite(led, LOW);
      }
      else {
        elStatus = ELWIRE_ON;
        digitalWrite(led, HIGH);
      }
      
      msgPacket = msgDATA[msgIndex] | elStatus;
      msgReady=1;
  }
  
  // Try to keep your callbacks short (no delays ect) as the contrary would slow down the loop()
  // and have a bad impact on real-time performance.
}

void setup() {
  // Initiate MIDI communications, listen to all channels
  MIDI.begin(MIDI_CHANNEL_OMNI);    
  
  // Connect the HandleNoteOn function to the library, so it is called upon reception of a NoteOn.
  MIDI.setHandleNoteOn(HandleNoteOn);  // Put only the name of the function
  
  pinMode(led, OUTPUT);
}


void loop() {
  // Call MIDI.read the fastest you can for real-time performance.
  MIDI.read();
  

  // There is no need to check if there are messages incoming if they are bound to a Callback function.
}
