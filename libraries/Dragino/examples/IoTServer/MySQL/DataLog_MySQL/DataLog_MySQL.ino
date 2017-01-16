/*
  This example shows how to log data from analog sensor
  to MySQL server
  
  For device:
  * LG01
  
  modified 29 Dec 2016
  by  Dragino Technology Co., Limited <support@dragino.com>

  This sketch needs write a script in the openwrt side. for more detail, please refer: http://wiki.dragino.com/index.php?title=Save_Data_to_MySQL
*/
 
#include <Bridge.h>

//If you use Dragino IoT Mesh Firmware, uncomment below lines.
//For product: LG01. 
#define BAUDRATE 115200

//If you use Dragino Yun Mesh Firmware , uncomment below lines. 
//#define BAUDRATE 250000 

 
int sensor;
 
void setup() {
  // Initialize the Bridge and the Console
  Bridge.begin(BAUDRATE);
  Console.begin();
 
  while (!Console); // wait for Serial port to connect.
  Console.println("Start Sketch\n");
}
 
void loop () {
  //read the sensor
  sensor = analogRead(0);
  send_data();
 
  delay(60000);
 
}
 
// This function call the linkmysql.lua
void send_data() {
  Process logdata;
  // date is a command line utility to get the date and the time
  // in different formats depending on the additional parameter
  logdata.begin("lua");
  logdata.addParameter("/root/linkmysql.lua");  //
  logdata.addParameter(String(sensor));  //
  logdata.run();  // run the command
 
  // read the output of the command
  while (logdata.available() > 0) {
    char c = logdata.read();
  }
  Console.println("send data done.");
}
