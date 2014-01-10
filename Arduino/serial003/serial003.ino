/*
  Serial Call and Response
 Language: Wiring/Arduino
 
 This program sends an ASCII A (byte of value 65) on startup
 and repeats that until it gets some data in.
 Then it waits for a byte in the serial port, and 
 sends three sensor values whenever it gets a byte in.
 
 Thanks to Greg Shakar and Scott Fitzgerald for the improvements
 
   The circuit:
 * potentiometers attached to analog inputs 0 and 1 
 * pushbutton attached to digital I/O 2
 
 Created 26 Sept. 2005
 by Tom Igoe
 modified 24 April 2012
 by Tom Igoe and Scott Fitzgerald

 This example code is in the public domain.

 http://www.arduino.cc/en/Tutorial/SerialCallResponse

 */

int led=13;

void setup()
{
  pinMode(led, OUTPUT);
  
  // start serial port at 9600 bps:
  Serial1.begin(9600, SERIAL_8N1);
  while (!Serial1) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
}

void loop()
{
    digitalWrite(led, HIGH);
    
    // send sensor values:
    Serial1.print("-ABCDEF:"); 
    delay(500);
    
    digitalWrite(led, LOW);
    delay(300);
       
}
