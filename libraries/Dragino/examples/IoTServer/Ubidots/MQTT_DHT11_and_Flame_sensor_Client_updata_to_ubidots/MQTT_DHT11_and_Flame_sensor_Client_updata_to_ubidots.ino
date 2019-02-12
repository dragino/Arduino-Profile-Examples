#include <dht.h>
#include <SPI.h>
#include <LoRa.h>

// Singleton instance of the radio driver

dht DHT;
#define DHT11_PIN A0
const int flame_pin=3;  //define the input pin of flame sensor
float temperature,humidity,tem,hum;
char tem_1[8]={"\0"},hum_1[8]={"\0"}; 
char *node_id = "<5678>";  //From LG01 via web Local Channel settings on MQTT.Please refer <> dataformat in here. 
uint8_t datasend[36];
unsigned int count = 1; 

void setup()
{
      Serial.begin(9600);
      while (!Serial);
      Serial.println(F("Start MQTT Example of Ubidots"));
      if (!LoRa.begin(868000000))   //868000000 is frequency
      { 
          Serial.println("Starting LoRa failed!");
          while (1);
      }
      LoRa.setSyncWord(0x34); 
      Serial.println("LoRa init succeeded.");
      
      pinMode(flame_pin,INPUT);
      attachInterrupt(1,fire,FALLING);
        
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
}
void dhtWrite()
{
    char data[50] = "\0";
    
    for(int i = 0; i < 50; i++)
    {
       data[i] = node_id[i];
    }
    
    dtostrf(tem,0,1,tem_1);
    dtostrf(hum,0,1,hum_1);

     strcat(data,"{");
     strcat(data,"\"temperature\":");
     strcat(data,tem_1);
     strcat(data,",");
     strcat(data,"\"humidity\":");
     strcat(data,hum_1);
     strcat(data,"}");
     strcpy((char *)datasend,data);
     
    //Serial.println((char *)datasend);    
}

void fire()  // Interrupt
{
     Serial.println("Have fire,the temperature is send");
     dhtTem(); 
     dhtWrite();
     LoRa.beginPacket();
     LoRa.print((char *)datasend);
     LoRa.endPacket();    
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
    Serial.print("###########    ");
    Serial.print("COUNT=");
    Serial.print(count);
    Serial.println("    ###########");
     count++;
     dhtTem();
     dhtWrite();
     SendData();
}

