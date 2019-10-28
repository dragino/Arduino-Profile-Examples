/*******************************************************************************
 * Copyright (c) 2019 Thomas Telkamp and Matthijs Kooijman
 *
 * Permission is hereby granted, free of charge, to anyone
 * obtaining a copy of this document and accompanying files,
 * to do whatever they want with them without any restriction,
 * including, but not limited to, copying, modification and redistribution.
 * NO WARRANTY OF ANY KIND IS PROVIDED.
 *
 * This example sends a valid LoRaWAN packet with payload "Hello,
 * world!", using frequency and encryption settings matching those of
 * the The Things Network.
 *
 * This uses OTAA (Over-the-air activation), where where a DevEUI and
 * application key is configured, which are used in an over-the-air
 * activation procedure where a DevAddr and session keys are
 * assigned/generated for use with all further communication.
 *
 * Note: LoRaWAN per sub-band duty-cycle limitation is enforced (1% in
 * g1, 0.1% in g2), but not the TTN fair usage policy (which is probably
 * violated by this sketch when left running for longer)!

 * To use this sketch, first register your application and device with
 * the things network, to set or generate an AppEUI, DevEUI and AppKey.
 * Multiple devices can use the same AppEUI, but each device has its own
 * DevEUI and AppKey.
 *
 * Do not forget to define the radio type correctly in config.h.
 *
 *******************************************************************************/

#include <dht.h>
#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>

dht DHT;
#define DHT11_PIN A0
const int ctl_pin=4; //define the output pin of realy
const int flame_pin=3;  //define the input pin of flame sensor

// This EUI must be in little-endian format, so least-significant-byte
// first. When copying an EUI from ttnctl output, this means to reverse
// the bytes. For TTN issued EUIs the last bytes should be 0xD5, 0xB3,
// 0x70.
static const u1_t PROGMEM APPEUI[8]={ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
void os_getArtEui (u1_t* buf) { memcpy_P(buf, APPEUI, 8);}

// This should also be in little endian format, see above.
static const u1_t PROGMEM DEVEUI[8]={ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
void os_getDevEui (u1_t* buf) { memcpy_P(buf, DEVEUI, 8);}

// This key should be in big endian format (or, since it is not really a
// number but a block of memory, endianness does not really apply). In
// practice, a key taken from ttnctl can be copied as-is.
// The key shown here is the semtech default key.
static const u1_t PROGMEM APPKEY[16] ={ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
void os_getDevKey (u1_t* buf) {  memcpy_P(buf, APPKEY, 16);}

static float temperature,humidity,tem,hum;
static uint8_t LPP_data[13] = {0x01,0x67,0x00,0x00,0x02,0x68,0x00,0x03,0x01,0x00,0x04,0x00,0x00}; //0xO1,0x02,0x03,0x04 is Data Channel,0x67,0x68,0x01,0x00 is Data Type
static uint8_t opencml[4]={0x03,0x00,0x64,0xFF},closecml[4]={0x03,0x00,0x00,0xFF}; //the payload of the cayenne or ttn downlink 
static unsigned int count = 1; 

static osjob_t sendjob;

// Schedule TX every this many seconds (might become longer due to duty
// cycle limitations).
const unsigned TX_INTERVAL = 60;

// Pin mapping
const lmic_pinmap lmic_pins = {
    .nss = 10,
    .rxtx = LMIC_UNUSED_PIN,
    .rst = 9,
    .dio = {2, 6, 7},
};

void onEvent (ev_t ev) {
    Serial.print(os_getTime());
    Serial.print(": ");
    switch(ev) {
        case EV_SCAN_TIMEOUT:
            Serial.println(F("EV_SCAN_TIMEOUT"));
            break;
        case EV_BEACON_FOUND:
            Serial.println(F("EV_BEACON_FOUND"));
            break;
        case EV_BEACON_MISSED:
            Serial.println(F("EV_BEACON_MISSED"));
            break;
        case EV_BEACON_TRACKED:
            Serial.println(F("EV_BEACON_TRACKED"));
            break;
        case EV_JOINING:
            Serial.println(F("EV_JOINING"));
            break;
        case EV_JOINED:
            Serial.println(F("EV_JOINED"));

            // Disable link check validation (automatically enabled
            // during join, but not supported by TTN at this time).
            LMIC_setLinkCheckMode(0);
            break;
        case EV_RFU1:
            Serial.println(F("EV_RFU1"));
            break;
        case EV_JOIN_FAILED:
            Serial.println(F("EV_JOIN_FAILED"));
            break;
        case EV_REJOIN_FAILED:
            Serial.println(F("EV_REJOIN_FAILED"));
            break;
            break;
        case EV_TXCOMPLETE:
            Serial.println(F("EV_TXCOMPLETE (includes waiting for RX windows)"));
            if (LMIC.txrxFlags & TXRX_ACK)
              Serial.println(F("Received ack"));
           if(LMIC.dataLen>0)
            {
              int i,j=0;
              uint8_t received[4]={0x00,0x00,0x00,0x00};
               Serial.println("Received :");
              for(i=9;i<(9+LMIC.dataLen);i++)   //the received buf
              {
                Serial.print(LMIC.frame[i],HEX);
                received[j]=LMIC.frame[i];
                j++;
                Serial.print(" ");
               }
              Serial.println(); 
            if ((received[0]==opencml[0])&&(received[1]==opencml[1])&&(received[2]==opencml[2])&&(received[3]==opencml[3])) {
              Serial.println("Set pin to HIGH.");
              digitalWrite(ctl_pin, HIGH);
            }
            if ((received[0]==closecml[0])&&(received[1]==closecml[1])&&(received[2]==closecml[2])&&(received[3]==closecml[3])) {
              Serial.println("Set pin to LOW.");
               digitalWrite(ctl_pin, LOW);
            }
            }
            // Schedule next transmission
            os_setTimedCallback(&sendjob, os_getTime()+sec2osticks(TX_INTERVAL), do_send);
            break;
        case EV_LOST_TSYNC:
            Serial.println(F("EV_LOST_TSYNC"));
            break;
        case EV_RESET:
            Serial.println(F("EV_RESET"));
            break;
        case EV_RXCOMPLETE:
            // data received in ping slot
            Serial.println(F("EV_RXCOMPLETE"));
            break;
        case EV_LINK_DEAD:
            Serial.println(F("EV_LINK_DEAD"));
            break;
        case EV_LINK_ALIVE:
            Serial.println(F("EV_LINK_ALIVE"));
            break;
         default:
            Serial.println(F("Unknown event"));
            break;
    }
}

void dhtTem()
{
       int16_t tem_LPP;
       temperature = DHT.read11(DHT11_PIN);    //Read Tmperature data
       tem = DHT.temperature*1.0;      
       humidity = DHT.read11(DHT11_PIN);      //Read humidity data
       hum = DHT.humidity* 1.0; 
       Serial.print("###########    ");
       Serial.print("COUNT=");
       Serial.print(count);
       Serial.println("    ###########");            
       Serial.println(F("The temperature and humidity:"));
       Serial.print("[");
       Serial.print(tem);
       Serial.print("℃");
       Serial.print(",");
       Serial.print(hum);
       Serial.print("%");
       Serial.print("]");
       Serial.println("");
       count++;
       tem_LPP=tem * 10; 
       LPP_data[2] = tem_LPP>>8;
       LPP_data[3] = tem_LPP;
       LPP_data[6] = hum * 2;
}

void pinread()
{  
    int val,val1;
    val=digitalRead(ctl_pin);
    val1=digitalRead(flame_pin);
    if(val==1)
     {
        LPP_data[9]=0x01;
     }
    else
    {
        LPP_data[9]=0x00;
    }
    if(val1==1)
    {
      LPP_data[12]=0x01;
    }
    else
    {
      LPP_data[12]=0x00;
    }
}

void do_send(osjob_t* j){
    // Check if there is not a current TX/RX job running
    if (LMIC.opmode & OP_TXRXPEND) {
        Serial.println(F("OP_TXRXPEND, not sending"));
    } else {
        dhtTem();
        pinread();
        // Prepare upstream data transmission at the next possible time.
        LMIC_setTxData2(1,LPP_data, sizeof(LPP_data), 0);
        Serial.println(F("Packet queued"));
    }
    // Next TX is scheduled after TX_COMPLETE event.
}

void setup() {
    Serial.begin(9600);
    while(!Serial);
    Serial.println("Connect to TTN and Send data to mydevice cayenne(Use DHT11 Sensor):");

    pinMode(ctl_pin,OUTPUT);
    pinMode(flame_pin,INPUT);
//  attachInterrupt(1,fire,LOW);  //no connect Flame sensor should commented this code
    
    #ifdef VCC_ENABLE
    // For Pinoccio Scout boards
    pinMode(VCC_ENABLE, OUTPUT);
    digitalWrite(VCC_ENABLE, HIGH);
    delay(1000);
    #endif

    // LMIC init
    os_init();
    // Reset the MAC state. Session and pending data transfers will be discarded.
    LMIC_reset();

    // Start job (sending automatically starts OTAA too)
    do_send(&sendjob);
}

void fire()
{
     LPP_data[12]=0x00;
     dhtTem();
     LMIC_setTxData2(1,LPP_data, sizeof(LPP_data), 0);
     Serial.println("Have fire,the temperature is send");
}

void loop() {
     os_runloop_once();
}
