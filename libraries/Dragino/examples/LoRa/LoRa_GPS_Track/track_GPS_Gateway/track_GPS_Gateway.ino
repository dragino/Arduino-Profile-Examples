/*
In this project,we'll show how to get GPS data from a remote Arduino via Wireless LoRa Protocol 
and show the track on the GPSWOX.com.

Hardware Set up
1 Client Side: Arduino + LoRa/GPS Shield (868Mhz).
    In the client side, we will use the hardware serial to print out debug data in PC and use software
    serial to get GPS data. 
    1/ Arduino UNO + LoRa GPS Shield . if use other Arduino board, please check the softserial connection and modify the code
    2/ Wire GPS_RXD to Arduino D3
    3/ Wire GPS_TXD to Arduino D4
    4/ Remove jumper in GPS_RXD/GPX_TXD 1x3 pin header
    a photo for hardware configuration is here: http://wiki.dragino.com/index.php?title=File:Uno_gps_softwareserial.png  
  
2 Gateway Side: LG01(868Mhz)

External Library Require:
  TinyGPS:  http://arduiniana.org/libraries/tinygps/
  RadioHead Library: https://github.com/dragino/RadioHead 

Client side will get GPS data and keep sending out to the gateway via LoRa wireless. Gateway side 
will listen on the LoRa wireless frequency, when it get the GPS data, the gateway will send it to 
the GPSWOX tracking server
 
Note: Press the "RST" button when you upload the sketch.

modified 09 6 2016
by Dragino Tech<support@dragino.com>
Dragino Technology Co., Limited
*/

#include <Console.h> 
#include <Process.h>
#include <SPI.h>
#include <RH_RF95.h>

RH_RF95 rf95;

char DeviceID[20]="\0"; //Storage DeviceID or Device identifier
char lon[20]="\0";           //Storage longtitude
char lat[20]="\0";           //Storage latitude
char alt[20]="\0";           //Storage altitude

void getTimeStamp();     //LG01 will call the Linux "date" command and get the time stamp
void receivepacket();    //Processing receive message and store it in the appropriate array
void run_send_gps_data();//LG01 will call the Linux "send_gps_data.sh" command and write the GPS data in GPSWOX.com

void setup() { 
  Bridge.begin(115200);  
  Console.begin();
  while (!Console);
  
   if (!rf95.init()) {// Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on
    Console.println("Starting LoRa failed!");
    while (1);
  }
  /* Set frequency is 868MHz,SF is 7,SB is 125KHz,CR is 4/5 and Tx power is 20dBm */
  rf95.setFrequency(868);
  rf95.setSpreadingFactor(7);
  rf95.setSignalBandwidth(125E3);
  rf95.setCodingRate4(5);
  rf95.setTxPower(20,false);
  
  Console.println("Ready to receive!");
}
void loop(){
 receivepacket();
}

void getTimeStamp() {
  Process time;
  // date is a command line utility to get the date and the time 
  // in different formats depending on the additional parameter 
  time.begin("date");
  time.addParameter("+%D-%T");  // parameters: D for the complete date mm/dd/yy
                                //             T for the time hh:mm:ss    
  time.run();  // run the command

  // read the output of the command
  while(time.available()>0) {
    char c = time.read();
    Console.print(c);
  }
}

void run_send_gps_data() {//send_gps_data.sh -d DeviceID -l latitude -n longtitude -a altitude
  Process p;    // Create a process and call it "p"
  p.begin("send_gps_data.sh");
  p.addParameter("-d");
  p.addParameter(DeviceID);
  p.addParameter("-l"); 
  p.addParameter(lat);
  p.addParameter("-n"); 
  p.addParameter(lon);
  p.addParameter("-a"); 
  p.addParameter(alt);
  p.run();    // Run the process and wait for its termination
}

//Receiver LoRa packets and forward it 
void receivepacket() {
   if (rf95.available())
  {
    // received a packet
    Console.print("Get new data: ");

    int i = 0,j=0,code[4];
    int m1=0,m2=0,m3=0,m4=0;   
    uint8_t buf[50];
    char message[50]="\0";
    uint8_t len = sizeof(buf);
    if (rf95.recv(buf, &len)){
      strcpy(message,(char *)buf);
      while(i<50)
      {
      if(message[i]==',')
      {
        code[j]=i;
        j++;
        }
        i++;
      }
    }
    for(int k=0;k<code[0];k++)
    {
      lon[m1]=message[k];//get longtitude
      m1++;
    }
     for(int k=code[0]+1;k<code[1];k++)
    {
      lat[m2]=message[k];//get latitude
      m2++;
    }
     for(int k=code[1]+1;k<code[2];k++)
    {
      alt[m3]=message[k];//get altitude
      m3++;
    }
    for(int k=code[2]+1;k<code[3];k++)
    {
      DeviceID[m4]=message[k];//get  DeviceID
      m4++;
    }
    run_send_gps_data();
    Console.print((char*)buf);
    Console.print("  with RSSI: ");
    Console.print(rf95.lastRssi(), DEC);
    Console.print("  ");getTimeStamp();
    //Console.print("the longtitude is " );Console.println(lon);
    //Console.print("the latitude is ");Console.println(lat);
    //Console.print("the altitude is ");Console.println(alt);
    //Console.print("the DeviceID is ");Console.println(DeviceID);
    
    uint8_t data[] = "Gateway receive GPS data";
    rf95.send(data, sizeof(data));
    rf95.waitPacketSent();
  }
}
