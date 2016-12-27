/*

 This example shows how to  upload a (random) value to Yeelink IoT Server. 

 modified 26 Dec 2016
 by Dragino <support@dragino.com>
 Dragino Technology Co., Limited 
   
 */

//If you use Dragino IoT Mesh Firmware, uncomment below lines.
//For product: LG01. 
#define BAUDRATE 115200

// include all Libraries needed:
#include <Process.h>

#define APIKEY        "c234d7d15xxxxxx9b555ac7b33d8b"                  // replace your Yeelink api key here
#define DEVICEID      "353392"                     // Device ID
#define SENSORID_TEMPERATURE "398437"              // Sensor ID for Temperature
#define USERAGENT     "Dragino"           // user agent is the project name

// set up net client info:
const unsigned long postingInterval = 60000;  //delay between updates to emoncms.com
unsigned long lastRequest = 0;      // when you last made a request
String dataStringTemp = "";


void setup() {
  // start Console port:
  Bridge.begin(BAUDRATE);
  Console.begin();

  while(!Console);    // wait for Network Console to open
  Console.println("Dragino client");

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
  dataStringTemp = "{\"value\":";
  dataStringTemp += random(10) + 20;
  dataStringTemp += "}";
}

// this method makes a HTTP connection to the server:
void sendData() {
  // form the string for the API header parameter:
  String apiString = "U-ApiKey: ";
  apiString += APIKEY;

  // form the string for the URL parameter:
  String url = "http://api.yeelink.net/v1.0/";
  url += "device/";
  url += DEVICEID;
  url += "/sensor/";
  url += SENSORID_TEMPERATURE;
  url += "/datapoints";
  // Send the HTTP PUT request

  // Is better to declare the Process here, so when the
  // sendData function finishes the resources are immediately
  // released. Declaring it global works too, BTW.
  Process yeelink;
  Console.print("\n\nSending data... ");
  yeelink.begin("curl");
  yeelink.addParameter("--request");
  yeelink.addParameter("POST");
  yeelink.addParameter("--data-binary");
  yeelink.addParameter(dataStringTemp);
  yeelink.addParameter("--header");
  yeelink.addParameter(apiString);
  yeelink.addParameter(url);
  yeelink.run();
  Console.println("done!");

  // If there's incoming data from the net connection,
  // send it out the Console:
  while (yeelink.available()>0) 
  {
    char c = yeelink.read();
    Console.write(c);
  }

}


