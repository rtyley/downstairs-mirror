   #include <Adafruit_DotStar.h>
// Because conditional #includes don't work w/Arduino sketches...
#include <SPI.h>         // COMMENT OUT THIS LINE FOR GEMMA OR TRINKET
//#include <avr/power.h> // ENABLE THIS LINE FOR GEMMA OR TRINKET


#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_SPI.h"
#include "Adafruit_BluefruitLE_UART.h"

#include "BluefruitConfig.h"

/* ...hardware SPI, using SCK/MOSI/MISO hardware SPI pins and then user selected CS/IRQ/RST */
Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);


#include <Encoder.h>
// #include <avr/sleep.h>

Encoder encA(A0, A1);
Encoder encB(A2, A3);
Encoder encC(A4, A5);
//   avoid using pins with LEDs attached



#define NUMPIXELS 20 // Number of LEDs in strip

Adafruit_DotStar strip = Adafruit_DotStar(NUMPIXELS, DOTSTAR_BRG);

uint16_t oldPosition = -999;
unsigned long nowish,changeTime;

// A small helper
void error(const __FlashStringHelper*err) {
  Serial.println(err);
  while (1);
}

// The service and characteristic index information 
int32_t gattServiceId;
int32_t gattNotifiableCharId;

void setup() {

  while (!Serial); // required for Flora & Micro
  delay(500);

  boolean success;

  Serial.begin(115200);
  Serial.println(F("Adafruit Bluefruit Heart Rate Monitor (HRM) Example"));
  Serial.println(F("---------------------------------------------------"));

  /* Initialise the module */
  Serial.print(F("Initialising the Bluefruit LE module: "));

  if ( !ble.begin(VERBOSE_MODE) )
  {
    error(F("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?"));
  }
  Serial.println( F("OK!") );

  /* Perform a factory reset to make sure everything is in a known state */
  Serial.println(F("Performing a factory reset: "));
  if (! ble.factoryReset() ){
       error(F("Couldn't factory reset"));
  }

  /* Disable command echo from Bluefruit */
  ble.echo(false);

  Serial.println("Requesting Bluefruit info:");
  /* Print Bluefruit information */
  ble.info();

  // this line is particularly required for Flora, but is a good idea
  // anyways for the super long lines ahead!
  // ble.setInterCharWriteDelay(5); // 5 ms

  /* Change the device name to make it easier to find */
  Serial.println(F("Setting device name to 'Book Remote': "));

  if (! ble.sendCommandCheckOK(F("AT+GAPDEVNAME=Book Remote")) ) {
    error(F("Could not set device name?"));
  }


  /* Add the Custom GATT Service definition */
  /* Service ID should be 1 */
  Serial.println(F("Adding the Custom GATT Service definition: "));
  success = ble.sendCommandWithIntReply( F("AT+GATTADDSERVICE=UUID128=F1-78-09-BF-BF-CE-4C-82-9E-58-52-30-90-E4-CA-03"), &gattServiceId);
  if (! success) {
    error(F("Could not add Custom GATT service"));
  }
 
  /* Add the Readable/Notifiable characteristic - this characteristic will be set every time the color of the LED is changed */
  /* Characteristic ID should be 1 */
  /* 0x00FF00 == RGB HEX of GREEN */
  Serial.println(F("Adding the Notifiable characteristic: "));
  success = ble.sendCommandWithIntReply( F("AT+GATTADDCHAR=UUID128=FD-D8-D9-70-9D-96-4A-C7-A2-E1-84-D2-04-F6-52-3B,PROPERTIES=0x12,MIN_LEN=3, MAX_LEN=3, VALUE=0x00FF00"), &gattNotifiableCharId);
    if (! success) {
    error(F("Could not add Custom Notifiable characteristic"));
  }
  

  /* Reset the device for the new service setting changes to take effect */
  Serial.print(F("Performing a SW reset (service changes require a reset): "));
  ble.reset();

  Serial.println();

  
  pinMode(LED_BUILTIN, OUTPUT);
  
  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
 

  strip.begin(); // Initialize pins for output
  strip.show();  // Turn all LEDs off ASAP
  changeTime = millis();
}

uint32_t color = 0xfffffff;      // 'On' color (starts red)


void loop() {
  nowish = millis(); //prints time since program started
  //Serial.println(nowish);
  uint16_t newPosition = (encA.read()/4)  % NUMPIXELS;
  Serial.println(encA.read());
  Serial.println(encB.read());
  Serial.println(encC.read());

  if (((encA.read() + encB.read() + encC.read()) / 4)%2 == 0) {
    digitalWrite(LED_BUILTIN, HIGH); 
  } else {
    digitalWrite(LED_BUILTIN, LOW);
  }
  
  delay(100);

  if (oldPosition != newPosition) {
    changeTime = nowish;
    /* Command is sent when \n (\r) or println is called */
    /* AT+GATTCHAR=CharacteristicID,value */
    ble.print( F("AT+GATTCHAR=") );
    ble.print( gattNotifiableCharId );
    ble.print( F(",") );
    ble.print(encA.read() & 0xFF, HEX);
    ble.print( F("-") );
    ble.print(encB.read() & 0xFF, HEX);
    ble.print( F("-") );
    ble.println(encC.read() & 0xFF, HEX);
  
    /* Check if command executed OK */
    if ( !ble.waitForOK() )
    {
      Serial.println(F("Failed to get response!"));
    }
  }
  oldPosition = newPosition;
  
  if (nowish - changeTime > 10000) {
    Serial.println("sleepy time");
    strip.clear();
    strip.show();                     // Refresh strip
    digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
//    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
//    cli();                         //stop interrupts to ensure the BOD timed sequence executes as required
//    sleep_enable();
//
//    //disable brown-out detection while sleeping (20-25µA)
////    uint8_t mcucr1 = MCUCR | _BV(BODS) | _BV(BODSE);
////    uint8_t mcucr2 = mcucr1 & ~_BV(BODSE);
////    MCUCR = mcucr1;
////    MCUCR = mcucr2;
//
//    //sleep_bod_disable();           //for AVR-GCC 4.3.3 and later, this is equivalent to the previous 4 lines of code
//    
//    sei();                         //ensure interrupts enabled so we can wake up again
//    sleep_cpu();                   //go to sleep
//    
//    sleep_disable();               //wake up here
    changeTime = millis();
    digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)  
  }

  //strip.clear();
  strip.setPixelColor(newPosition, color); // 'On' pixel at head
  strip.show();                     // Refresh strip
}
