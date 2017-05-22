/*
  Running process using Process clas for LG01

 This sketch demonstrate how to run linux command in Arduino MCU side. 

 created 5 Jun 2013
 by Cristian Maglie
 Modified 22 May 2017
 by Dragino Tech <support@dragino.com>

 This example code is in the public domain.

 http://www.arduino.cc/en/Tutorial/Process

 */

#include <Process.h>

void setup() {
  // Initialize Bridge
  Bridge.begin(115200);

  // Initialize Serial
  Console.begin();

  // Wait until a Serial Monitor is connected.
  while (!Console);

  // run various example processes
  runDeviceInfo();
  runCpuInfo();
}

void loop() {
  // Do nothing here.
}

void runDeviceInfo() {
  // Launch "curl" command and get Arduino ascii art logo from the network
  // curl is command line program for transferring data using different internet protocols
  Process p;		// Create a process and call it "p"
  p.begin("cat");	// Process that launch the "cat" command
  p.addParameter("/etc/banner");  // Process that launch the "cat" command
  p.run();		// Run the process and wait for its termination

  // Print arduino logo over the Serial
  // A process output can be read with the stream methods
  while (p.available() > 0) {
    char c = p.read();
    Console.print(c);
  }
  // Ensure the last bit of data is sent.
  Console.flush();
}

void runCpuInfo() {
  // Launch "cat /proc/cpuinfo" command (shows info on Atheros CPU)
  // cat is a command line utility that shows the content of a file
  Process p;		// Create a process and call it "p"
  p.begin("cat");	// Process that launch the "cat" command
  p.addParameter("/proc/cpuinfo"); // Add the cpuifo file path as parameter to cut
  p.run();		// Run the process and wait for its termination

  // Print command output on the SerialUSB.
  // A process output can be read with the stream methods
  while (p.available() > 0) {
    char c = p.read();
    Console.print(c);
  }
  // Ensure the last bit of data is sent.
  Console.flush();
}

