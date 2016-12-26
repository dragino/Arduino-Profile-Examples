/*
 This example shows how to get components lastest value and show in the LG01 console
 The lastest value include LED state, Sampling Frequency, Temperature-C and Temperature-F. The LG01 will turn on/off its heart LED according
 to the LED state

 A full "how to" guide for how to set up GroveStram can be found at https://www.grovestreams.com/developers/getting_started_arduino_yun_command.html
 Detail of GroveStreams API: https://www.grovestreams.com/developers/api.html
 
 GroveStreams Setup:
 * Sign Up for Free User Account - https://www.grovestreams.com
 * Create a GroveStreams organization while selecting the Arduino blueprint
 * Enter a unique MAC Address for this network in this sketch under "Local Network Settings"
 *    Navigate to http://your_dragino_ip to find your dragino's MAC address
 * Enter your GroveStreams secret api key under "GroveStreams Settings" in this sketch  
 *    (The api key can be retrieved from within a GroveStreams organization: click the Api Keys toolbar button,
 *     select your Api Key, and click View Secret Key. Paste the Secret Key below)
 
 modified 15 Dec 2016
 by Edwin Chen <support@dragino.com>
 Dragino Technology Co., Limited 
*/
 
#include <Bridge.h>
#include <Console.h>
#include <Process.h>
#include <string.h>
#include <stdio.h>
//#include <ArduinoJson.h>  // Installed this in Arduino Library Manager

//If you use Dragino IoT Mesh Firmware, uncomment below lines.
//For product: LG01. 
#define BAUDRATE 115200

//define LG01 LED
int HEART_LED=A2;
 
// Local Network Settings
char mac[] = "A8:40:41:16:8B:2E"; // Change this!!! Must be unique on local network.
                                  // Browse to your dragino to find it: http://your_device_ip
 
// GroveStreams Settings
char gsApiKey[] = "8a8b2102-a7c2-332d-a848-cc4a0ea8b5c4";   //The API key must with auto-registration and with stream request right.
 

// Other Settings
const unsigned long GETFrequency = 30000UL;    // GET frequency in milliseconds (20000 = 20 seconds). Change this to change your GET frequency.
char response[255] = {0}; //Initialize buffer to nulls; 
unsigned long lastGETTime = 0; //Don't change. Used to determine if need to GET
String   stringOne = ""; 
void setup()
{
 
  Bridge.begin(BAUDRATE);
  
  // Open Serial monitor and start here.
  Console.begin();
  while(!Console);
 
  //Wait for the connection to finish stopping
  delay(2500);
 
  //Set the mac and ip variables so that they can be used during sensor uploads later
  Console.print("MAC: ");
  Console.println(mac);

  // initialize digital pin  as an output.
  pinMode(HEART_LED, OUTPUT);
  digitalWrite(HEART_LED, HIGH);   // turn the HEART_LED on (HIGH is the voltage level)
}
 
void GETCompStreamsLastSamples()
{
  Console.println("GET Components Streams Last Samples ...");
 
  //Assemble the url that is used to get streams from gs
  lastGETTime = millis();
 
  //You may need to increase the size of urlBuf if any other char array sizes have increased
  char urlBuf[180];
  sprintf(urlBuf, "http://grovestreams.com/api/comp/%s/last_value?retStreamId&api_key=%s", mac,gsApiKey);
                 
  char xHeadBuf[40];
  sprintf(xHeadBuf, "X-Forwarded-For:%s\0", mac);
                 
  //Uncomment the following lines to debug your strings and to ensure their buffers are large enough
  //Console.println(strlen(xHeadBuf));
  //Console.println(xHeadBuf);
  //Console.println(urlBuf);
  //Console.println(strlen(urlBuf));
  //Console.println(getSamples());
  //Console.println(strlen(getSamples()));
 
  Process process;
  process.begin("curl");
  process.addParameter("-k");
  process.addParameter("-X");
  process.addParameter("GET");
 
  //Headers
  process.addParameter("-H");
  process.addParameter(xHeadBuf);
  process.addParameter("-H");
  process.addParameter("Connection:close");                      
  process.addParameter("-H");
  process.addParameter("Content-Type:application/json");  
 
  //URL
  process.addParameter(urlBuf);
 
  //Make the request
  unsigned int result = process.run();
  int i = 0;
  //Display whatever is returned (usually an error message if one occurred)
  while (process.available()>0) {
    char c = process.read();
    if (i < 255) {
      response[i++] = c;
    } 
    else if (i == 255) {
      Console.println("response string buffer too small!");
    }
    //Console.print(c);
  }

  Console.flush();
  
  Console.println(response);
  Console.print("Feed GET Result: ");
  Console.println(result);

  //delay(5000);

  Console.println("");
  Console.println("");
}
 
void loop()
{
 
  // Update sensor data to GroveStreams
  if(millis() - lastGETTime > GETFrequency)
  {
    GETCompStreamsLastSamples();

    char response1[] = "ledfalsetrue";
    char *str1 = "led";
    char *Dat = NULL;
    char *p = NULL;
    char *q = NULL;
    char str [20] = {0};//存放单个对象,长度20
    Dat = strstr(response,str1); 
    if(Dat != NULL)
    {
      Dat += 18;//指针向后偏移18个字节；
      int n = 0;
      for(n = 0 ;n < 20 ; n++)//提取对象
        {
        str [n]= *Dat;
          Dat++;
        }
        str [n] = '\0';    
        p = strstr(str,"false") ;
        q = strstr(str,"true") ;
     }
   if( p )
    {
      digitalWrite(HEART_LED, HIGH);
    }
   if( q )
    {
      digitalWrite(HEART_LED, LOW);
     // Console.println("Turn Off LED");
    }
    Console.print("flase:");
    Console.println( p);
    Console.print("turn:");
    Console.println(q);
  }
}
                    
