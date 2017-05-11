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
String Port = "1700";//ttn

// Set center frequency
uint32_t freq = 868100000;

long old_time=millis();
long new_time;
long status_update_interval=300000;  //update Status every 300 seconds. 

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

//Receiver LoRa packets and forward it 
void receivepacket() {
  // try to parse packet
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    // received a packet
    Console.print("Received packet '");

    // read packet
    int i = 0;
    char message[256];
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

  if (!LoRa.begin(freq)) {
    Console.println("Starting LoRa failed!");
    while (1);
  }
  
  LoRa.setSyncWord(0x34); 
  LoRa.receive(0);       
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
  }
}
