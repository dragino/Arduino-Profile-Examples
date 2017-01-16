/*
 Examples to show how to publish data to Baidu IoT hub using the MQTT protocol. https://cloud.baidu.com/doc/IOT/Quickstart.html 

Device Requirements:
 * LG01
 * Arduino 1.5.8 IDE or above

Modified : 2016-Dec-14 Support@dragino.com

 */


#include <Process.h>
unsigned long lastMillis = 0;


//If you use Dragino IoT Mesh Firmware, uncomment below lines.
//For product: LG01. 
#define BAUDRATE 115200

//If you use Dragino Yun Firmware, uncomment below lines.
//For product: Yun Shield 
//#define BAUDRATE 250000

void setup() 
{
    // Initialize Bridge
    Bridge.begin(BAUDRATE);
   // Initialize Serial
    Console.begin();
}

void loop()
{
    if(millis() - lastMillis > 20000) // Upload Per 20 seconds
    {
        lastMillis = millis();
        Console.println("Publish a data Baidu");
        mqtt_publish();
    }
}

String generate_data() // Generate a random data
{
    String count = "" ;
    count = random(100, 500);
    return count;
}

void mqtt_publish()
{
    Process p;    // Create a process and call it "p"
    p.begin("mosquitto_pub"); // Process that launch the "mosquitto_pub" command
    p.addParameter("-d");
    p.addParameter("-h");
    p.addParameter("lishunan.mqtt.iot.gz.baidubce.com");
    p.addParameter("-i");
    p.addParameter("deviceId-wY8HTBUTtu");  // Add Baidu Device ID
    p.addParameter("-p");
    p.addParameter("1883");
    p.addParameter("-q");
    p.addParameter("0");
    p.addParameter("-m");
    p.addParameter(generate_data());// Add data
    p.addParameter("-u");
    p.addParameter("lishunan/mything");      // User Name for Baidu IoT hub
    p.addParameter("-P");
    p.addParameter("a6bv9iszcxxxxxxxxxc9mq1vnkPwugjiWRB+Q=");   // Password for Biadu IoT Hub
    p.addParameter("-t");
    p.addParameter("TestTopic");    // Publish to this topic
    p.run();    // Run the process and wait for its termination
  
    // Print arduino logo over the Serial
    // A process output can be read with the stream methods
    while (p.available() > 0) {
      char c = p.read();
      Console.print(c);
    }
  //   Ensure the last bit of data is sent.
    Console.flush();
}

