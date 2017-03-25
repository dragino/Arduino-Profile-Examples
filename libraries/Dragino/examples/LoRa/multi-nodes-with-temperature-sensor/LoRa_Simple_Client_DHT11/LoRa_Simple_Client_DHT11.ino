/*
  LoRa_Simple_Client_DHT11 for Arduino:
  Support Devices: 
  1/ LoRa Shield + Arduino + DHT11 Temperature Sensor
  2/ LoRa mini / LoRa mini dev + DHT11 Temperature Sensor

  Hardware Connection:
  1/ Connect DHT11 vcc to 3.3v 
  2/ Connect DHT11 GND to LoRa mini dev GND
  3/ Connect DHT11 Data pin to LoRa mini dev A0 pin

  Software Requirement:
  1/ Install the Radiohead Library(http://www.airspayce.com/mikem/arduino/RadioHead/) to Arduino. 
  
  Example sketch showing how to get temperature and humidity value and send to 
  LoRa Gateway: Detail refer
  http://wiki.dragino.com/index.php?title=LoRa_Mini#Example_2:_Multi_LoRa_nodes_simple_connection_--_RadioHead_Lib

  It is designed to work with the other example LoRa_Simple_Gateway_DHT11

  modified 25 MAr 2017
  by Dragino Tech <support@dragino.com>
  Dragino Technology Co., Limited
*/

#include <SPI.h>
#include <RH_RF95.h>
#include <String.h>

RH_RF95 rf95;
float frequency = 868.0; // Change the frequency here. 

#define dht_dpin A0 // Use A0 pin to connect the data line of DHT11
byte bGlobalErr;
char dht_dat[5];

void setup()
{
   InitDHT();
   Serial.begin(9600);
   while (!Serial) ; // Wait for serial port to be available
   Serial.println("LoRa_Simple_Client_DHT11");
   if (!rf95.init())
     Serial.println("init failed");
   Serial.println("Humidity and temperature\n\n"); 
  // Setup ISM frequency
   rf95.setFrequency(frequency);
   // Setup Power,dBm
   rf95.setTxPower(13);
}
void InitDHT() // Initiate DHT11
{
    pinMode(dht_dpin,OUTPUT);//
    digitalWrite(dht_dpin,HIGH);//Set A0 to output high
}

void ReadDHT() // Read Temperature and Humidity value 
{ 
    bGlobalErr=0;
    byte dht_in;
    byte i;
    
     //pinMode(dht_dpin,OUTPUT);
    digitalWrite(dht_dpin,LOW);//pull low data line to send signal.
    delay(30);//Add a delay higher than 18ms so DHT11 can detect the start signal
    
    digitalWrite(dht_dpin,HIGH);
    delayMicroseconds(40);
    pinMode(dht_dpin,INPUT);
    // delayMicroseconds(40);
    dht_in=digitalRead(dht_dpin);//Get A0 state
    //   Serial.println(dht_in,DEC);
    if(dht_in){
       bGlobalErr=1;
       return;
       }
    delayMicroseconds(80);//Get DHT11 response , pull low data lineDHT11
    dht_in=digitalRead(dht_dpin);
    
    if(!dht_in){
       bGlobalErr=2;
       return;
       }
    delayMicroseconds(80);//
    for (i=0; i<5; i++)//Ger Temperature and Humidity value
       dht_dat[i] = read_dht_dat();
           
    pinMode(dht_dpin,OUTPUT);
    digitalWrite(dht_dpin,HIGH);
    byte dht_check_sum =
           dht_dat[0]+dht_dat[1]+dht_dat[2]+dht_dat[3];//Calculate check sum
    if(dht_dat[4]!= dht_check_sum)//Error when check sum dismatch
       {bGlobalErr=3;}
};

byte read_dht_dat(){
  byte i = 0;
  byte result=0;
  for(i=0; i< 8; i++){
      while(digitalRead(dht_dpin)==LOW);//
      delayMicroseconds(30);//
      if (digitalRead(dht_dpin)==HIGH)
     result |=(1<<(7-i));//
    while (digitalRead(dht_dpin)==HIGH);//
    }
  return result;
}

void loop()
{
    ReadDHT();
    uint8_t data[50] = {0} ;
    data[0] = 1 ;
    data[1] = 1 ;
    data[2] = 1 ;// Use Data [0].Data[1], Data[2] to combine a Device ID. 
    data[3] = dht_dat[0]; // store humidity data
    data[4] = dht_dat[2];// store temperature data
    rf95.send(data, sizeof(data)); // Send out ID + Sensor data to LoRa gateway
   switch (bGlobalErr)
   {
     case 0:
       Serial.print("Current humdity = ");
       Serial.print(dht_dat[0], DEC);
       Serial.print(".");
       Serial.print(dht_dat[1], DEC);
       Serial.print("%  ");
       Serial.print("temperature = ");
       Serial.print(dht_dat[2], DEC);
       Serial.print(".");
       Serial.print(dht_dat[3], DEC);
       Serial.println("C  ");
       break;
     case 1:
        Serial.println("Error 1: DHT start condition 1 not met.");
        break;
     case 2:
        Serial.println("Error 2: DHT start condition 2 not met.");
        break;
     case 3:
        Serial.println("Error 3: DHT checksum error.");
        break;
     default:
        Serial.println("Error: Unrecognized code encountered.");
        break;
    }
  delay(4000);
}
