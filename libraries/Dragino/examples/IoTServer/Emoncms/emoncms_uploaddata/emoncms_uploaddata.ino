/*
  Emoncms sensor client with Strings 
 
 This sketch connects an analog sensor to Emoncms,
 using a LG01
 
 created 15 March 2010
 updated 27 May 2013
 by Tom Igoe
   
 updated 26, Dec, 2016 to support Emoncms
 by Edwin Chen
   
 */

// include all Libraries needed:
#include <Process.h>
//Emoncms Settings
#define APIKEY        "16e2dc9266c6601xxxx681c0dbeaaa"         // replace your Emoncms api key here
#define USERAGENT     "Temperature"                              // user agent is the project name

//If you use Dragino IoT Mesh Firmware, uncomment below lines.
//For product: LG01. 
#define BAUDRATE 115200

// set up net client info:
const unsigned long postingInterval = 60000;  //delay between updates to emoncms.com
unsigned long lastRequest = 0;                // when you last made a request
String dataString = "";

void setup() {
  // start serial port:
  Bridge.begin(BAUDRATE);
  Console.begin();

  while(!Console);    // wait for Network Console to open
  Console.println("Emoncms client");

  // Do a first update immediately
  updateData();
  sendData();
  lastRequest = millis();
}

void loop() {
  // get a timestamp so you can calculate reading and sending intervals:
  long now = millis();

  // if the sending interval has passed since your
  // last connection, then connect again and send data:
  if (now - lastRequest >= postingInterval) {
    updateData();
    sendData();
    lastRequest = now;
  }
}

void updateData() {
  // convert the readings to a String to send it:
  dataString = "Temperature:";
  dataString += random(10) + 20;
  // add pressure:
  dataString += ",Pressure:";
  dataString += random(5) + 100;
}

// this method makes a HTTP connection to the server:
void sendData() {
  // form the string for the API header parameter:
  String apiString = "apikey=";
  apiString += APIKEY;

  // form the string for the URL parameter:
  String url = "http://emoncms.org/api/post?";
  url += "json={";
  url += dataString;
  url += "}&";
  url += apiString;
  // Send the HTTP PUT request

  // Is better to declare the Process here, so when the
  // sendData function finishes the resources are immediately
  // released. Declaring it global works too, BTW.
  Process emoncms;
  Console.print("\n\nSending data... ");
  emoncms.begin("curl");
  emoncms.addParameter("-g");
  emoncms.addParameter(url);
  emoncms.run();
  Console.println("done!");

  // If there's incoming data from the net connection,
  // send it out the Console:
  while (emoncms.available()>0) 
  {
    char c = emoncms.read();
    Console.write(c);
  }

}


