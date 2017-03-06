/*
  UART Pass Through Examples. v0.1
  Support Devices: LoRa Mini, LoRa Bee Pro 
  
  Example sketch showing how to use UART pass through over LoRa.

  Hardware Setup:
  * Two LoRa devices connect to PC via USB. So PC can use serial see the UART output from both devices and
    able to sent data to their UART
  
  Before running this sketch:
  * Set the LoRa Radio paratemeters to the devices EEPROM via the write_eeprom sketch. both devices can use 
    the same sketch. 

  To Test:
  * Upload this sketch to both devices. Use PC to open monitor to both devices and type characters on the monitor
    the other device will receive and print the chars in its serial monitor

  More about this sketch:
  * User is able to modify the LoRa Radio pameters and write back to EEPROM in this sketch via AT COMMAND, please see Check_UART function

  modified 2016-Mar-3
  by Dragino Tech <support@dragino.com>
  Dragino Technology Co., Limited
*/

#include <SPI.h>
#include <RH_RF95.h>
#include <EEPROM.h>
#include "encrypt.h"

RH_RF95 rf95;
ENCRYPT encrypt_decrypt;

#define POWER_ADDRESS 6  //EEPROM Address to store transmit power 
#define BAUD_ADDRESS 7   // EEPROM address to store uart baudrate
#define ID_ADDRESS 8     // EERPOM address to store device ID. Not used in the Passthrough sketch. reserver for further used

int configure_mode = 0;
int baud ;
int power = 0;
int device_id = 0;
int baud_id = 4;
unsigned char encrypt_key[16] ={0};
uint8_t sbuf[255] = {0};
uint8_t dat[RH_RF95_MAX_MESSAGE_LEN] = {0};
union data
{
  float fre;
  byte FRE[4];
};
data module_config;

void setup() 
{
  for(int i=0;i<4;i++)
  { module_config.FRE[i]=EEPROM.read(i);}
  device_id = EEPROM.read(ID_ADDRESS); 
  power =  EEPROM.read(POWER_ADDRESS);
  set_baud_rate();
  for(int a = 18; a < 34 ;a++ ) // Get encrypt key
  {
   encrypt_key[a-18]=EEPROM.read(a);
  }
  Serial.begin(baud);
  if (!rf95.init())
    Serial.println("init failed");
  rf95.setTxPower(power); 
  rf95.setFrequency(module_config.fre);
  // Print the default settings here. 
  Serial.println("Ready for use ");
  Serial.print("UART Baud Rate:");
  Serial.print(baud);
  Serial.println("bps");
  Serial.print("Frequency:");
  Serial.print(module_config.fre);
  Serial.println("MHz");
  Serial.print("Transmit Power:");
  Serial.print(power);
  Serial.println("dB");
  Serial.print("Device ID:");
  Serial.println(device_id);
}
void set_baud_rate() // Get Baud Rate ID and covert it to actual UART baud rate.
{
  int baud_id =  EEPROM.read(BAUD_ADDRESS); 
  switch(baud_id)
   {
     case 1:
        baud  = 1200;
        break;
     case 2:
        baud  = 2400;
        break;
     case 3:
        baud  = 4800;
        break;
     case 4:
        baud  = 9600;
        break;
     case 5:
        baud  = 19200;
        break;
     case 6:
        baud  = 38400;
        break;
     case 7:
        baud  = 57600;
        break;
     case 8:
        baud  = 115200;
        break;
     case 9:
        baud  = 230400;
        break;
     default:
        baud  = 9600;
        break;        
    }
}
void loop()
{
  if(configure_mode == 0)
  {
    if (rf95.available())
    { 
     uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
     uint8_t len = sizeof(buf);  
     if (rf95.recv(buf, &len))
     {
        encrypt_decrypt.btea(buf,-len, encrypt_key);//decrypt the incoming message.
        Serial.print("Got message: ");
        Serial.println((char*)buf);
     }
    }
  }
  Check_UART();
  delay(400);
}
void Check_UART()
{
 while (Serial.available() > 0) 
  {
      char uart_rx =  (char)Serial.peek(); /// uart_rx stock the char comes from UART. 
      sbuf[uart_rx];
      Serial.readBytes(sbuf, uart_rx);
      for(int i = 0 ;i < uart_rx; i++)
      {
        dat[0] = device_id ;
        dat[i+1] = sbuf[i];
      }
      if(sbuf[0] == 'A' && sbuf[1] == 'T' && sbuf[2] == '+' && sbuf[3] == '+') // If user type an AT++ to UART. it will enter configure mode
      {
        //Jump = 1;
        configure_mode = 1;
        Serial.println("Enter Configure Mode,Current Settings:");
        Serial.print("UART Baud Rate: ");
        Serial.print(baud);
        Serial.println("bps");
        Serial.print("LoRa Frequency: ");
        Serial.print(module_config.fre);
        Serial.println("MHz");
        Serial.print("Trasmit Power:");
        Serial.print(power);
        Serial.println("dB");
        Serial.print("Device ID:");
        Serial.println(device_id);
      }
     if( configure_mode == 1)
     {   
       if(sbuf[0] == 'A' && sbuf[1] == 'T' && sbuf[2] == '+'&& sbuf[3] == 'F' && sbuf[4] == 'R' && sbuf[5] == 'E')//Set new frequency.
       {
          if(sbuf[7] == '"' && sbuf[13] == '"') // Frquency format: AT+FRE="xxx.xx"
          {
             long int a = (sbuf[8]-48) * 100;
             int b = (sbuf[9]-48) * 10;
             int c = (sbuf[10]-48);
             float d = (sbuf[12]-48)/10.0;
             module_config.fre = a + b + c + d ;
             Serial.println(module_config.fre);
             rf95.setFrequency(module_config.fre);
          }
          else if(sbuf[7] == '"' && sbuf[14] == '"') // Frquency format: AT+FRE="xxx.x"
          {
             long int a = (sbuf[8]-48) * 100;
             int b = (sbuf[9]-48) * 10;
             int c = (sbuf[10]-48);
             float d = (sbuf[12]-48)/10.0;
             float E = (sbuf[13]-48)/100.0;
             module_config.fre = a + b + c + d + E ;
             Serial.println(module_config.fre);
             rf95.setFrequency(module_config.fre);
          }
          else  // Frquency format: AT+FRE="xxx"
          {
             long int a = (sbuf[6]-48) * 100;
             int b = (sbuf[7]-48) * 10;
             int c = (sbuf[8]-48);
             module_config.fre = a + b + c ;
             Serial.println(module_config.fre);
             rf95.setFrequency(module_config.fre);
           }
        }
        else if(sbuf[0] == 'A' && sbuf[1] == 'T' && sbuf[2] == '+'&& sbuf[3] == 'B' && sbuf[4] == 'A' && sbuf[5] == 'U' && sbuf[6] == 'D')//Set new uart baud rate
        {
           baud_id  = sbuf[9]-48;
           switch(baud_id) // baud id: 1:1200;2:2400;3:4800;4:9600;5:19200;6:38400;7:57600;8:115200;9:230400;   Format: AT+BAUD="x" : x for baud id 
           {
             case 1:
                baud  = 1200;
                Serial.println(baud);
                break;
             case 2:
                baud  = 2400;
                Serial.println(baud);
                break;
             case 3:
               baud  = 4800;
               Serial.println(baud);
               break;
             case 4:
               baud  = 9600;
               Serial.println(baud);
               break;
             case 5:
               baud  = 19200;
               Serial.println(baud);
               break;
             case 6:
               baud  = 38400;
               Serial.println(baud);
               break;
             case 7:
               baud  = 57600;
               Serial.println(baud);
               break;
             case 8:
               baud  = 115200;
               Serial.println(baud);
               break;
             case 9:
               baud  = 230400;
               Serial.println(baud);
               break;
             default:
               baud  = 9600;
               Serial.println(baud);
               break;        
           }
         }
       else if(sbuf[0] == 'A' && sbuf[1] == 'T' && sbuf[2] == '+'&& sbuf[3] == 'P' && sbuf[4] == 'O' && sbuf[5] == 'W' && sbuf[6] == 'E'  && sbuf[7] == 'R')//Set transmit power
       {
           if(sbuf[9] == '"' && sbuf[12]) // Set power format: AT+POWER="xx" 
            {
              int a = (sbuf[10]-48) * 10;
              int b = (sbuf[11]-48);
              power = a + b;
              rf95.setTxPower(power);
            }
           else
           {
             int a = (sbuf[10]-48) * 10;  //Set power format: AT+POWER="x" 
             power = a ;
             rf95.setTxPower(power);  
            }
      }
      else if(sbuf[0] == 'A' && sbuf[1] == 'T' && sbuf[2] == '+'&& sbuf[3] == 'I' && sbuf[4] == 'D')//Set device id, format: AT+ID="x". only 1 digit at the moment. 
      {
         device_id = sbuf[7];  
      }
      else if(sbuf[0] == 'A' && sbuf[1] == 'T' && sbuf[2] == '+'&& sbuf[3] == 'E' && sbuf[4] == 'N'&& sbuf[5] == 'C' && sbuf[6] == 'R' && sbuf[7] == 'Y'&& sbuf[8] == 'P' && sbuf[9] == 'T')//Set encrypt key
      // Format: AT+ENCRYPT="xxxxxxxxxxxxxxxx" ..xx is 16 bytes. 
      {
          int len = sizeof(data);
          int i;
          for( i = 13 ;i < 29; i++)
          {
             encrypt_key[i-13] = dat[i];      
          }
          encrypt_key[i] = '\0' ;
          Serial.println((char*)encrypt_key);
       }
      else if(sbuf[0] == 'A' && sbuf[1] == 'T' && sbuf[2] == '+'&& sbuf[3] == 'Q' && sbuf[4] == 'U'&& sbuf[5] == 'I' && sbuf[6] == 'T')//Quit and not configure to EEPROM, format: AT+QUIT
      {
         configure_mode = 0;
      }
      else if(sbuf[0] == 'A' && sbuf[1] == 'T' && sbuf[2] == '+'&& sbuf[3] == 'S' && sbuf[4] == 'A'&& sbuf[5] == 'V' && sbuf[6] == 'E')//Save configure to EEPROM, not quit, format: AT+SAVE
      {
         Serial.println("Save Configure");
         Serial.println((char*)encrypt_key);
         for(int i = 0;i < 4;i++)
         {
           EEPROM.write(i, module_config.FRE[i]);  // write frequency to EEPROM
         }
         EEPROM.write(POWER_ADDRESS, power); 
         EEPROM.write(BAUD_ADDRESS, baud_id); 
         EEPROM.write(ID_ADDRESS, device_id); 
         for(int S = 0; S < 16 ;S++ )
         {
            EEPROM.write(S+18, encrypt_key[S]); 
         } 
         Serial.println("Saved Successfully");
      }
      else if( sbuf[0] == 'A' && sbuf[1] == 'T' && sbuf[2] == '+'&& sbuf[3] == 'S' && sbuf[4] == 'Q')//Save configure to EEPROM and  quit, format: AT+SQ
       {
         Serial.println("Save and exit");
         for(int i = 0;i < 4;i++)
         {
            EEPROM.write(i, module_config.FRE[i]);  // write frequency to EEPROM  
         }
         for(int S = 0; S < 16 ;S++ )
         {
            EEPROM.write(S+18, encrypt_key[S]); 
         }
         EEPROM.write(POWER_ADDRESS, power); 
         EEPROM.write(BAUD_ADDRESS, baud_id); 
         EEPROM.write(ID_ADDRESS, device_id); 
         configure_mode = 0;   // Exit Configure mode. 
         }
     }
   if( configure_mode == 0 ) // This is a normal UART message to transfer via LoRa
    {
      int length = sizeof(dat);//get data length 
      encrypt_decrypt.btea(dat, length, encrypt_key);//encrypt the outgoing message.
      rf95.send(dat,sizeof(dat));
      rf95.waitPacketSent();//Sent packet.
    }
  }
}

