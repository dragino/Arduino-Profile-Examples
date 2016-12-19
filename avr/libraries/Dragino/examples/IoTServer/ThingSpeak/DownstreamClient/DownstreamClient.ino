/*
  Get Data from LoRa Server
  Support Devices: LoRa Shield + 1 Relay 
  
  Example sketch showing how to get data from LoRa Server 
  Then control a relay
  
  It is designed to work with the other sketch DownstreamServer. 

  modified 2016-Dec-9
  by Edwin Chen <support@dragino.com>
  Dragino Technology Co., Limited
*/

#include <SPI.h>
#include <RH_RF95.h>


RH_RF95 rf95;
int gateway_id = 255;
float frequency = 868.0;

void setup() 
{
    Serial.begin(9600);
    if (!rf95.init())
        Serial.println("init failed");  
        
    // Setup ISM frequency
    rf95.setFrequency(frequency);
    // Setup Power,dBm
    rf95.setTxPower(13);
    Serial.println("Start Listening from Server...:"); 
}

void loop()
{
    if (rf95.waitAvailableTimeout(500))//Check Received LoRa message
    {
        // Should be a message for us now   
        uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
        uint8_t len = sizeof(buf);
        if (rf95.recv(buf, &len))
        {
            if (buf[0] == gateway_id ) // check if receive package is from gateway
            {
                int a = buf[1]-48;  // check if the receive data is 0 or 1. 
                if(a == 1)//Open Relay
                {
                  pinMode(4, OUTPUT);
                  digitalWrite(4, HIGH);
                  Serial.println("Open Relay:");
                }
                if(a == 0)//close
                {
                    pinMode(4, OUTPUT);
                    digitalWrite(4, LOW);
                    Serial.println("Close Relay:");
                }
            }
        }
        else
        {
          Serial.println("recv failed");
        }
    }
}


