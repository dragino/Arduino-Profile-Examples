/*
  HC-SR04_TEST :
    Use SR04 ultro sonic sensor to measure distance and show it on the serial monity

  Hardware Connection:
   Trig < -- > Arduino D3
   Echo pin < -- > Arduino D4
   VCC < -- > Arduino 5v
   GND < -- > Arduino GND

  by Dragino <support@dragino.com>
  Dragino Technology Co., Limited
*/
 
// Set up SR04 pins to Arduino
const int TrigPin = 3; 
const int EchoPin = 4; 
float distance; 
void setup() 
{   // initial serial connection
        Serial.begin(9600); 
        pinMode(TrigPin, OUTPUT); 
    // Set up pin to input mode to detect wave width. 
        pinMode(EchoPin, INPUT); 
        Serial.println("Ultrasonic sensor:");
} 
void loop() 
{ 
    // Generate a 19us high wave to trigger TrigPin
        digitalWrite(TrigPin, LOW); 
        delayMicroseconds(2); 
        digitalWrite(TrigPin, HIGH); 
        delayMicroseconds(10);
        digitalWrite(TrigPin, LOW); 
    // Detect the Wave Length and Caculate the distance. 
        distance = pulseIn(EchoPin, HIGH) / 58.00;
        Serial.print(distance); 
        Serial.print("cm"); 
        Serial.println(); 
        delay(1000); 
}
