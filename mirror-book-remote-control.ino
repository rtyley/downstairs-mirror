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

/* The service information */

int32_t hrmServiceId;
int32_t hrmMeasureCharId;
int32_t hrmLocationCharId;

void setup() {

  while (!Serial); // required for Flora & Micro
  delay(500);

  boolean success;

  Serial.begin(115200);
  Serial.println(F("Adafruit Bluefruit Heart Rate Monitor (HRM) Example"));
  Serial.println(F("---------------------------------------------------"));

  randomSeed(micros());

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

  /* Add the Heart Rate Service definition */
  /* Service ID should be 1 */
  Serial.println(F("Adding the Heart Rate Service definition (UUID = 0x180D): "));
  success = ble.sendCommandWithIntReply( F("AT+GATTADDSERVICE=UUID=0x180D"), &hrmServiceId);
  if (! success) {
    error(F("Could not add HRM service"));
  }

  /* Add the Heart Rate Measurement characteristic */
  /* Chars ID for Measurement should be 1 */
  Serial.println(F("Adding the Heart Rate Measurement characteristic (UUID = 0x2A37): "));
  success = ble.sendCommandWithIntReply( F("AT+GATTADDCHAR=UUID=0x2A37, PROPERTIES=0x10, MIN_LEN=2, MAX_LEN=3, VALUE=00-40"), &hrmMeasureCharId);
    if (! success) {
    error(F("Could not add HRM characteristic"));
  }

  /* Add the Body Sensor Location characteristic */
  /* Chars ID for Body should be 2 */
  Serial.println(F("Adding the Body Sensor Location characteristic (UUID = 0x2A38): "));
  success = ble.sendCommandWithIntReply( F("AT+GATTADDCHAR=UUID=0x2A38, PROPERTIES=0x02, MIN_LEN=1, VALUE=3"), &hrmLocationCharId);
    if (! success) {
    error(F("Could not add BSL characteristic"));
  }

  /* Add the Heart Rate Service to the advertising data (needed for Nordic apps to detect the service) */
  Serial.print(F("Adding Heart Rate Service UUID to the advertising payload: "));
  ble.sendCommandCheckOK( F("AT+GAPSETADVDATA=02-01-06-05-02-0d-18-0a-18") );

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

  /* Command is sent when \n (\r) or println is called */
  /* AT+GATTCHAR=CharacteristicID,value */
  ble.print( F("AT+GATTCHAR=") );
  ble.print( hrmMeasureCharId );
  ble.print( F(",00-") );
  ble.println(newPosition, HEX);

  /* Check if command executed OK */
  if ( !ble.waitForOK() )
  {
    Serial.println(F("Failed to get response!"));
  }


  
  delay(100);

  if (oldPosition != newPosition) {
    changeTime = nowish;
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
