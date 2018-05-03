#include <SPI.h>
#include <RH_RF95.h>
#include <Console.h>
#include <Process.h>
#include <FileIO.h>
RH_RF95 rf95;
#define BAUDRATE 115200

const unsigned long postingInterval = 10000;  //delay
unsigned long lastRequest = 0;      // when you last made a request
int SF,Denominator;
long SBW;
uint32_t freq;
char cr1[2];
char sbw1[2];
char sf1[3];
char fre1[9];
uint8_t data[72] = {'\0'};

void setup(){
     Bridge.begin(BAUDRATE);
      while (!Console) ; // Wait for console port to be available
         read_config();
         
      if (!rf95.init())
        Console.println("init failed");
         rf95.setFrequency((double)freq/1000000);
         rf95.setTxPower(13);
         rf95.setSpreadingFactor(SF);
         rf95.setSignalBandwidth(SBW);
         rf95.setCodingRate4(Denominator);
        // show_config();
}

void loop(){
     ReceiveData();
     mkfile();
}

void ReceiveData(){
     long now = millis();
     if (now - lastRequest >= postingInterval)
     {
     if (rf95.available())
     {
            uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
            uint8_t len = sizeof(buf);

       if (rf95.recv(buf, &len))
       {
            RH_RF95::printBuffer("request: ", buf, len);
            Console.print(F("got request from LoRa Node: "));
            Console.println((char*)buf);
            //Console.print(F("RSSI: "));
            //Console.println(rf95.lastRssi(), DEC);
            //Console.println(sizeof(buf));
       }
            
             for (int k = 0;k < len-13;k++)
             {
                data[k] = buf[k]; 
            //Console.print(buf[k],HEX);
            //Console.print("");
             }
             rf95.send(data, sizeof(data));
             rf95.waitPacketSent();
             Console.println(F("Sent a reply to Node and sent data to TCP Server."));
             
            
       
     }
     else
     {
            // Console.println("recv failed");
     }
  }
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

void mkfile()
{
  FileSystem.begin();
  File file_name = FileSystem.open("/tmp/iot/data3",FILE_WRITE);
  file_name.print((char *)data);
  file_name.close();
}
