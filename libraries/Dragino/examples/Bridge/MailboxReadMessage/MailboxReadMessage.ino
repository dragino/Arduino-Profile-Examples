/*
  Read Messages from the Mailbox

 This example for the LG01(with the latest firmware > 4.0.0) shows how to
 read the messages queue, called Mailbox, using the Bridge library.
 The messages can be sent to the queue through REST calls. Appen the message in the URL after the keyword "/mailbox".
 
 How to use:
 When running this example, make sure your computer is on the same network as the LG01.
 Once you have uploaded the sketch on LG01 you can start to append messages in the LG01 mailbox. The Mailbox will be checked every 10 seconds and the available messages displayed on the Consoler.
 To use the REST APIs you need to insert the password or disable it from the Web panel. 
 
 You can use a browser with the following URL structure to send mailbox message: "http://LG01_IP_ADDRESS/mailbox/hello"
 or use a Linux machine to send mailbox via REST Call: "curl --user root:dragino "http://LG01_IP_ADDRESS/mailbox/hello""

 modified 22 May 2017
 by Dragino Tech <support@dragino.com>

 */

#include <Mailbox.h>

void setup() {
  pinMode(A2, OUTPUT);
  digitalWrite(A2, LOW);
  // Initialize Bridge and Mailbox
  Bridge.begin(115200);
  Mailbox.begin();
  digitalWrite(A2, HIGH);

  // Initialize Console
  Console.begin();

  // Wait until a Serial Monitor is connected.
  while (!Console);

  Console.println("Mailbox Read Message\n");
  Console.println("The Mailbox is checked every 10 seconds. The incoming messages will be shown below.\n");
}

void loop() {
  String message;

  // if there is a message in the Mailbox
  if (Mailbox.messageAvailable()) {
    // read all the messages present in the queue
    while (Mailbox.messageAvailable()) {
      Mailbox.readMessage(message);
      Console.println(message);
    }

    Console.println("Waiting 10 seconds before checking the Mailbox again");
  }

  // wait 10 seconds
  delay(10000);
}
