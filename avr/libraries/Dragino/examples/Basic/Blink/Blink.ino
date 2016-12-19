/*
  Blink :
  For LG01: Turn on/off the HEART LED on LG01, 

  modified 8 May 2014
  by Edwin Chen <support@dragino.com>
  Dragino Technology Co., Limited
 */

int HEART_LED=A2;
void setup() {
  // initialize digital pin  as an output.
  pinMode(HEART_LED, OUTPUT);
}

// the loop function runs over and over again forever
void loop() {
  digitalWrite(HEART_LED, HIGH);   // turn the HEART_LED on (HIGH is the voltage level)
  delay(1000);              // wait for a second
  digitalWrite(HEART_LED, LOW);    // turn the HEART_LED off by making the voltage LOW
  delay(1000);              // wait for a second
}
