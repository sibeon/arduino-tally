#include <FastLED.h>
#include <SPI.h>         // needed for Arduino versions later than 0018
#include <Ethernet.h>


// MAC address and IP address for this *particular* Ethernet Shield!
// MAC address is printed on the shield
// IP address is an available address you choose on your subnet where the switcher is also present:
byte mac[] = { 
  0x90, 0xA2, 0xDA, 0x0E, 0x0E, 0x07 };        // <= SETUP erledigt Letzte Stelle ist Kanal Nummer
//IPAddress clientIp(192, 168, 178, 205);        // <= SETUP!  IP address of the Arduino Letzte Stelle ist Kanal Nummer
IPAddress switcherIp(192, 168, 178, 240);     // <= SETUP!  IP address of the ATEM Switcher


// Include ATEM library and make an instance:
#include <ATEM.h>
#include <ATEMbase.h>
#include <ATEMstd.h>

// Connect to an ATEM switcher on this address and using this local port:
// The port number is chosen randomly among high numbers.
ATEMstd AtemSwitcher;

#define Kanal 4

bool gruen = 0;
bool rot = 0;

// Für LEDs
// How many leds are in the strip?
#define NUM_LEDS 6

// Data pin that led data will be written out over
#define DATA_PIN 7

// Clock pin only needed for SPI based chipsets when not using hardware SPI
//#define CLOCK_PIN 8

// This is an array of leds.  One item for each led in your strip.
CRGB leds[NUM_LEDS];

// This function sets up the ledsand tells the controller about them
void setup() {

  // Start the Ethernet, Serial (debugging) and UDP:
  Ethernet.begin(mac);
  Serial.begin(9600);  

  // Initialize a connection to the switcher:
  AtemSwitcher.begin(switcherIp);
  AtemSwitcher.serialOutput(2);
  AtemSwitcher.connect();
  
	// sanity check delay - allows reprogramming if accidently blowing power w/leds
   	delay(2000);
      FastLED.addLeds<WS2811, DATA_PIN, RGB>(leds, NUM_LEDS);
}

// This function runs over and over, and is where you do the magic to light
// your leds.
void loop() {
     
  // Check for packets, respond to them etc. Keeping the connection alive!
  AtemSwitcher.runLoop();
  Ethernet.maintain();

  if (AtemSwitcher.isConnected()){
        if (AtemSwitcher.getPreviewTally(Kanal) and not AtemSwitcher.getProgramTally(Kanal) and not gruen){
         
           for(int whiteLed = 0; whiteLed < NUM_LEDS; whiteLed = whiteLed + 1) {
              // Turn our current led on to white, then show the leds
              leds[whiteLed] = CRGB::Green;
        
           }
        // Show the leds (only one of which is set to white, from above)
              FastLED.show();
              Serial.println("Gruen");
              gruen = 1;
              rot = 0;
                 // Wait a little bit
        }
        
        if (AtemSwitcher.getProgramTally(Kanal) and not rot){
         
           for(int whiteLed = 0; whiteLed < NUM_LEDS; whiteLed = whiteLed + 1) {
              // Turn our current led on to white, then show the leds
              leds[whiteLed] = CRGB::Red;
        
           }
        // Show the leds (only one of which is set to white, from above)
              FastLED.show();
              Serial.println("Rot");
              rot = 1;
              gruen = 0;
                 // Wait a little bit
        }
        
        if (not AtemSwitcher.getProgramTally(Kanal) and not AtemSwitcher.getPreviewTally(Kanal)){
         
           for(int whiteLed = 0; whiteLed < NUM_LEDS; whiteLed = whiteLed + 1) {
              // Turn our current led on to white, then show the leds
              leds[whiteLed] = CRGB::Black;
        
           }
        // Show the leds (only one of which is set to white, from above)
              FastLED.show();
              Serial.println("Aus");
              rot = 0;
              gruen = 0;
                 // Wait a little bit
        }
  }
        // If connection is gone anyway, try to reconnect:
    else  {
    Serial.println("Connection to ATEM Switcher has timed out - reconnecting!");
    AtemSwitcher.connect();
  
    for(int whiteLed = 0; whiteLed < NUM_LEDS; whiteLed = whiteLed + 1) {
      // Turn our current led on to white, then show the leds
      leds[whiteLed] = CRGB::Black;

   }
// Show the leds (only one of which is set to white, from above)
      FastLED.show();
      Serial.println("Aus");
      rot = 0;
      gruen = 0;
         // Wait a little bit
  
  }  

}
