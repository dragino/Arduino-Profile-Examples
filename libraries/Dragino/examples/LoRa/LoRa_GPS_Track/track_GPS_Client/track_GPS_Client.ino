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

#include <SoftwareSerial.h>
#include <TinyGPS.h>
#include <SPI.h>
#include <RH_RF95.h>
TinyGPS gps;
RH_RF95 rf95;
SoftwareSerial ss(3, 4); // Arduino RX, TX to conenct to GPS module.

static void smartdelay(unsigned long ms);

int LED=A0;

char DeviceID[10]="a840418"; //DeviceID or Device identifier in GPSWOX.com

String datastring1="";
String datastring2="";
String datastring3="";
uint8_t datasend[50];    //Storage  longtitude,latitude and altitude

char gps_lon[50]={"\0"};  //Storage GPS info
char gps_lat[20]={"\0"}; //Storage latitude
char gps_alt[20]={"\0"}; //Storage altitude
void setup()
{
  // initialize digital pin  as an output.
  pinMode(LED, OUTPUT);
  
  // initialize both serial ports:
  Serial.begin(9600);  // Serial to print out GPS info in Arduino IDE
  ss.begin(9600);       // SoftSerial port to get GPS data. 
  while (!Serial);

   if (!rf95.init()) {  // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  
  /* Set frequency is 868MHz,SF is 7,SB is 125KHz,CR is 4/5 and Tx power is 20dBm */
  rf95.setFrequency(868);
  rf95.setSpreadingFactor(7);
  rf95.setSignalBandwidth(125E3);
  rf95.setCodingRate4(5);
  rf95.setTxPower(20,false);
  
  Serial.println("Ready to send!");
}

void loop()
{
  float flat, flon,falt;
  unsigned long age;
  gps.f_get_position(&flat, &flon, &age);
  falt=gps.f_altitude();  //get altitude       
  flon == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flon, 6;//save six decimal places 
  flat == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flat, 6;
  falt == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : falt, 2;//save two decimal places
  datastring1 +=dtostrf(flat, 0, 6, gps_lat); 
  datastring2 +=dtostrf(flon, 0, 6, gps_lon);
  datastring3 +=dtostrf(falt, 0, 2, gps_alt);
  if(flon!=1000.000000)
  {
  strcat(gps_lon,",");
  strcat(gps_lon,gps_lat); 
  strcat(gps_lon,","); 
  strcat(gps_lon,gps_alt);
  strcat(gps_lon,","); 
  strcat(gps_lon,DeviceID);
  strcat(gps_lon,",");
  strcpy((char *)datasend,gps_lon);//the format of datasend is longtitude,latitude,altitude,DeviceID,
  Serial.println((char *)datasend);
  
  // send data
  rf95.send(datasend, sizeof(datasend));  
  rf95.waitPacketSent();
  
  // Now wait for a reply
  receivepacket();
  }
  smartdelay(1000);
}

//If the packet arrive LG01, LG01 will send a ACK and here will receive it and turn on the led.  
void receivepacket(){
    uint8_t indatabuf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(indatabuf);
    
    if (rf95.waitAvailableTimeout(3000))
     { 
       // Should be a reply message for us now   
       if (rf95.recv(indatabuf, &len))
         {
         Serial.println((char*)indatabuf);        
         digitalWrite(LED, HIGH);   // turn the LED on (HIGH is the voltage level)
         }
         else 
         {
          digitalWrite(LED, LOW);    // turn the LED off by making the voltage LOW
          Serial.println("receive failed!");
         }
    }
    else
    {
      // Serial print "No reply, is track_GPS_server running or overranging the receive distance?" if don't get the reply .
      //Serial.println("No reply, is track_GPS_Server running or overranging the receive distance?");
      digitalWrite(LED, LOW);    // turn the HEART_LED off by making the voltage LOW
    }
}

static void smartdelay(unsigned long ms)
{
  unsigned long start = millis();
  do 
  {
    while (ss.available())
    {
      //ss.print(Serial.read());
      gps.encode(ss.read());
    }
  } while (millis() - start < ms);
}
