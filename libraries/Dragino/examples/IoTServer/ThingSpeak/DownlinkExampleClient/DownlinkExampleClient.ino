/*
  Get Data from LoRa Gateway
  Support Devices: LoRa Shield + Arduino + 1 x Relay 
  
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
float frequency = 868.0;
char node_id[3] = {1,1,1}; //LoRa End Node ID

void setup() 
{
    Serial.begin(9600);
    if (!rf95.init())
        Serial.println("init failed");  
        
    // Setup ISM frequency
    rf95.setFrequency(frequency);
    // Setup Power,dBm
    rf95.setTxPower(13);
    Serial.println("Start listening message from gateway:");
    Serial.print("Monitor Frequency at: ");   
    Serial.print(frequency);  
    Serial.println("Mhz");       
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
            Serial.println("");
            Serial.print("======== Got Message from Gateway: ");//print reply
            for(int i=0;i< len;i++)
            {
              Serial.print(buf[i],DEC);
              Serial.print(" ");
            }
            Serial.println("");
            if((buf[0]-48) == node_id[0] && (buf[1]-48) == node_id[2] && (buf[2]-48) == node_id[2] ) // Check if reply message has the our node ID
            {
                Serial.println("Node ID match, this message is for us");
                int ctl_pin;
                if (buf[3] == 48 || buf[3] == 49 ) // for digital pin 0~9
                {
                   if (buf[3] == 48) ctl_pin = buf[4] - 48;
                   if (buf[3] == 49) ctl_pin = buf[4] - 48 + 10;
                   pinMode(ctl_pin, OUTPUT);
                   Serial.print("The pin to be controlled is:");
                   Serial.println(ctl_pin);
                   if( buf[5] == 49)//Open Relay
                   {
                      digitalWrite(ctl_pin, HIGH);
                      Serial.println("Set pin to HIGH.");
                    }
                    else if ( buf[5] == 48)
                    {
                      digitalWrite(ctl_pin, HIGH);
                      Serial.println("Set pin to LOW.");
                    }           
                }
            }
            else 
              Serial.println("Node ID dismatch, ignore message");
        }
        else
        {
          Serial.println("recv failed");
        }
    }
}


