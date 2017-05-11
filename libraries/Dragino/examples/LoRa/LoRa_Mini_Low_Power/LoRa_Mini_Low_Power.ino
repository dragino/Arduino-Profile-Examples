/*
  LoRa Mini Low Power :
  Support Devices: LoRa Mini 
  
  Example to show how to put LoRa Mini in Low Power Mode

  modified 19 Apr 2017
  by Dragino <support@dragino.com>
  Dragino Technology Co., Limited
*/

#include <SPI.h>
#include <RH_RF95.h>
#include <avr/sleep.h>
#include <avr/wdt.h>

RH_RF95 rf95;

volatile byte wdt_counter=0;//Counter for Watch Dog

void setup() 
{
  Serial.begin(9600);
  if (!rf95.init())
    Serial.println("init failed");


// The modem config should be configured in RH_RF95.cpp : setModemConfig 
//  rf95.setModemConfig(Bw125Cr45Sf128); 
//  in this mode: transmit time: 52ms    Current: 126ma@3.3v   LoRa power: 23dBm  
//  Ideal Case: Send a packet each 24 seconds, 2 x AA Battery can last about 2 year. (attention:the time need to double check)

// The modem config should be configured in RH_RF95.cpp : setModemConfig 
//  rf95.setModemConfig(Bw500Cr45Sf128); 
//  in this mode: transmit time: 13ms    Current: 63ma@3.3v   LoRa power: 23dBm  
//  Ideal Case: Send a packet each 24 seconds, 2 x AA Battery can last about 4 years. (attention:the time need to double check) 

  rf95.setFrequency(915.0);
  rf95.setTxPower(23);
  setup_watchdog(9);//Set up WatchDog interupt time
// 0=16ms, 1=32ms,2=64ms,3=128ms,4=250ms,5=500ms
// 6=1 sec,7=2 sec, 8=4 sec, 9= 8sec

  // Entering Sleeping Mode
  ACSR |=_BV(ACD);//OFF ACD
  ADCSRA=0;//OFF ADC
  rf95.sleep();
  Sleep_avr();//Sleep_Mode, in sleep mode, the current is 22uA@3.3v
}

void loop()
{
  if (wdt_counter >= 3)// 3 interrupt counters then TX,total 24 Seconds 
  {
    Serial.println("Sending to rf95_server");
    uint8_t data[] = "Hello World!";
    rf95.send(data, sizeof(data));
  
    rf95.waitPacketSent();
    wdt_counter = 0; // Reset Counter

    // Entering Sleeping Mode
    ACSR |=_BV(ACD);//OFF ACD
    ADCSRA=0;//OFF ADC
    rf95.sleep();
    Sleep_avr();//Sleep_Mode
  }
  else
  {
    // Entering Sleeping Mode
    ACSR |=_BV(ACD);//OFF ACD
    ADCSRA=0;//OFF ADC
    rf95.sleep();
    Sleep_avr(); 
  }
}

//Set up sleep mode time. 
void setup_watchdog(int time) {

  byte wdt_time;

  if (time > 9 ) time = 9;
  wdt_time=time & 7;
  if (time > 7) wdt_time |= ( 1 << 5 );
  wdt_time |= ( 1 << WDCE );

  MCUSR &= ~( 1 << WDRF );//Clear WDRF in MCUSR
  
  // 启动时序
  WDTCSR |= ( 1 << WDCE) | ( 1 << WDE );
  
  // set up new watchdog timeout
  WDTCSR = wdt_time;
  WDTCSR |= _BV(WDIE);// close watchdog
}

//WDT interrupt
ISR(WDT_vect) {
  ++wdt_counter;
}

void Sleep_avr(){
  set_sleep_mode(SLEEP_MODE_PWR_DOWN  ); // set up sleep mode
  sleep_enable();  
  sleep_mode();                        // entering sleep mode
  rf95.sleep();
}
