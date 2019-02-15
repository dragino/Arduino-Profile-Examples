#include <TinyGPS.h>
#include <SPI.h>
#include <LoRa.h>
#include <SoftwareSerial.h>

TinyGPS gps;
SoftwareSerial ss(4, 3); // Arduino RX, TX to conenct

static void smartdelay(unsigned long ms);
unsigned int count = 0;        //For times count

float longitude,latitude;
float flat, flon;
float mgLon,mgLat;  // World Geodetic System ==> Mars Geodetic System
char mgLon_1[10]={"\0"},mgLat_1[10]={"\0"}; 
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
  if((flon < 72.004 || flon > 137.8347)&&(flat < 0.8293 || flat >55.8271))  //out of China
  {
    longitude=flon;
    latitude=flat;
  // Serial.println("Out of China");
  }
  else
  {
    WGS2GCJTransform(flon,flat,mgLon,mgLat);
    longitude=mgLon;
    latitude=mgLat;
   //Serial.println("In China");
  }
     if(flon!=1000.000000)    //Successful positioning 
  {  
       Serial.print(F("###########    "));
       Serial.print(F("NO."));
       Serial.print(count);
       Serial.println(F("    ###########"));
       Serial.println(F("The longtitude and latitude:"));
       Serial.print(F("["));
       Serial.print(longitude,4);
       Serial.print(F(","));
       Serial.print(latitude,4);
       Serial.print(F("]"));
     Serial.println(F(""));
       count++;
 } 
    else
   {
   Serial.println(F("Unsuccessfully positioning"));
   }
}
void GPSWrite()
{
    char data[80] = "\0";
    
    for(int i = 0; i < 50; i++)
    {
       data[i] = node_id[i];
    }

    dtostrf(mgLat,0,4,mgLat_1);
    dtostrf(mgLon,0,4,mgLon_1);

     strcat(data,"{");
     strcat(data,"\"location\":");
     strcat(data,"{");
     strcat(data,"\"value\":1");
     strcat(data,",");
     strcat(data,"\"context\":");
     strcat(data,"{");
     strcat(data,"\"lat\":");
     strcat(data,mgLat_1);
     strcat(data,",");
     strcat(data,"\"lng\":");
     strcat(data,mgLon_1);
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

// World Geodetic System ==> Mars Geodetic System
double transformLat(double x, double y)  
{  
    double ret = -100.0 + 2.0 * x + 3.0 * y + 0.2 * y * y + 0.1 * x * y + 0.2 * sqrt(abs(x));  
    ret += (20.0 * sin(6.0 * x * M_PI) + 20.0 * sin(2.0 * x * M_PI)) * 2.0 / 3.0;  
    ret += (20.0 * sin(y * M_PI) + 40.0 * sin(y / 3.0 * M_PI)) * 2.0 / 3.0;  
    ret += (160.0 * sin(y / 12.0 * M_PI) + 320 * sin(y * M_PI / 30.0)) * 2.0 / 3.0;  
    return ret;  
}  
  
 double transformLon(double x, double y)  
{  
    double ret = 300.0 + x + 2.0 * y + 0.1 * x * x + 0.1 * x * y + 0.1 * sqrt(abs(x));  
    ret += (20.0 * sin(6.0 * x * M_PI) + 20.0 * sin(2.0 * x * M_PI)) * 2.0 / 3.0;  
    ret += (20.0 * sin(x * M_PI) + 40.0 * sin(x / 3.0 * M_PI)) * 2.0 / 3.0;  
    ret += (150.0 * sin(x / 12.0 * M_PI) + 300.0 * sin(x / 30.0 * M_PI)) * 2.0 / 3.0;  
    return ret;  
}  
    
void WGS2GCJTransform(float wgLon, float wgLat, float &mgLon, float &mgLat)  
{  
    const double a = 6378245.0;  
    const double ee = 0.00669342162296594323;  
  
    double dLat = transformLat(wgLon - 105.0, wgLat - 35.0);  
    double dLon = transformLon(wgLon - 105.0, wgLat - 35.0);  
  
    double radLat = wgLat / 180.0 * M_PI;  
    double magic = sin(radLat);  
    magic = 1 - ee * magic * magic;  
  
    double sqrtMagic = sqrt(magic);  
    dLat = (dLat * 180.0) / ((a * (1 - ee)) / (magic * sqrtMagic) * M_PI);  
    dLon = (dLon * 180.0) / (a / sqrtMagic * cos(radLat) * M_PI);  
  
    mgLat = wgLat + dLat;  
    mgLon = wgLon + dLon;  
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

