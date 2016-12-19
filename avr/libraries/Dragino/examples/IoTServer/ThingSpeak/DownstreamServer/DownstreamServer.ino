/*
  Get Data from IoT Server ThingSpeak (https://thingspeak.com/):
  Support Devices: LG01 
  
  Example sketch showing how to get data from IoT Server 
  Then send the value to a LoRa Node
  
  It is designed to work with the other sketch DownstreamClient. 

  modified 24 11 2016
  by Edwin Chen <support@dragino.com>
  Dragino Technology Co., Limited
*/

#include <SPI.h>
#include <RH_RF95.h>
#include <Console.h>
#include <Process.h>
#include <String.h>
#include<EEPROM.h>
//#include "ThingSpeak.h"
//#include "YunClient.h"
#include "HttpClient.h"
//YunClient client;
//HttpClient client;
RH_RF95 rf95;

//If you use Dragino IoT Mesh Firmware, uncomment below lines.
//For product: LG01. 
#define BAUDRATE 115200

String thingSpeakAPI = "api.thingspeak.com";
String talkBackAPIKey = "VJ183E4EG4GYWL4Y";
String talkBackID = "12095";
const int checkTalkBackInterval = 10 * 1000;    // Time interval in milliseconds to check TalkBack (number of seconds * 1000 = interval)

int gateway_id = 0;
int address = 0;
// Variable Setup
long lastConnectionTime = 0;
int talkBackCommand;
int charIn = 0;  
float frequency = 868.0;

void setup()
{
    Bridge.begin(BAUDRATE);
    Console.begin();
    //ThingSpeak.begin(client);
    if (!rf95.init())
        Console.println("init failed");  
    // Setup ISM frequency
    rf95.setFrequency(frequency);
    // Setup Power,dBm
    rf95.setTxPower(13);
}

void loop()
{  
    // Check ThingSpeak for TalkBack Commands
    checkTalkBack();//Check if there is talkback command
    delay(checkTalkBackInterval); 
     
}

void checkTalkBack()//Check if there is talkback command
{
    HttpClient client;
    
    String talkBackURL =  "http://" + thingSpeakAPI + "/talkbacks/" + talkBackID + "/commands/execute?api_key=" + talkBackAPIKey;
    
    // Make a HTTP GET request to the TalkBack API:
    client.get(talkBackURL);
    charIn = 0;
    talkBackCommand = 0;
    
    while (client.available())
    {
        charIn = client.read();//
        talkBackCommand += charIn;
    }

    Console.print("Get Response from Server: ");
    Console.println(talkBackCommand);
    if (talkBackCommand != 0)
    {
        gateway_id = EEPROM.read(address);//Get Local ID from ATMEGA328P, if not set, it is 255.
        unsigned char data[50] = {0} ;//The data to be sent to LoRa Node 
        data[0] = gateway_id;
        data[1] = charIn;
        Console.println("_____________________________________" );
        Console.println(data[0] ,DEC);
        Console.println(data[1] ,DEC);
        rf95.send(data, strlen((char*)data)); //Send data to LoRa Node.
        rf95.waitPacketSent();  //wait for sending
        
        Console.flush(); 
    }
    delay(1000);
}




