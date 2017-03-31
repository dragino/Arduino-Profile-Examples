/*
  LG01S_DS18B20_LoRa :
  Support Devices: LG01-S
  
  Example sketch showing use the sensor pin of LG01-S, connect it to DS18B20 sensor 
  and get the temperature value then transfer to the gateway side via LoRa. 

  Use together with LG01P_gateway

  modified 30 03 2017
  by Edwin Chen <support@dragino.com>
  Dragino Technology Co., Limited
*/

#include <OneWire.h>
#include <SPI.h>

#include <RH_RF95.h>
#include <Console.h>
RH_RF95 rf95; 
float frequency = 868.0;
 
OneWire  ds(3);  // Connect to LG01-S 7A pin, which is Arduino D3

//If you use Dragino IoT Mesh Firmware, uncomment below lines.
//For product: LG01. 
#define BAUDRATE 115200

//If you use Dragino Yun Mesh Firmware , uncomment below lines. 
//#define BAUDRATE 250000
 
void setup(void) {
  Bridge.begin(BAUDRATE);
  
  // Initialize Console
  Console.begin();
  if (!rf95.init())
    Console.println("init failed");
    
  rf95.setTxPower(20);
  rf95.setFrequency(frequency);
}
 
void loop(void) 
{
    byte i;
    byte present = 0;
    byte type_s;
    byte data[12];
    byte addr[8];
    float celsius, fahrenheit;
    uint8_t dta[2] = {0};
    if ( !ds.search(addr))
    {
      Console.println("No more addresses.");
      Console.println();
      ds.reset_search();
      delay(2500);
      return;
    }
    Console.print("ROM =");
    for( i = 0; i < 8; i++) 
    {
      Console.write(' ');
      Console.print(addr[i], HEX);
    }
    if (OneWire::crc8(addr, 7) != addr[7])
    {
      Console.println("CRC is not valid!");
      return;
    }
    Console.println();
    // the first ROM byte indicates which chip
    switch (addr[0])
    {
      case 0x10:
        Console.println("  Chip = DS18S20");  // or old DS1820
        type_s = 1;
        break;
      case 0x28:
        Console.println("  Chip = DS18B20");
        type_s = 0;
        break;
      case 0x22:
        Console.println("  Chip = DS1822");
        type_s = 0;
        break;
      default:
        Console.println("Device is not a DS18x20 family device.");
        return;
    }   
    ds.reset();
    ds.select(addr);
    ds.write(0x44,1);         // start conversion, with parasite power on at the end  
     
    delay(10000);     // maybe 750ms is enough, maybe not
    // we might do a ds.depower() here, but the reset will take care of it.
    present = ds.reset();
    ds.select(addr);    
    ds.write(0xBE);         // Read Scratchpad  
   
    Console.print("  Data = ");
    Console.print(present,HEX);
    Console.print(" ");
    for ( i = 0; i < 9; i++) 
    {           // we need 9 bytes 
      data[i] = ds.read();
      Console.print(data[i], HEX);
      Console.print(" ");
    }
    Console.print(" CRC=");
    Console.print(OneWire::crc8(data, 8), HEX);
    Console.println();
   
    // convert the data to actual temperature 
    
    unsigned int raw = (data[1] << 8) | data[0];
    if (type_s) 
    {
      raw = raw << 3; // 9 bit resolution default
      if (data[7] == 0x10) 
      {
       // count remain gives full 12 bit resolution
       raw = (raw & 0xFFF0) + 12 - data[6];
      }
    }
   else 
    {
      byte cfg = (data[4] & 0x60);
      if (cfg == 0x00) raw = raw << 3;  // 9 bit resolution, 93.75 ms
      else if (cfg == 0x20) raw = raw << 2; // 10 bit res, 187.5 ms
      else if (cfg == 0x40) raw = raw << 1; // 11 bit res, 375 ms
      // default is 12 bit resolution, 750 ms conversion time
    }
    celsius = (float)raw / 16.0;
    int Temperature = celsius *100;
    dat[0] = Temperature;
    dat[1] = Temperature >> 8;  
    rf95.send(data, sizeof(data));  // Sent out temperature data
    Console.println( Temperature );
//    fahrenheit = celsius * 1.8 + 32.0;
//    Console.print("  Temperature = ");
//    Console.println(celsius);
//    Console.print(" Celsius, ");   
//    Console.print(fahrenheit);
//    Console.println(" Fahrenheit");
      delay(500);
}
