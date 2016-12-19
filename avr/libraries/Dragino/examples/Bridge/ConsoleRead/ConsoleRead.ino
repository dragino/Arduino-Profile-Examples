/*
  Console Read example

 Read data coming from bridge using the Console.read() function
 and store it in a string.

 This example shows the interactive between mcu and linux cpu.

 To see the Console, pick your Dragino's name and IP address in the Port menu
 then open the Port Monitor.When prompted for the password, enter it.

 create 4 Nov 2016
 by Edwin Chen <support@dragino.com>
 Dragino Technology Co., Limited
 */
 
//If you use Dragino IoT Mesh Firmware, uncomment below lines.
//For product: LG01. 
#define BAUDRATE 115200

//If you use Dragino Yun Mesh Firmware , uncomment below lines. 
//#define BAUDRATE 250000

#include <Console.h>

String name;

void setup() {
  // Initialize Console and wait for port to open:
  Bridge.begin(BAUDRATE);
  Console.begin();

  // Wait for Console port to connect
  while (!Console);

  Console.println("Hi, what's your name?");
}

void loop() {
  if (Console.available() > 0) {
    char c = Console.read(); // read the next char received
    // look for the newline character, this is the last character in the string
    if (c == '\n') {
      //print text with the name received
      Console.print("Hi ");
      Console.print(name);
      Console.println("! Nice to meet you!");
      Console.println();
      // Ask again for name and clear the old name
      Console.println("Hi, what's your name?");
      name = "";  // clear the name string
    } else {     // if the buffer is empty Cosole.read() returns -1
      name += c; // append the read char from Console to the name string
    }
  } else {
    delay(100);
  }
}
