/*
  Example to show how to write ID to the LoRa Gateway
  Support Devices: LG01 
 
  modified 24 Dec 2016
  by  Dragino Tech <support@dragino.com>
  Dragino Technology Co., Limited
*/

#include <Console.h>
#include <EEPROM.h>

//If you use Dragino IoT Mesh Firmware, uncomment below lines.
//For product: LG01. 
#define BAUDRATE 115200

/** the current address in the EEPROM (i.e. which byte we're going to write to next) **/
#define DEVICE_ID_ADDR 0x00 // Write to this address in EEPROM
int gateway_id = 255;//Write Gateway ID: 255
void setup() 
{
  Bridge.begin(BAUDRATE);
  Console.begin();

  while (!Console);
  
  EEPROM.write(DEVICE_ID_ADDR, gateway_id);
  delay(1000);
  Console.print("Device ID update to:");
  Console.println(EEPROM.read(DEVICE_ID_ADDR));
}

void loop() {

}
