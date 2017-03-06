/*
  Write Default Parameters for LoRa Radio :
  Support Devices: LoRa Mini, LoRa Bee Pro 
  
  Example sketch showing how to write the default Radio parameters to the MCU EEPROM
  These parameters are required for running the UART_Passthrough Sketch. 

  modified 2016-Mar-3
  by Dragino Tech <support@dragino.com>
  Dragino Technology Co., Limited
*/

#include <EEPROM.h>

#define POWER_ADDRESS 6 //EEPROM Address to store transmit power 
#define BAUD_ADDRESS 7 // EEPROM address to store uart baudrate
#define ID_ADDRESS 8 // EERPOM address to store device ID. Not used in the Passthrough sketch. reserver for further used
int  baud;
int  power ;
int  id = 1;
// Set encryption key. devices can only communicate with other devices with the same key
unsigned char encryptkey[16]={1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
union data  // data to store the defaut configure for LoRa wireless
{
  float f;   // f to store frequency
  byte c[4]; // temperory array. 
  int e[4];  // 
};
data module_config;
int addr = 0;

void setup()
{
  
  module_config.f = 868.13;  // set default frequency
  power=15;    // set default power 
  baud= 4;     // set baud rate to 4 , which is 9600 . see the passthrough sketch for detail. 
  for(int i=0;i<4;i++)
  {
    EEPROM.write(i, module_config.c[i]);  //  write Frequency to EEPROM 
  }
    EEPROM.write(POWER_ADDRESS, power);  //  write power to EEPROM 
    EEPROM.write(BAUD_ADDRESS, baud);    //  write baud rate to EEPROM 
    EEPROM.write(ID_ADDRESS, id);  //  write ID to EEPROM 
  for(int S = 0; S < 16 ;S++ )
  {
    EEPROM.write(S+18, encryptkey[S]);   //  write Encryption key to EEPROM 
  }
}

void loop()
{
}
