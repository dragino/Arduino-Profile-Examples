/*
 LG01  HTTP Client

 This example for the LG01 shows how create a basic
 HTTP client that connects to the internet and downloads
 content. In this case, you'll connect to the Arduino
 website and download a version of the logo as ASCII text.

 created by Tom igoe
 May 2013
 modified by Dragino 
 May 2017
 

 This example code is in the public domain.

 http://www.arduino.cc/en/Tutorial/HttpClient

 */

#include <Bridge.h>
#include <HttpClient.h>
#define BAUDRATE 115200

void setup() {
  // Bridge takes about two seconds to start up
  // it can be helpful to use the on-board LED
  // as an indicator for when it has initialized
  pinMode(A2, OUTPUT);
  digitalWrite(A2, LOW);
  Bridge.begin(BAUDRATE);
  digitalWrite(A2, HIGH);

  Console.begin();

  while (!Console); // wait for a serial connection
}

void loop() {
  // Initialize the client library
  HttpClient client;

  // Make a HTTP request:
  client.get("https://www.arduino.cc/asciilogo.txt");

  // if there are incoming bytes available
  // from the server, read them and print them:
  while (client.available()) {
    char c = client.read();
    Console.print(c);
  }
  Console.flush();

  delay(5000);
}


