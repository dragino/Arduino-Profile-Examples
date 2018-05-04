/*
  LoRaWAN packet forwarder example :
  Support Devices: LG01. 

  Require Library:
  https://github.com/sandeepmistry/arduino-LoRa 
  
  Example sketch showing how to get LoRaWAN packets from LoRaWAN node and forward it to LoRaWAN Server

  It is designed to work with the other example Arduino_LMIC

  modified 10 May 2017
  by Dragino Tech <support@dragino.com>

  Original Topic from http://qiita.com/openwave-co-jp/items/7edb3661ab5703e38e7c 
*/
#include <SPI.h>
#include <LoRa.h>
#include <Console.h>
#include <Process.h>
#include <FileIO.h>

// define servers
String SERVER = "router.au.thethings.network"; // The Things Network
String Port="1700";//ttn
// Set center frequency
uint32_t freq;
int SF,Denominator;
long SBW;
long old_time=millis();
long new_time;
long status_update_interval=30000;  //update Status every 300 seconds. 
char message[256];
void sendudp(String rssi, String packetSize, String freq) {
  Process p;
  delay(3000);
  p.begin("python");
  p.addParameter("/root/gwstat.py");
  p.addParameter(SERVER);
  p.addParameter(Port);
  p.addParameter(rssi);
  p.addParameter(packetSize);
  p.addParameter(freq);
  p.run();
  while (p.running());
  while (p.available()) {
    char c = p.read();
    Console.print(c);
  }
  Console.flush();
}

//Update Gateway Status to IoT Server
void sendstat() {
  sendudp("stat","",String((double)freq/1000000));
}
void read_fre() {
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
  freq=atol(fre1);
  Console.flush();
}
void read_SF() {
  char sf1[3];
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
  char cr1[2];
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
  Console.print("The frequency is ");Console.print(freq);Console.println("Hz");
  Console.print("The spreading factor is ");Console.println(SF);
  Console.print("The coderate is ");Console.println(Denominator);
  Console.print("The single bandwith is ");Console.print(SBW);Console.println("Hz");
}

//Receiver LoRa packets and forward it 
void receivepacket() {
  // try to parse packet
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    // received a packet
    Console.print("Received packet '");

    // read packet
    int i = 0;
    //char message[256];
    while (LoRa.available() && i < 256) {
      message[i]=LoRa.read();
      Console.print(message[i],HEX);
      i++;
    }
        
    FileSystem.begin();
    File dataFile = FileSystem.open("/root/data/bin", FILE_WRITE);
    for(int j=0;j<i;j++)
        dataFile.print(message[j]);
    dataFile.close();
        
    delay(1000);
    //send the messages
    sendudp(String(LoRa.packetRssi()), String(packetSize),String((double)freq/1000000));
  }
}

void setup() {
  Bridge.begin(115200);
  Console.begin();
  while (!Console);
  Console.println("LoRa Receiver");
  read_config();
  if (!LoRa.begin(freq)) {
    Console.println("Starting LoRa failed!");
    while (1);
  }
  LoRa.setSpreadingFactor(SF);
  LoRa.setSignalBandwidth(SBW);
  LoRa.setCodingRate4(Denominator);
  LoRa.setSyncWord(0x34); 
  LoRa.receive(0);
  show_config();       
  sendstat();
}

void loop() {
  receivepacket();
  new_time = millis();

  // Update Gateway Status
  if( (new_time-old_time) > status_update_interval){
      Console.println("");
      Console.println("Update Status");
      old_time = new_time;
      sendstat();
      mkfile();
  }
}
void mkfile()
{
  FileSystem.begin();
  File file_name = FileSystem.open("/tmp/iot/data3",FILE_WRITE);
  file_name.print((char *)message);
  file_name.close();
}

