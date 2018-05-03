#include <dht.h>
#include <SPI.h>
#include <RH_RF95.h>

// Singleton instance of the radio driver
RH_RF95 rf95;
float frequency = 868.3;  //frequency settings
dht DHT;
#define DHT11_PIN A1
float temperature,humidity,tem,hum;
String datastring1="";
String datastring2="";
char tem_1[8]={"\0"},hum_1[8]={"\0"};
uint8_t datasend[72];
unsigned int count = 1;

void setup()
{
     Serial.begin(9600);
     Serial.println(F("Start Lewei Example"));
    
          if (!rf95.init())
     Serial.println(F("init failed"));
        
         rf95.setFrequency(frequency);
         rf95.setTxPower(13);
}

void dhtTem()
{
        temperature = DHT.read11(DHT11_PIN);    //Read Tmperature data
        tem = DHT.temperature*1.0;      
        humidity = DHT.read11(DHT11_PIN);      //Read humidity data
        hum = DHT.humidity* 1.0;             
       
        Serial.println(F("The temperature and humidity:"));
        Serial.print("[");
        Serial.print(tem);
        Serial.print("℃");
        Serial.print(",");
        Serial.print(hum);
        Serial.print("%");
        Serial.print("]");
        Serial.println("");
        
        delay(2000);
}

void dhtWrite()
{
      char data[50] = "\0";

      dtostrf(tem,0,1,tem_1);
      dtostrf(hum,0,1,hum_1);

      strcat(data,"The temperature and humidity:");
      strcat(data,"[");
      strcat(data,tem_1);
      strcat(data,"℃");
      strcat(data,",");
      strcat(data,hum_1);
      strcat(data,"%");
      strcat(data,"]");
      strcpy((char *)datasend,data);
}

void SendData()
{
     Serial.println(F("Sending data to LG01"));
           
   
      rf95.send((char *)datasend,sizeof(datasend));  
      rf95.waitPacketSent();  // Now wait for a reply
    
      uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
      uint8_t len = sizeof(buf);

      //Serial.println(sizeof(buf));
     if (rf95.waitAvailableTimeout(3000))
   { 
    // Should be a reply message for us now   
    if (rf95.recv(buf, &len))
   {
     
      Serial.print("got reply from LG01: ");
      Serial.println((char*)buf);
      Serial.print("RSSI: ");
      Serial.println(rf95.lastRssi(), DEC);    
    }
    else
    {
      Serial.println("recv failed");
    }
  }
  else
  {
    Serial.println("No reply, is LoRa server running?");
  }
  delay(5000);
}

void loop()
{
     Serial.print(F("###########    "));
     Serial.print(F("COUNT="));
     Serial.print(count);
     Serial.println(F("    ###########"));
     count++;
     dhtTem();
     dhtWrite();
     SendData();
}

