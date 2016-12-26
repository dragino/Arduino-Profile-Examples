/*
 
 Arduino GroveStreams Stream Feed via Ethernet
 
 This GroveStreams update_feed is designed for the LG01.
 A full "how to" guide for this sketh can be found at https://www.grovestreams.com/developers/getting_started_arduino_yun.html
 This sketch updates several stream feeds with an analog input reading,
 from a temperature probe, via the GroveStreams API: https://www.grovestreams.com/developers/api.html
 Use the Serial Monitor on the Arduino IDE to see verbose network feedback
 and the GroveStreams connectivity status.
 
 License:
  Copyright 2015 GroveStreams LLC.
  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at: http://www.apache.org/licenses/LICENSE-2.0
 
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
 
 GroveStreams Setup:
 
 * Sign Up for Free User Account - https://www.grovestreams.com
 * Create a GroveStreams organization while selecting the Arduino blueprint
 * Enter a unique MAC Address for this network in this sketch under "Local Network Settings"
 *    Navigate to http://your_dragino_ip to find your dragino's MAC address
 * Enter your GroveStreams secret api key under "GroveStreams Settings" in this sketch  
 *    (The api key can be retrieved from within a GroveStreams organization: click the Api Keys toolbar button,
 *     select your Api Key, and click View Secret Key. Paste the Secret Key below)
 
Device Requirements:
 * Dragino Board
 * Arduino 1.5.8 IDE or above

Modified : 2016-Dec-14 Support@dragino.com
 
*/
 
#include <Bridge.h>
#include <Process.h>

 //If you use Dragino IoT Mesh Firmware, uncomment below lines.
//For product: LG01. 
#define BAUDRATE 115200
 
// Local Network Settings
char mac[] = "A8:40:41:xx:xx:xx"; // Change this!!! Must be unique on local network.
                                  // Browse to your dragino to find it: http://your_device_ip
 
// GroveStreams Settings
char gsApiKey[] = "d0230096-2c8d-xxxx-xxxx-53ac9668b0af";   //Change This!!!
char gsComponentName[] = "Temperature";        //Optionally change. Set this to give your component a name when it initially registers.
 
char gsComponentTemplateId[] = "temp";  //Don't change. Tells GS what template to use when the feed initially arrives and a new component needs to be created.
                                        // The blueprint is expecting "temp".
 
//GroveStreams Stream IDs. Stream IDs tell GroveStreams which component streams the values will be assigned to.
//Don't change these unless you edit your GroveStreams component definition and change the stream IDs to match these.
char gsStreamId1[] = "s1";   //Don't change. Temp C.
char gsStreamId2[] = "s2";   //Don't change. Temp F.
 
// Other Settings
const unsigned long updateFrequency = 20000UL;    // Update frequency in milliseconds (20000 = 20 seconds). Change this to change your sample frequency.
 
const int temperaturePin = 0;          // You might need to change depending on the Pin you are using. The Temperature pin number.    
char samples[40];                      // Change this buffer size only if you increase or decrease the size of samples being uploaded.
 
unsigned long lastUploadTime = 0; //Don't change. Used to determine if samples need to be uploaded.
 
void setup()
{
 
  Bridge.begin(BAUDRATE);

  // Open Serial monitor and start here.
  Console.begin();
  while(!Console);
 
  //Wait for the connection to finish stopping
  delay(2500);
 
  //Set the mac and ip variables so that they can be used during sensor uploads later
  Console.print("MAC: ");
  Console.println(mac);
  Console.println();
 
}


char* trim(char* input)                                        
{
  //Trim leading and ending spaces
  int i,j;
  char *output=input;
  for (i = 0, j = 0; i<strlen(input); i++,j++)          
  {
    if (input[i]!=' ')                          
      output[j]=input[i];                    
    else
      j--;                                    
  }
  output[j]=0;
 
  return output;
}
 
char* getSamples()
{
  //Get the temperature analog reading and convert it to a string
  float voltage, degreesC, degreesF;
 
  voltage = (analogRead(temperaturePin) * 0.004882814);
  degreesC = (voltage - 0.5) * 100.0;
  degreesF = degreesC * (9.0/5.0) + 32.0;
 
  char tempC[15] = {0}; //Initialize buffer to nulls
  dtostrf(degreesC, 12, 3, tempC); //Convert float to string
 
  char tempF[15] = {0}; //Initialize buffer to nulls
  dtostrf(degreesF, 12, 3, tempF); //Convert float to string
 
  // Example: &s1=25.684&s2=78.231
  sprintf(samples, "&%s=%s&%s=%s", gsStreamId1, trim(tempC), gsStreamId2, trim(tempF));
 
  return samples;
}
  
void updateGroveStreams()
{
  Console.println("updateGroveStreams...");
 
  //Assemble the url that is used to pass the temperature readings to GroveStreams and call it
  lastUploadTime = millis();
 
  //You may need to increase the size of urlBuf if any other char array sizes have increased
  char urlBuf[180];
  sprintf(urlBuf, "http://grovestreams.com/api/feed?compTmplId=%s&compId=%s&compName=%s&api_key=%s%s",
                  gsComponentTemplateId, mac, gsComponentName, gsApiKey, getSamples());
                 
  char xHeadBuf[40];
  sprintf(xHeadBuf, "X-Forwarded-For:%s\0", mac);
                 
  //Uncomment the following lines to debug your strings and to ensure their buffers are large enough
  //Console.println(strlen(xHeadBuf));
  //Console.println(xHeadBuf);
  //Console.println(urlBuf);
  //Console.println(strlen(urlBuf));
  //Console.println(getSamples());
  //Console.println(strlen(getSamples()));
 
  Process process;
  process.begin("curl");
  process.addParameter("-k");
  process.addParameter("-X");
  process.addParameter("PUT");
 
  //Headers
  process.addParameter("-H");
  process.addParameter(xHeadBuf);
  process.addParameter("-H");
  process.addParameter("Connection:close");                      
  process.addParameter("-H");
  process.addParameter("Content-Type:application/json");  
 
  //URL
  process.addParameter(urlBuf);
 
  //Make the request
  unsigned int result = process.run();
 
  //Display whatever is returned (usually an error message if one occurred)
  while (process.available()>0) {
    char c = process.read();
    Console.print(c);
  }
  Console.flush();
 
  Console.print("Feed PUT Result: ");
  Console.println(result);
}
 
void loop()
{
 
  // Update sensor data to GroveStreams
  if(millis() - lastUploadTime > updateFrequency)
  {
    updateGroveStreams();
  }
}
