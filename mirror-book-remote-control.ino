#include <Adafruit_DotStar.h>
// Because conditional #includes don't work w/Arduino sketches...
#include <SPI.h>         // COMMENT OUT THIS LINE FOR GEMMA OR TRINKET
//#include <avr/power.h> // ENABLE THIS LINE FOR GEMMA OR TRINKET

#include <Encoder.h>
#include <avr/sleep.h>

// Change these two numbers to the pins connected to your encoder.
//   Best Performance: both pins have interrupt capability
//   Good Performance: only the first pin has interrupt capability
//   Low Performance:  neither pin has interrupt capability
Encoder myEnc(0, 1);
//   avoid using pins with LEDs attached



#define NUMPIXELS 20 // Number of LEDs in strip

Adafruit_DotStar strip = Adafruit_DotStar(NUMPIXELS, DOTSTAR_BRG);

uint16_t oldPosition = -999;
unsigned long nowish,changeTime;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  
  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  
  Serial.begin(9600);
  Serial.println("Basic Encoder Test:");

  strip.begin(); // Initialize pins for output
  strip.show();  // Turn all LEDs off ASAP
  changeTime = millis();
}

uint32_t color = 0xfffffff;      // 'On' color (starts red)


void loop() {
  nowish = millis(); //prints time since program started
  Serial.println(nowish);
  uint16_t newPosition = (myEnc.read()/4)  % NUMPIXELS;
  Serial.println(newPosition);

  if (oldPosition != newPosition) {
    changeTime = nowish;
  }
  oldPosition = newPosition;
  
  if (nowish - changeTime > 10000) {
    Serial.println("sleepy time");
    strip.clear();
    strip.show();                     // Refresh strip
    digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    cli();                         //stop interrupts to ensure the BOD timed sequence executes as required
    sleep_enable();

    //disable brown-out detection while sleeping (20-25µA)
//    uint8_t mcucr1 = MCUCR | _BV(BODS) | _BV(BODSE);
//    uint8_t mcucr2 = mcucr1 & ~_BV(BODSE);
//    MCUCR = mcucr1;
//    MCUCR = mcucr2;

    //sleep_bod_disable();           //for AVR-GCC 4.3.3 and later, this is equivalent to the previous 4 lines of code
    
    sei();                         //ensure interrupts enabled so we can wake up again
    sleep_cpu();                   //go to sleep
    
    sleep_disable();               //wake up here
    changeTime = millis();
    digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)  
  }

  //strip.clear();
  strip.setPixelColor(newPosition, color); // 'On' pixel at head
  strip.show();                     // Refresh strip
}
