/*
 
 Downlink Example for Emoncms Server
   
 Arduino Requirements:
 
   * Dragino LG01
   * Arduino 1.5.4 IDE or newer
   
  Network Requirements:
   * Router with Wi-Fi
   * DHCP enabled on Router
 
 Created: December 20, 2016 by Dragino (http://wiki.dragino.com)
 
 by Dragino Technology(hhtp://www.dragino.com)
 
*/

#include <Bridge.h>
#include <Console.h>
#include <Process.h>


//If you use Dragino IoT Mesh Firmware, uncomment below lines.
//For product: LG01. 
#define BAUDRATE 115200
char data[3] = "1";
 char Dat[1] = {0} ;

// Emoncms Settings
char feedID[] = "148847";   //Emoncms feed ID

// Other Settings
const unsigned long GETFrequency = 30000UL;    // GET frequency in milliseconds (20000 = 20 seconds). Change this to change your GET frequency.
char response[3] = {0}; //Initialize buffer to nulls; 
unsigned long lastGETTime = 0; //Don't change. Used to determine if need to GET
 
void setup()
{
 
  Bridge.begin(BAUDRATE);
  
  // Open Serial monitor and start here.
  Console.begin();
  while(!Console);
 
  //Wait for the connection to finish stopping
  delay(2500);
  // initialize digital pin  as an output.
  pinMode(A2, OUTPUT);
  digitalWrite(A2, HIGH);   // turn the A2 on (HIGH is the voltage level)
}
 
void GETCompStreamsLastSamples()
{
  Console.println("GET Components Streams Last Samples ...");
 
  //Assemble the url that is used to get streams from gs
  lastGETTime = millis();
 
  //You may need to increase the size of urlBuf if any other char array sizes have increased
  char urlBuf[180];
  sprintf(urlBuf, "https://www.emoncms.org/feed/value.json?id=%s",feedID);
 
  Process process;
  process.begin("curl");
  process.addParameter("-k");
 // process.addParameter("-X");
  //process.addParameter("GET");
 
  //Headers
 // process.addParameter("-H");
 // process.addParameter(xHeadBuf);
 // process.addParameter("-H");
  //process.addParameter("Connection:close");                      
 // process.addParameter("-H");
  //process.addParameter("Content-Type:application/json");  
 
  //URL
  process.addParameter(urlBuf);
 
  //Make the request
  unsigned int result = process.run();
  
  int i = 0;
  //Display whatever is returned (usually an error message if one occurred)
  while (process.available()>0) {
    char c = process.read();
    if (i < 3 )
    {
      response[i++] = c;
    } 
    else if (i == 3)
    {
      Console.println("response string buffer too small!");
    }
    //Console.print(c);
  }
  int a = 0 ;
  for(  a = 0 ; a < sizeof(response) ; a++)
     {
       // Console.print( response[a]);
       if( response[a] == 48 || response[a] == 49)
       {
         Dat[0] = response[a]-48 ; 
       }      
     }
 // Console.flush();
    if (Dat[0] == 1)
  {  
    Console.print("turn on led");
    digitalWrite(A2, HIGH);
  }
  else if (Dat[0] == 0)
  {      
    Console.print("turn off led ");
    digitalWrite(A2, LOW);
  }
  
  //Console.println(Dat[0]);
  //Console.println(response[1],DEC);
  Console.print("Feed GET Result: ");
  Console.println(result);
}
void loop() {
 
  // Update sensor data
  if(millis() - lastGETTime > GETFrequency)
  {
    GETCompStreamsLastSamples();
  }
}

