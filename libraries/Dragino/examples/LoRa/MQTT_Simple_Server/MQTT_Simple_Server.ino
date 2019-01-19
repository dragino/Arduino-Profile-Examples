 /*
  Private LoRa protocol example :
  Support Devices: LG01 Single Channel LoRa Gateway

  This sketch is running in LG01. LG01 with firmware version v4.3.3 or above

  Example sketch shows how to get sensor data from remote LoRa Sensor and Store the data into LG01. 
  The LG01 has daemon script to check the data periodically and update the data to IoT server. 

  The LoRa Protocol use here is private LoRa protocol which base on RadioHead Library. 

  Created 26 March 2018
  by Dragino Tech <support@dragino.com>
*/
#define BAUDRATE 115200
#include <Console.h>
#include <SPI.h>
#include <RH_RF95.h>
#include <Process.h>
#include <FileIO.h>


RH_RF95 rf95;  // Singleton instance of the radio driver
const String Sketch_Ver = "collect_data_v001";
static unsigned long newtime; 
static uint8_t packet[64];
int data_pos;
boolean data_format=false;
const long sendpkt_interval = 10000;  // 10 seconds for replay.
int debug = 0;
int SF,Denominator;
long SBW;
uint32_t freq;
char cr1[2];
char sbw1[2];
char sf1[3];
char fre1[9];

void setup() 
{
      
  Bridge.begin(BAUDRATE);
  //while (!Console) ; // Wait for console port to be available
        read_config();
  
   if (!rf95.init())
        Console.println("init failed");
        rf95.setFrequency((double)freq/1000000);
        rf95.setTxPower(13);
        rf95.setSpreadingFactor(SF);
        rf95.setSignalBandwidth(SBW);
        rf95.setCodingRate4(Denominator);
        rf95.setSyncWord(0x34);
       // show_config();    //LG01 configure be shown
        writeVersion();
        Console.print(F("Sketch Version:"));
        Console.println(Sketch_Ver);
}

void loop()
{
     ReceiveData();
     feeddog();
}

void ReceiveData()
{
  if (rf95.available())
  {
    // Should be a message for us now   
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);
   
    if (rf95.recv(buf, &len))
    {
         RH_RF95::printBuffer("request: ", buf, len);
         if(debug > 0){
           Console.print(F("got request from LoRa Node: "));
           Console.print((char*)buf);
           Console.print(F("RSSI: "));
           Console.println(rf95.lastRssi(), DEC);
         }
         for (int i = 0; i < len-5; i++)
         {
          if(debug > 0){
           Console.print(buf[i],HEX);
           Console.print(" ");
          }
         }
      // Send a reply
      uint8_t data[64] = {'\0'};
      char id[8] =  {'\0'};
      int i;
        if (buf[0] == 0x3C)   // Only process if data is start with <
        {
          for (i = 0; i < len; i++)
          {
              id[i]= buf[i];
              if (buf[i] == 0x3E)
              {
                data_pos = i+1;
                if (data_pos < len) 
                data_format=true;
                break; //Get ASCII >, so break;
                data_format==false;
                break; // Data Format mismatch, quit process. 
              } 
          }
         int h = 0;
         char id1[8] =  {'\0'};
          for (int j = 1; j< i;j++)
          {
            id1[h++] = id[j];            
          }
           for (int k=data_pos;k<len;k++)
          {
            data[k-(i+1)]=buf[k];
          }

      //Console.println(id1);
     // Console.println((char*)data);          
      rf95.send(data, sizeof(data));
      rf95.waitPacketSent();
        Console.println(F("Sent a reply to Node and update data to IoT Server."));
           
     Process p;
     p.begin("store_data");
     p.addParameter(id1);
     p.addParameter(data);  
     p.run();    
    }
    else
    {
      Console.println("recv failed");
    }
  }
}
}

void feeddog()
{
    int i = 0;

    memset(packet, 0, sizeof(packet));

    Process p;    // Create a process
    p.begin("date");
    p.addParameter("+%s");
    p.run();    
    while (p.available() > 0 && i < 32) {
        packet[i] = p.read();
        i++;
    }
    newtime = atol(packet);

    File dog = FileSystem.open("/var/iot/dog", FILE_WRITE);
    dog.println(newtime);
    dog.close();
}

void read_fre() {
  int j=0;
  Process p;    // Create a process and call it "p"
  p.begin("uci");
  p.addParameter("get"); 
  p.addParameter("lorawan.radio.rx_frequency");
  p.run();    // Run the process and wait for its termination
  while (p.available()>0 && j<9) {   
   fre1[j]= p.read();
   j++;
  }
  freq=atol(fre1);
  Console.flush();
}

void read_SF() {
  int j=0;
  Process p;    // Create a process and call it "p"
  p.begin("uci");
  p.addParameter("get"); 
  p.addParameter("lorawan.radio.SF");
  p.run();    // Run the process and wait for its termination
  while (p.available()>0 && j<3) {   
   sf1[j]= p.read();
   j++;
  }
  SF=atoi(sf1);
  Console.flush();
}

void read_CR() {
  int c1,j=0;
  Process p;    // Create a process and call it "p"
  p.begin("uci");
  p.addParameter("get"); 
  p.addParameter("lorawan.radio.coderate");
  p.run();    // Run the process and wait for its termination
  while (p.available()>0 && j<2) {   
   cr1[j]= p.read();
   j++;
  }
  Denominator=atoi(cr1);
  Console.flush();
}

void read_SBW() {
  char sbw1[2];
  int b1,j=0;
  Process p;    // Create a process and call it "p"
  p.begin("uci");
  p.addParameter("get"); 
  p.addParameter("lorawan.radio.BW");
  p.run();    // Run the process and wait for its termination
  while (p.available()>0 && j<2) {   
   sbw1[j]= p.read();
   j++;
  }
  b1=atoi(sbw1);
  switch(b1)
{
  case 0:SBW=7.8E3;break;
  case 1:SBW=10.4E3;break;
  case 2:SBW=15.6E3;break;
  case 3:SBW=20.8E3;break;
  case 4:SBW=31.25E3;break;
  case 5:SBW=41.7E3;break;
  case 6:SBW=62.5E3;break;
  case 7:SBW=125E3;break;
  case 8:SBW=250E3;break;
  case 9:SBW=500E3;break;
  default:break;
}
  Console.flush();
}

void read_config()
{
  read_fre();
  read_SF();
  read_CR();
  read_SBW();
}

void show_config()
{
  Console.print(F("The frequency is "));Console.print(freq);Console.println("Hz");
  Console.print(F("The spreading factor is "));Console.println(SF);
  Console.print(F("The coderate is "));Console.println(Denominator);
  Console.print(F("The single bandwith is "));Console.print(SBW);Console.println("Hz");
}

void writeVersion()
{
  FileSystem.begin();
  File fw_version = FileSystem.open("/var/avr/fw_version", FILE_WRITE);
  fw_version.print(Sketch_Ver);
  fw_version.close();
}

