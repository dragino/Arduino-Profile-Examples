#include <TinyGPS.h>
#include <SPI.h>
#include <LoRa.h>
#include <SoftwareSerial.h>

TinyGPS gps;
SoftwareSerial ss(4, 3); // Arduino RX, TX to conenct

static void smartdelay(unsigned long ms);
unsigned int count = 0;        //For times count

float flat, flon;
char flat_1[10]={"\0"},flon_1[10]={"\0"}; 
char *node_id = "<5678>";  //From LG01 via web Local Channel settings on MQTT.Please refer <> dataformat in here. 
uint8_t datasend[80];

void setup()
{
      Serial.begin(9600);
      while (!Serial);
      ss.begin(9600);
      Serial.println(F("Start MQTT Example of Ubidots"));
      if (!LoRa.begin(868000000))   //868000000 is frequency
      { 
          Serial.println("Starting LoRa failed!");
          while (1);
      }
      LoRa.setSyncWord(0x34); 
      Serial.println("LoRa init succeeded.");    
        
}

void GPSRead()
{
  unsigned long age;
  gps.f_get_position(&flat, &flon, &age);
  flon == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flon, 6;//save six decimal places 
  flat == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flat, 6;  
       if(flon!=1000.000000)
  {  
       Serial.print(F("###########    "));
       Serial.print(F("NO."));
       Serial.print(count);
       Serial.println(F("    ###########"));
       Serial.println(F("The longtitude and latitude and altitude are:"));
       Serial.print(F("["));
       Serial.print(flat,3);
       Serial.print(F(","));
       Serial.print(flon,3);
       Serial.print(F("]"));
     Serial.println(F(""));
       count++;
 } 
}
void GPSWrite()
{
    char data[80] = "\0";
    
    for(int i = 0; i < 50; i++)
    {
       data[i] = node_id[i];
    }
    
    dtostrf(flon,0,3,flon_1);
    dtostrf(flat,0,3,flat_1);

     strcat(data,"{");
     strcat(data,"\"location\":");
     strcat(data,"{");
     strcat(data,"\"value\":1");
     strcat(data,",");
     strcat(data,"\"context\":");
     strcat(data,"{");
     strcat(data,"\"lat\":");
     strcat(data,flat_1);
     strcat(data,",");
     strcat(data,"\"lng\":");
     strcat(data,flon_1);
     strcat(data,"}");
     strcat(data,"}");
     strcat(data,"}");
     strcpy((char *)datasend,data);
    
 // Serial.println((char *)datasend);
    delay(1000);    
}

static void smartdelay(unsigned long ms)
{
  unsigned long start = millis();
  do 
  {
    while (ss.available())
    {
      gps.encode(ss.read());
    }
  } while (millis() - start < ms);
}

void SendData()
{
     LoRa.beginPacket();
     LoRa.print((char *)datasend);
     LoRa.endPacket();
     Serial.println("The packet is send successful");
     delay(60000);  // one minute
}    
   
void loop()
{
     smartdelay(1000);
     GPSRead();
     GPSWrite();
     SendData();
}

