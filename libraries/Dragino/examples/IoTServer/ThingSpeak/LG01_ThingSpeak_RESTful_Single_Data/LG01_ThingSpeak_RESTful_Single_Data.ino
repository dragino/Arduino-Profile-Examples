/*
  Upload Data to IoT Server ThingSpeak (https://thingspeak.com/):
  Support Devices: LG01 

  Require Library: 
  https://github.com/dragino/RadioHead
  
  Example sketch showing how to get data from remote LoRa node, 
  Then send the value to IoT Server

  It is designed to work with the other sketch dht11_client. 

  modified 24 11 2016
  by Edwin Chen <support@dragino.com>
  Dragino Technology Co., Limited
*/

#include <SPI.h>
#include <RH_RF95.h>
#include <Console.h>
#include <Process.h>
RH_RF95 rf95;

//If you use Dragino IoT Mesh Firmware, uncomment below lines.
//For product: LG01. 
#define BAUDRATE 115200

String myWriteAPIString = "P07KVY59P5QEY6M6";
uint16_t crcdata = 0;
uint16_t recCRCData = 0;
float frequency = 868.0;
String dataString = "";

void uploadData(); // Upload Data to ThingSpeak.

void setup()
{
    Bridge.begin(BAUDRATE);
    Console.begin(); 
    // while(!Console);
    if (!rf95.init())
        Console.println("init failed");
    ;
    // Setup ISM frequency
    rf95.setFrequency(frequency);
    // Setup Power,dBm
    rf95.setTxPower(13);
    rf95.setSyncWord(0x34);
    
    Console.println("LoRa Gateway Example  --");
    Console.println("    Upload Single Data to ThinkSpeak");
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

uint16_t CRC16(uint8_t *pBuffer, uint32_t length)
{
    uint16_t wCRC16 = 0;
    uint32_t i;
    if (( pBuffer == 0 ) || ( length == 0 ))
    {
        return 0;
    }
    for ( i = 0; i < length; i++)
    {
        wCRC16 = calcByte(wCRC16, pBuffer[i]);
    }
    return wCRC16;
}

uint16_t recdata( unsigned char* recbuf, int Length)
{
    crcdata = CRC16(recbuf, Length - 2); //Get CRC code
    recCRCData = recbuf[Length - 1]; //Calculate CRC Data
    recCRCData = recCRCData << 8; //
    recCRCData |= recbuf[Length - 2];
}
void loop()
{
    if (rf95.waitAvailableTimeout(2000))// Listen Data from LoRa Node
    {
        uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];//receive data buffer
        uint8_t len = sizeof(buf);//data buffer length
        if (rf95.recv(buf, &len))//Check if there is incoming data
        {
            recdata( buf, len);
            Console.print("Get LoRa Packet: ");
            for (int i = 0; i < len; i++)
            {
                Console.print(buf[i],HEX);
                Console.print(" ");
            }
            Console.println();
            if(crcdata == recCRCData) //Check if CRC is correct
            { 
                if(buf[0] == 1 && buf[1] == 1 && buf[2] ==1) //Check if the ID match the LoRa Node ID
                {
                    uint8_t data[] = "   Server ACK";//Reply 
                    data[0] = buf[0];
                    data[1] = buf[1];
                    data[2] = buf[2];
                    rf95.send(data, sizeof(data));// Send Reply to LoRa Node
                    rf95.waitPacketSent();
                    int newData[4] = {0, 0, 0, 0}; //Store Sensor Data here
                    for (int i = 0; i < 4; i++)
                    {
                        newData[i] = buf[i + 3];
                    }
                    int hh = newData[0];
                    int hl = newData[1];
                    int th = newData[2];
                    int tl = newData[3];
                    Console.print("Get Temperature:");
                    Console.print(th);
                    Console.print(".");
                    Console.println(tl);
                    Console.print("Get Humidity:");
                    Console.print(hh);
                    Console.print(".");
                    Console.println(hl);
                                       
                    dataString ="field1=";
                    dataString += th;
                    dataString +=".";
                    dataString += tl;
                    dataString +="&field2=";
                    dataString += hh;
                    dataString +=".";
                    dataString += hl;
                                       
                    uploadData(); // 
                    dataString="";
                }
            } 
            else 
              Console.println(" CRC Fail");     
         }
         else
         {
              //Console.println("recv failed");
              ;
          }
     }
 
}

void uploadData() {//Upload Data to ThingSpeak
  // form the string for the API header parameter:


  // form the string for the URL parameter, be careful about the required "
  String upload_url = "https://api.thingspeak.com/update?api_key=";
  upload_url += myWriteAPIString;
  upload_url += "&";
  upload_url += dataString;

  Console.println("Call Linux Command to Send Data");
  Process p;    // Create a process and call it "p", this process will execute a Linux curl command
  p.begin("curl");
  p.addParameter("-k");
  p.addParameter(upload_url);
  p.run();    // Run the process and wait for its termination

  Console.print("Feedback from Linux: ");
  // If there's output from Linux,
  // send it out the Console:
  while (p.available()>0) 
  {
    char c = p.read();
    Console.write(c);
  }
  Console.println("");
  Console.println("Call Finished");
  Console.println("####################################");
  Console.println("");
}


