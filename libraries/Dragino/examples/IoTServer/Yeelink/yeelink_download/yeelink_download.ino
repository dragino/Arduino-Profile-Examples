/*
Downlink --> LG01 via Wi-Fi
 
 The TalkBack App sketch is designed for LG01 connected to a
 Wi-Fi network and the Arduino 1.5.4 IDE or newer. This sketch allows the
 LG01 to request commands stored by a Yeelink downlink via the
 Yeelink API (http://www.yeelink.net/developer/api).
  
 Getting Started with Yeelink:
 
   * Sign Up for New User Account - http://www.yeelink.net/login
   * Create a new device
   * Create a new sensor
 
 Arduino Requirements:
 
   * Dragino LG01
   * Arduino 1.5.4 IDE or newer
   
  Network Requirements:
   * Router with Wi-Fi
   * DHCP enabled on Router
 
 Created: December 20, 2016 by Dragino (http://wiki.dragino.com)
 
 Additional Credits:
 Example sketches from Arduino team, Yeelink, ThingSpeak and Yun Example
 
 
 by Dragino Technology(hhtp://www.dragino.com)
 
*/

//If you use Dragino IoT Mesh Firmware, uncomment below lines.
//For product: LG01. 
#define BAUDRATE 115200

#include "Bridge.h"
#include "HttpClient.h"
#include <Console.h>

//Yeelink Settings 
String YeelinkAPI = "api.yeelink.net";
String deviceID = "353392";
String sensorID = "398439";
const int checkTalkBackInterval = 3 * 1000;    // Time interval in milliseconds to check TalkBack (number of seconds * 1000 = interval)
char response[60] = {0};

// Variable Setup
long lastConnectionTime = 0;

void setup()
{
  // Setup On-board LED
  pinMode(A2, OUTPUT);
  digitalWrite(A2, LOW);
  delay(1000);
  digitalWrite(A2, HIGH);
  delay(1000);
  digitalWrite(A2, LOW);
  
  // Initialize Bridge
  Bridge.begin(BAUDRATE);
  
  // Initialize Console
  Console.begin();
  while(!Console);
}

void loop()
{  
  // Check Yeelink for downlink Commands
  checkTalkBack();
  delay(checkTalkBackInterval);
}

void checkTalkBack()
{
  HttpClient client;
  char charIn;
  int i = 0;
  String talkBackURL =  "http://" + YeelinkAPI + "/v1.0/device/" + deviceID + "/sensor/" + sensorID + "/datapoints";  
  // Make a HTTP GET request to the Yeelink API:
  client.get(talkBackURL);
    
  while (client.available()) 
  {
    charIn = client.read();
    
     if (i < 60) 
     {
      response[i++] = charIn;
     } 
     else if (i == 60) 
     {
      Console.println("response string buffer too small!");
     }
  }
  //Console.println(response);


  char *str1 = "value";
  char *Dat = NULL;
  char *p = NULL;
  char *q =NULL;
  char str [10] = {0};

  Dat = strstr(response,str1);
  Console.println(Dat);

  if(Dat !=NULL)
   {
    Dat +=7;
    int n=0;
    for(n=0 ;n<1 ; n++)
   {
    str [n] = *Dat;
    Dat++;
   }
    str [n] = '\0';
    Console.println(str[0],DEC);
   }
  
  // Turn On/Off the On-board LED
   if (str[0] == 49)
   {  
    Console.println("turn on");
    digitalWrite(A2, HIGH);
   }
    else if (str[0] == 48)
  {      
    Console.println("turn off");
    digitalWrite(A2, LOW);
  }
  
 Console.flush(); 
 delay(1000);
}
