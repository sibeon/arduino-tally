/*
UDP Tally HUB inclusive Tally Client

@Author Stefan Nöbauer
 */

#include <FastLED.h>
#include <Ethernet.h>
#include <EthernetUdp.h>

// Include ATEM library and make an instance:
#include <ATEM.h>
#include <ATEMbase.h>
#include <ATEMstd.h>

#define USEATEMMOCK false
// Define channel state
#define OFF 0
#define PREVIEW 1
#define PROGRAM 2
// Max supported channels that are distrtibuted by the hub
#define MAX_CHANNELS 8
// How many leds are in the strip?
#define NUM_LEDS 6
// Datapin led data will be written to
#define DATA_PIN 7

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
//byte mac[] = { 0xA8, 0x61, 0x0A, 0xAE, 0x87, 0x82 };
byte mac[] = { 0x90, 0xA2, 0xDA, 0x0E, 0x0E, 0x07 }; // Tally 4

IPAddress hubIP(192, 168, 178, 220);
IPAddress IP_Remote(192, 168, 178, 255);
IPAddress switcherIP(192, 168, 178, 240);     // <= SETUP!  IP address of the ATEM Switcher

unsigned int localPort = 8888;      // local port to listen on



// Current displayed color
CRGB currentColor = CRGB::Black;

// This is an array of leds.  One item for each led in your strip.
CRGB leds[NUM_LEDS];

// Connect to an ATEM switcher on this address and using this local port:
// The port number is chosen randomly among high numbers.
ATEMstd AtemSwitcher;

// An EthernetUDP instance to let us send and receive packets over UDP
EthernetUDP Udp;

// defined channel by digital pin 2, 3, 4, and 5 
int myChannel;

void setup() {
   // Open serial communications if exits
  Serial.begin(9600);

  pinMode(2, INPUT);
  pinMode(3, INPUT);
  pinMode(4, INPUT);
  pinMode(5, INPUT);

  readMyChannel();
  Serial.println("My Channel: " + String(myChannel));

  // You can use Ethernet.init(pin) to configure the CS pin
  Ethernet.init(10);  // Most Arduino shields
  // start the Ethernet
  Ethernet.begin(mac, hubIP);

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

  // Initialize a connection to the switcher:
  AtemSwitcher.begin(switcherIP);
  //AtemSwitcher.serialOutput(2);
  AtemSwitcher.connect();

  // start UDP
  Udp.begin(localPort);
  
  Serial.print("Tally hub address:");
  Serial.print(Ethernet.localIP());
  Serial.print(":");
  Serial.println(Udp.localPort());

  delay(2000);
  FastLED.addLeds<WS2811, DATA_PIN, RGB>(leds, NUM_LEDS);
}

void loop() {
    // Check for packets, respond to them etc. Keeping the connection alive!
    if(!USEATEMMOCK) {
      AtemSwitcher.runLoop();
    }
    Ethernet.maintain();
    
    if (AtemSwitcher.isConnected()){
      //Serial.println("ATEM connected");
      for (int i = 1; i <= MAX_CHANNELS; i++) {
        bool program = isProgram(i);
        bool preview = isPreview(i);
        int state = OFF;
        if(program and not preview) {
          state = PROGRAM;
        }
        if (not program and preview){
          state = PREVIEW;
        }
        if (not program and not preview){
          state = OFF;
        }
        if(program and preview) {
          state = PROGRAM;
        }
        send(i, state);
      }
    } else {
      Serial.println("Connection to ATEM Switcher has timed out - reconnecting!");
      AtemSwitcher.connect();
      delay(2000);
    }
}

void initTallies(){
  for (int i = 1; i <= MAX_CHANNELS; i++) {
    send(i, PREVIEW);
    delay(200);
    send(i, PROGRAM);
    delay(200);
    send(i, OFF);
  }
}

void send(int channel, int state) {
  if(myChannel == channel) {
    showState(state);  
  }
  else {
    // send a reply to the IP address and port that sent us the packet we received
    Udp.beginPacket(IP_Remote, 8889);
    char WriteBuffer[8];
    sprintf(WriteBuffer, "TALLY%d|%d", channel, state);
    //Serial.println(WriteBuffer);
    Udp.write(WriteBuffer);
    Udp.endPacket();
  }
    
}

bool isAtemConnected(){
    return AtemSwitcher.isConnected();  
}

bool isProgram(int channel) {
    return AtemSwitcher.getProgramTally(channel);
}

bool isPreview(int channel) {
    return AtemSwitcher.getPreviewTally(channel);
}

void showState(int state) {
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

void readMyChannel() {
  if(digitalRead(2) == HIGH) {
    bitSet(myChannel, 0);
  }
  if(digitalRead(3) == HIGH) {
    bitSet(myChannel, 1);
  }
  if(digitalRead(4) == HIGH) {
    bitSet(myChannel, 2);
  }
  if(digitalRead(5) == HIGH) {
    bitSet(myChannel, 3);
  }
}
