/*
  LG01P_gateway  :
  Support Devices: LG01. 
  
  Example sketch showing how to get sensor value via LoRa

  It is designed to work with the other example LG01S_DS18B20_LoRa

  modified 31 03 2017
  by Dragino Tech <support@dragino.com>
  Dragino Technology Co., Limited
*/

#include <SPI.h>

#include <RH_RF95.h>
#include <Console.h>
RH_RF95 rf95;
float frequency = 868.0;

//If you use Dragino IoT Mesh Firmware, uncomment below lines.
//For product: LG01. 
#define BAUDRATE 115200

//If you use Dragino Yun Mesh Firmware , uncomment below lines. 
//#define BAUDRATE 250000

void setup(void) 
{
    Bridge.begin(BAUDRATE);
    
    // Initialize Console
    Console.begin();
    if (!rf95.init())
      Console.println("init failed");
    
    rf95.setTxPower(20);
    rf95.setFrequency(frequency);
}

void loop()
{
  if (rf95.available())
  {
    // Should be a message for us now   
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);
    if (rf95.recv(buf, &len))
    {
        int a = buf[0];
        int b = buf[1];
        unsigned int value = a + b * 256;
        float celsius = (float)value / 100.0;  // get temperature value and print it out via console
        Console.print("  Temperature = "); 
        Console.print(celsius);
        Console.println(" ℃ ");
//     Console.print("RSSI: ");
//     Console.println(rf95.lastRssi(), DEC);
    }
    else
    {
     Console.println("recv failed");
    }
  }
}
