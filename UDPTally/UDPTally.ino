/*
UDP Tally Client

@Author Stefan Nöbauer
*/

#include <FastLED.h>
#include <Ethernet.h>
#include <EthernetUdp.h>

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
//byte mac[] = { 0x90, 0xA2, 0xDA, 0x0E, 0x0E, 0x56 }; // Tally 2
//byte mac[] = { 0x90, 0xA2, 0xDA, 0x0E, 0x0E, 0x08 }; // Tally 3
//byte mac[] = { 0x90, 0xA2, 0xDA, 0x0E, 0x0E, 0x07 }; // Tally 4
byte mac[] = { 0x90, 0xA2, 0xDA, 0x0E, 0x0E, 0x06 }; // Tally 5

unsigned int localPort = 8889;      // local port to listen on

int CHANNEL = 0;

#define  OFF '0'
#define  PREVIEW '1'
#define  PROGRAM '2'

// buffers for receiving and sending data
char packetBuffer[UDP_TX_PACKET_MAX_SIZE];  // buffer to hold incoming packet,

CRGB currentColor = CRGB::Black;

// Für LEDs
// How many leds are in the strip?
#define NUM_LEDS 6

// Data pin that led data will be written out over
#define DATA_PIN 7





// Clock pin only needed for SPI based chipsets when not using hardware SPI
//#define CLOCK_PIN 8

// This is an array of leds.  One item for each led in your strip.
CRGB leds[NUM_LEDS];

// An EthernetUDP instance to let us send and receive packets over UDP
EthernetUDP Udp;



void setup() {

  pinMode(2, INPUT);
  pinMode(3, INPUT);
  pinMode(4, INPUT);
  pinMode(5, INPUT);

  // You can use Ethernet.init(pin) to configure the CS pin
  Ethernet.init(10);  // Most Arduino shields

  // start the Ethernet
  Ethernet.begin(mac);

  // Open serial communications and wait for port to open:
  Serial.begin(9600);

  CHANNEL = readChannel();
  Serial.println(CHANNEL);
  
  // Check for Ethernet hardware present
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    while (true) {
      delay(1); // do nothing, no point running without Ethernet hardware
    }
  }
  while (Ethernet.linkStatus() == LinkOFF) {
    Serial.println("Ethernet cable is not connected.");
    delay(500);
  }

  // start UDP
  Udp.begin(localPort);
  
  Serial.print("Tally address:");
  Serial.print(Ethernet.localIP());
  Serial.print(":");
  Serial.println(Udp.localPort());

  // sanity check delay - allows reprogramming if accidently blowing power w/leds
  FastLED.addLeds<WS2811, DATA_PIN, RGB>(leds, NUM_LEDS);
}

void loop() {

  int packetSize = Udp.parsePacket();
  if (packetSize) {
    //log(packetSize);
    
    // read the packet into packetBufffer
    Udp.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);
    int channel = String(packetBuffer[5]).toInt();
    char state = packetBuffer[7];

    if(channel == CHANNEL) {
      Serial.println(state);
      showState(state);
    }
  }
}

void showState(char state) {
  switch (state) {
    case PREVIEW:
      showColor(CRGB::Green);
      break;
    case PROGRAM:
      showColor(CRGB::Red);
      break;
    case OFF:
      showColor(CRGB::Black);
      break;
    default:
      showColor(CRGB::Black);
      break;
  }
}

void showColor(CRGB newColor) {
  if(currentColor != newColor) {
    for(int whiteLed = 0; whiteLed < NUM_LEDS; whiteLed = whiteLed + 1) {
      // Turn our current led on to white, then show the leds
      leds[whiteLed] = newColor;
    }
    // Show the leds (only one of which is set to white, from above)
    FastLED.show();
    currentColor = newColor;
  }
} 


void log(int packetSize){
    Serial.print("Received packet of size ");
    Serial.println(packetSize);
    Serial.print("From ");
    IPAddress remote = Udp.remoteIP();
    for (int i=0; i < 4; i++) {
      Serial.print(remote[i], DEC);
      if (i < 3) {
        Serial.print(".");
      }
    }
    Serial.print(", port ");
    Serial.println(Udp.remotePort());
}

int readChannel() {
  int output = 0;

  if(digitalRead(2) == HIGH) {
    bitSet(output, 0);
  }
  if(digitalRead(3) == HIGH) {
    bitSet(output, 1);
  }
  if(digitalRead(4) == HIGH) {
    bitSet(output, 2);
  }
  if(digitalRead(5) == HIGH) {
    bitSet(output, 3);
  }
  return output;
}

