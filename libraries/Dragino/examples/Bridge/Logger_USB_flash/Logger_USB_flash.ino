/*
 USB flash datalogger
 
 This example shows how to log data to the USB flash attached to Dragino LG01 .
   
 The circuit:
 * LG01
 * USB flash acctached to LG01. 
 
 Prepare your USB flash: creating an empty folder in the USB flash root 
 named "data". and create a file call "datalog.csv".
 
 You can remove the USB flash while the Linux and the 
 sketch are running but be careful not to remove it while
 the system is writing to it.
 
 This example is a reference from:
 
 http://arduino.cc/en/Tutorial/YunDatalogger
   
 */
//If you use Dragino IoT Mesh Firmware, uncomment below lines.
//For product: LG01. 
#define BAUDRATE 115200

//If you use Dragino Yun Mesh Firmware , uncomment below lines. 
//#define BAUDRATE 250000

#include <FileIO.h>
#include <Console.h>


void setup() {
  // Initialize the Console
  Bridge.begin(BAUDRATE);
  Console.begin();
  FileSystem.begin();

  while(!Console);  // wait for Serial port to connect.
  Console.println("Filesystem datalogger\n");
}


void loop () {
  // make a string that start with a timestamp for assembling the data to log:
  String dataString;
  dataString += getTimeStamp();
  dataString += " , ";

  // read three sensors and append to the string:
  for (int analogPin = 0; analogPin < 3; analogPin++) {
    int sensor = analogRead(analogPin);
    dataString += String(sensor);
    if (analogPin < 2) {
      dataString += ",";  // separate the values with a comma
    }
  }

  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  // The FileSystem card is mounted at the following "/mnt/FileSystema1"
  File dataFile = FileSystem.open("/mnt/sda1/data/datalog.csv", FILE_APPEND);

  // if the file is available, write to it:
  if (dataFile) {
    dataFile.println(dataString);
    dataFile.close();
    // print to the serial port too:
    Console.println(dataString);
  }  
  // if the file isn't open, pop up an error:
  else {
    Console.println("error opening datalog.csv");
  } 
  
  delay(15000);

}

// This function return a string with the time stamp
// LG01 will call the Linux "date" command and get the time stamp
String getTimeStamp() {
  String result;
  Process time;
  // date is a command line utility to get the date and the time 
  // in different formats depending on the additional parameter 
  time.begin("date");
  time.addParameter("+%D-%T");  // parameters: D for the complete date mm/dd/yy
                                //             T for the time hh:mm:ss    
  time.run();  // run the command

  // read the output of the command
  while(time.available()>0) {
    char c = time.read();
    if(c != '\n')
      result += c;
  }
  
  return result;
}
