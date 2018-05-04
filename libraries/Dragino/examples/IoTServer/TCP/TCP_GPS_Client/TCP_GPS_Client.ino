#include <SoftwareSerial.h>
#include <TinyGPS.h>
#include <SPI.h>
#include <RH_RF95.h>
TinyGPS gps;
RH_RF95 rf95;
SoftwareSerial ss(3, 4); // Arduino TX, RX to conenct to GPS module.
float frequency = 868.3; 
unsigned int count = 1;
static void smartdelay(unsigned long ms);

String datastring1="";
String datastring2="";
String datastring3="";
uint8_t datasend[50];    //Storage  longtitude,latitude and altitude

char gps_lon[50]={"\0"};  //Storage GPS info
char gps_lat[20]={"\0"}; //Storage latitude
char gps_alt[20]={"\0"}; //Storage altitude
void setup()
{
  // initialize both serial ports:
  Serial.begin(9600);  // Serial to print out GPS info in Arduino IDE
  ss.begin(9600);       // SoftSerial port to get GPS data. 
  while (!Serial);

   if (!rf95.init()) {  // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  
  /* Set frequency is 868MHz,SF is 7,SB is 125KHz,CR is 4/5 and Tx power is 20dBm */
  rf95.setFrequency(frequency);
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
  strcpy((char *)datasend,gps_lon);//the format of datasend is longtitude,latitude,altitude,DeviceID,
  
  // send data
  rf95.send(datasend, sizeof(datasend));  
  rf95.waitPacketSent();
  
  // Now wait for a reply
  receivepacket();
  }
  smartdelay(1000);
     Serial.print(F("###########    "));
     Serial.print(F("COUNT="));
     Serial.print(count);
     Serial.println(F("    ###########"));
     Serial.println(F("The longtitude and latitude and altitude are:"));
     Serial.print(F("["));
     Serial.print((char *)datasend);
     Serial.print(F("]"));
     Serial.print("");
     Serial.println("");
     count++;
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
         //Serial.println((char*)indatabuf);        
         }
         else 
         {
          Serial.println("receive failed!");
         }
    }
    else
    {
      // Serial print "No reply, is track_GPS_server running or overranging the receive distance?" if don't get the reply .
      Serial.println("No reply, is track_GPS_Server running or overranging the receive distance?");
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
