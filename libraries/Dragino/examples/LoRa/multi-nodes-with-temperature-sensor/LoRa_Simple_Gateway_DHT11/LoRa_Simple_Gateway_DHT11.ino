/*
  LoRa Simple Arduino Server :
  Support Devices: 
  * LoRa Mini Dev
  * LoRa Shield + Arduino;
  * LG01

  Software Requirement:
  1/ Install the Radiohead Library(http://www.airspayce.com/mikem/arduino/RadioHead/) to Arduino. 
  
  Example sketch showing how to create a simple messageing gateway,
  This sketch will get the data from LoRa Node and print out the data
  Connection and result please refer: http://wiki.dragino.com/index.php?title=LoRa_Mini#Example_2:_Multi_LoRa_nodes_simple_connection_--_RadioHead_Lib 

  It is designed to work with the other example LoRa_Simple_Client_DHT11

  modified 25 Mar 2017
  by Dragino Tech <support@dragino.com>
  Dragino Technology Co., Limited
*/

#include <SPI.h>
#include <RH_RF95.h>

//When use LG01, uncomment this line, so print the result in Console. 
//When use LoRa Mini Dev, Comment this link
//#define  LG01_GATEWAY  

#ifdef LG01_GATEWAY
#include <Console.h>
#include <Process.h>
#define BAUDRATE 115200 
#define SerialPrint Console
#else
#define SerialPrint Serial
#endif

float frequency = 868.0;// Change the frequency here.

RH_RF95 rf95;
void setup() 
{
  #ifdef LG01_GATEWAY
    Bridge.begin(BAUDRATE);
    SerialPrint.begin();
  #else
    SerialPrint.begin(9600);
  #endif 
  
  if (!rf95.init())
    {
      SerialPrint.println("LoRa module init failed, Please cehck hardware connection");
      while(1) ;
    }
    
  // Setup ISM frequency
  rf95.setFrequency(frequency);
  // Setup Power,dBm
  rf95.setTxPower(13);
  // Defaults BW Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on
  SerialPrint.print("Listening on frequency: ");
  SerialPrint.println(frequency);
}

void loop()
{
  if (rf95.available())
  {
    // Should be a message for us now   
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);
    if (rf95.recv(buf, &len))
    {
     if(buf[0] == 1 & buf[1] == 1 & buf[2] ==1) //Get sensor data from node id 111
      {
        int newData[4] = {0, 0, 0, 0};
        for (int i = 0; i < 2; i++)
        {
          newData[i] = buf[i + 3];
        }
        int a = newData[0];
        int b = newData[1];
        SerialPrint.print("ID = 111 :");
        SerialPrint.print("Current humdity = ");
        SerialPrint.print(a);
        SerialPrint.print("%  ");
        SerialPrint.print("temperature = ");
        SerialPrint.print(b);
        SerialPrint.println("C  ");
      }
     if(buf[0] == 1 & buf[1] == 1 & buf[2] == 2) //Get sensor data from node id 112
      {
        int newData[4] = {0, 0, 0, 0};
        for (int i = 0; i < 2; i++)
        {
          newData[i] = buf[i + 3];
        }
        int a = newData[0];
        int b = newData[1];
        SerialPrint.print("ID = 112 :");
        SerialPrint.print("Current humdity = ");
        SerialPrint.print(a);
        SerialPrint.print("%  ");
        SerialPrint.print("temperature = ");
        SerialPrint.print(b);
        SerialPrint.println("C  ");
      }
    }
    else
    {
        SerialPrint.println("recv failed");
    }
  }
}


