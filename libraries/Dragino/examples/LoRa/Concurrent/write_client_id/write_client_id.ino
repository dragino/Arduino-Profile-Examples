/*
  Example to show how to write ID to the LoRa Node
  Support Devices: LoRa Shield + Arduino 
 
  modified 24 Dec 2016
  by  Dragino Tech <support@dragino.com>
  Dragino Technology Co., Limited
*/

#include <EEPROM.h>

/** the current address in the EEPROM (i.e. which byte we're going to write to next) **/
#define DEVICE_ID_ADDR 0x00 // Write to this address in EEPROM
int client_id = 1;//Write Client ID: 255
void setup() 
{
  Serial.begin(9600);

  while (!Serial);
  
  EEPROM.write(DEVICE_ID_ADDR, client_id);
  delay(1000);
  Serial.print("Device ID update to:");
  Serial.println(EEPROM.read(DEVICE_ID_ADDR));
}

void loop() {

}
