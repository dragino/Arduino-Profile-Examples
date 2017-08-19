/*
  LoRaWAN packet forwarder example :
  Support Devices: LG01 Single Channel LoRa Gateway
  Require Library:
  RadioHead Library: https://github.com/dragino/RadioHead 

  LG01 test with firmware version v4.2.3
  
  Example sketch showing how to get LoRaWAN packets from LoRaWAN node and forward it to LoRaWAN Server
  It is designed to work with 
  modified 27 Jul 2017
  by Dragino Tech <support@dragino.com>
*/

#include <FileIO.h>
#include <Console.h> 
#include <Process.h>
#include <SPI.h>
#include <LoRa.h>

String Sketch_Ver = "single_pkt_fwd_v002";
float freq;
int SF,CR;
long BW,preLen;
long old_time=millis();
long new_time;
long status_update_interval=300000;  //update Status every 300 seconds. 

void sendGatewayStat();// Update Gateway Status to LoRaWAN Server. 
void getRadioConf();//Get LoRa Radio Configure from LG01
void setLoRaRadio();//Set LoRa Radio
void receivepacket();// receive packet

//Set Debug = 1 to enable Console Output;
int debug=0; 

void setup() {
 // delay(5000);
  // Setup Bridge 
  Bridge.begin(115200);
    
  // Setup File IO
  FileSystem.begin();
  
  if ( debug > 0 )
  {
    Console.begin();
    //Print Current Version 
    Console.print("Sketch Version:");
    Console.println(Sketch_Ver);
  }


  //write sketch version to Linux
  writeVersion();
  //Get 
  getRadioConf();
  if ( debug > 0 )
  {
      Console.println("Start LoRaWAN Single Channel Gateway");
      Console.print("RX Frequency: ");
      Console.println(freq); 
      Console.print("Spread Factor: SF");
      Console.println(SF);   
      Console.print("Coding Rate: 4/");
      Console.println(CR);  
      Console.print("Bandwidth: ");
      Console.println(BW);  
      Console.print("PreambleLength: ");
      Console.println(preLen);   
  } 

  if (!LoRa.begin(freq))
    if ( debug > 0 ) Console.println("init LoRa failed");
  
  setLoRaRadio();// Set LoRa Radio to Semtech Chip

  delay(3000);
  sendGatewayStat(); // Upload GW status to LoRaWAN server
}

void loop(){
  receivepacket();
  
  // Update Gateway Status
  new_time = millis();
  if( (new_time-old_time) > status_update_interval)
  {
      old_time = new_time;
      sendGatewayStat();
  }
}


//Update Gateway Status to IoT Server
void sendGatewayStat() {
  Process p;    // Create a Linux Process
  p.begin("/usr/bin/lora_udp_fwd");  // Process that launch the "lora_udp_fwd" command
  p.addParameter("stat"); // Send stat
  p.run();    // Run the process and wait for its termination

  if ( debug > 0 )  Console.println("Update Gateway Status");
}


//Get LoRa Radio Configure from LG01
void getRadioConf() {
  //Read frequency from uci
  char fre1[9];
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
  freq=atof(fre1);

  //Read Spread Factor
  char sf1[3];
  j=0;
  p.begin("uci");
  p.addParameter("get"); 
  p.addParameter("lorawan.radio.SF");
  p.run();    // Run the process and wait for its termination
  while (p.available()>0 && j<3) {   
   sf1[j]= p.read();
   j++;
  }
  SF=atoi(sf1);

  //Read Coding Rate
  char cr1[2];
  j=0;
  p.begin("uci");
  p.addParameter("get"); 
  p.addParameter("lorawan.radio.coderate");
  p.run();    // Run the process and wait for its termination
  while (p.available()>0 && j<2) {   
   cr1[j]= p.read();
   j++;
  }
  CR = atoi(cr1);

  //Read PreambleLength
  char pr[2];
  j=0;
  p.begin("uci");
  p.addParameter("get"); 
  p.addParameter("lorawan.radio.preamble");
  p.run();    // Run the process and wait for its termination
  while (p.available()>0 && j<5) {   
   pr[j]= p.read();
   j++;
  }
  preLen = atol(pr);

  //Read BandWidth
  char sbw1[2];
  int b1;
  j=0;
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
    case 0:BW=7.8E3;break;
    case 1:BW=10.4E3;break;
    case 2:BW=15.6E3;break;
    case 3:BW=20.8E3;break;
    case 4:BW=31.25E3;break;
    case 5:BW=41.7E3;break;
    case 6:BW=62.5E3;break;
    case 7:BW=125E3;break;
    case 8:BW=250E3;break;
    case 9:BW=500E3;break;
    default:break;
  }
}

void setLoRaRadio(){
  LoRa.setFrequency(freq);
  LoRa.setSpreadingFactor(SF);
  LoRa.setSignalBandwidth(BW);
  LoRa.setCodingRate4(CR);
  LoRa.setSyncWord(0x34);
  LoRa.setPreambleLength(preLen);
  LoRa.receive(0);   
}

//Receiver LoRa packets and forward it 
void receivepacket() {
  // try to parse packet
  int packetSize = LoRa.parsePacket(); 
  if (packetSize)
  {
      // Received a packet
      if ( debug > 0 )
      { 
          Console.print("Get Packet:");
          Console.print(packetSize);
          Console.println(" Bytes");
      }
      // read packet
      int i = 0;
      char message[256];
      while (LoRa.available() && i < 256) {
        message[i]=LoRa.read();
        if ( debug > 0 )
        {
          Console.print("[");
          Console.print(i);
          Console.print("]");
          Console.print(message[i],HEX);
          Console.print(" ");
        }
        i++;
      }
      if ( debug > 0 ) Console.println("");

      FileSystem.begin();
      File dataFile = FileSystem.open("/var/iot/data", FILE_WRITE);            
      for (int j=0;j<i;j++) 
          dataFile.print(message[j]);
      dataFile.close();
          
      delay(1000);
      
      //send the messages
      
      Process p;    // Create a Linux Process
      p.begin("/usr/bin/lora_udp_fwd");  // Process that launch the "lora_udp_fwd" command
      p.addParameter("/var/iot/data");
      p.addParameter(String(LoRa.packetRssi())); // Send stat
      p.addParameter(String(packetSize));
      p.run();    // Run the process and wait for its termination
      while (p.available() > 0) {
        char c = p.read();
        if ( debug > 0 ) Console.print(c);
      }
      // Ensure the last bit of data is sent.
      if ( debug > 0 ) Console.flush(); 
  }
}

//Function to write sketch version number into Linux. 
void writeVersion()
{
  File fw_version = FileSystem.open("/var/avr/fw_version", FILE_WRITE);
  fw_version.print(Sketch_Ver);
  fw_version.close();
}

