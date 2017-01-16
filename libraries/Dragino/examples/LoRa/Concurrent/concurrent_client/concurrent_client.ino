/*
  example: concurrent_client
  Support Devices: 
    1)LoRa Shield + Arduino
    2)LoRa GPS Shield + Arduino 

  Please use write_client_id to set different ID to clients before using this sketch. otherwise, there will 
  be conflict in the server side. 
    
  modified 24 Dec 2016
  by Dragino Tech <support@dragino.com>
  Dragino Technology Co., Limited
*/

#include <SPI.h>
#include <RH_RF95.h>
#include <String.h>
#include <EEPROM.h>
#include "encrypt.h"

RH_RF95 rf95;
ENCRYPT encrypt_decrypt;

//Define the timeout to re-start to listen the broadcast info from server to establish network.
//Default: 10 minutes 
#define TIMEOUT 600000

//Define the LoRa frequency use for this client
float frequency = 868.0;

//Define the encrypt encryptkey. so different group of LoRa devices won't communicate with each other.
unsigned char encryptkey[16]={0x01,0x23,0x45,0x67,0x89,0xAB,0xCD,0xEF,0x01,0x23,0x45,0x67,0x89,0xAB,0xCD,0xEF};

// Client ID address in EEPROM.
#define ID_ADDRESS 0x00


int sent_count = 0;//Client send count, increase after sent data. 
int client_id = 0;

uint16_t crcdata = 0; // crc data
uint16_t recCRCData = 0; // 
int detected = 0; // check if detected server's broadcast message
int flag = 0; // 
long start = 0;
long total_time = 0;//check how long doesn't receive a server message
void setup() 
{
    Serial.begin(9600);
    if (!rf95.init())
      Serial.println("init failed");
      
    //Setup ISM frequency
    rf95.setFrequency(frequency);
    // Setup Power,dBm
    rf95.setTxPower(13);
    client_id = EEPROM.read(ID_ADDRESS);//Get Client id.     
}

uint16_t calcByte(uint16_t crc, uint8_t b)
{
    uint32_t i;
    crc = crc ^ (uint32_t)b << 8;
    for ( i = 0; i < 8; i++)
     {
       if ((crc & 0x8000) == 0x8000)
       crc = crc << 1 ^ 0x1021;
       else
       crc = crc << 1;
     }  
    return crc & 0xffff;
}

uint16_t CRC16(uint8_t *pBuffer,uint32_t length)
{
    uint16_t wCRC16=0;
    uint32_t i;
    if (( pBuffer==0 )||( length==0 ))
    {
      return 0;
    }
    for ( i = 0; i < length; i++)
    { 
      wCRC16 = calcByte(wCRC16, pBuffer[i]);
    }
    return wCRC16;
}

uint16_t recdata( unsigned char* recbuf,int Length)
{
    crcdata = CRC16(recbuf,Length-2);//Calculate the CRC for the received message
    recCRCData = recbuf[Length-1];//get the CRC high byte 
    recCRCData = recCRCData<<8;// get the CRC low byte
    recCRCData |= recbuf[Length-2];//get the receive CRC
}

void listen_server(void)
{  
    if (rf95.waitAvailableTimeout(100))
    { 
       Serial.println("Get Message. ");
       uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];//buffer to store the server response message
       uint8_t len = sizeof(buf);// data length
       if (rf95.recv(buf, &len))//check if receive data is correct 
       {    
            Serial.println("waiting for broadcast message");          
            encrypt_decrypt.btea(buf, -len, encryptkey);// decode receive message
            Serial.println(buf[0]);
            Serial.println(buf[1]);
            Serial.println(buf[2]);
            Serial.println(buf[3]);
            if(buf[0] == 'B' && buf[1] == 'C' && buf[2] == ':' && buf[3] == 255  )//Get Broadcast message from Server, send a join request                                                                                          
            {
               int delay_ms = 0;
               delay_ms= random(100, 500);//generate a random delay. this is to avoid the channel congestion.This may happen when all clients start 
                                          //to send the join message to the server at the same time after get the broadcast message
               delay(delay_ms);

               uint8_t join[4] = {0}; // Construct a join message
               join[0] = 'J';
               join[1] = 'R';
               join[2] = ':';
               join[3] = client_id;// Put Client ID
               Serial.print("Send a Join Request Message, Client ID is:");
               Serial.println(join[3]);
               int length = sizeof(join);//get data length 
               encrypt_decrypt.btea(join, length, encryptkey);//encrypt the outgoing message.
               rf95.send(join, sizeof(join));// Send a Join Message
               rf95.waitPacketSent();// wait for send finished 
               flag = 1;
            }
           if(flag == 1) 
            {
                Serial.println("send Join request,waiting for Join ACK");
                Serial.println(buf[0]);
                Serial.println(buf[1]);
                Serial.println(buf[2]);
                Serial.println(buf[3]);
               if(buf[0] == 'J' && buf[1] == 'A'  && buf[2] == ':' && buf[3] == client_id)  //successful if get join ACK, otherwise, listen for broadcast again. 
                {
                  Serial.println("Get Join ACK, Join successfulm entering polling mode");
                  detected = 1;
                  flag = 0;     
               }
            }
       }
    } 
   delay(100);
}

 void polling_detect(void)
 {
   // detect if there is timeout to get response from server.
   if (rf95.waitAvailableTimeout(500))//check if there is polling request
    { 
      Serial.println("Get Message. ");
      start = millis( );
      uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];//get message 
      uint8_t len = sizeof(buf);//get message length
      if (rf95.recv(buf, &len))//check if receive message is correct
      { 
        encrypt_decrypt.btea(buf, -len, encryptkey);//decode
        Serial.println(buf[0]);
        Serial.println(buf[1]);
        Serial.println(buf[2]);
        Serial.println(buf[3]);
    
       if(buf[0] == 'D' && buf[1] == 'R' && buf[2] == ':' && buf[3] == client_id )//check if we receive a data request message
       {                        
        sent_count++;
        char  data[50] = {0};//data to be sent
        data[0] =  'D';
        data[1] =  'S';
        data[2] = ':';
        data[3] = client_id;//put client ID
        data[4] = ':';
        data[5] = sent_count;//increase after sent

        int dataLength = strlen(data);//get data length
        uint16_t crcData = CRC16((unsigned char*)data,dataLength);//calculate CRC
         
        unsigned char sendBuf[50]={0};
        strcpy((char*)sendBuf,data);//copy data to sendbuf
        
        sendBuf[dataLength] = (unsigned char)crcData;
     
        sendBuf[dataLength+1] = (unsigned char)(crcData>>8);

        int length = strlen((char*)sendBuf);//get data length 
        encrypt_decrypt.btea(sendBuf, length, encryptkey);//encryption
        
        rf95.send(sendBuf, strlen((char*)sendBuf));//send message
        rf95.waitPacketSent();//wait till send finished 
        detected = 1; 
        total_time = 0;  
       }
     else
      {
        //Check how long we have not received a data request.Client will enter into listening mode if timeout
        Serial.println("Get message, but not data request message");
        total_time += millis( )-start;//get total time out
        if(total_time > TIMEOUT)
         {
           detected = 0;
           total_time = 0;
           Serial.println("polling listening time out, listening network set up again");
         }
       }
     }
   else
    {
      Serial.println("No reply, is rf95_server running?");//didn't get a LoRa message. 
    }
  }  
}

void loop()
{   
    Serial.println("Start concurrent client: ");    
    Serial.print("Encrypt encryptkey :");
    Serial.println((char*)encryptkey);     
    Serial.print("Frequency :");
    Serial.println(frequency);           
    Serial.print("Client ID :");
    Serial.println(client_id);      
    if(detected == 0)//has not joined to the LoRa Network, listen the broadcast. 
    {
    //detect if there is server broadcast package and join the LoRa Network
      listen_server();   
    }
   else
   {
      polling_detect();
   }
   delay(100);
}

