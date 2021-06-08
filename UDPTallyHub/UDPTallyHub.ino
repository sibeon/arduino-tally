/*
UDP Tally HUB

@Author Stefan Nöbauer
 */


#include <Ethernet.h>
#include <EthernetUdp.h>

// Include ATEM library and make an instance:
#include <ATEM.h>
#include <ATEMbase.h>
#include <ATEMstd.h>

#define USEATEMMOCK false

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = { 0xA8, 0x61, 0x0A, 0xAE, 0x87, 0x82 };
IPAddress ip(192, 168, 178, 220);
IPAddress IP_Remote(192, 168, 178, 255);
IPAddress switcherIp(192, 168, 178, 240);     // <= SETUP!  IP address of the ATEM Switcher

unsigned int localPort = 8888;      // local port to listen on

#define  OFF 0
#define  PREVIEW 1
#define  PROGRAM 2

#define CHANNELS 8

// buffers for receiving and sending data
char packetBuffer[UDP_TX_PACKET_MAX_SIZE];  // buffer to hold incoming packet,
char WriteBuffer[8];

// Connect to an ATEM switcher on this address and using this local port:
// The port number is chosen randomly among high numbers.
ATEMstd AtemSwitcher;

// An EthernetUDP instance to let us send and receive packets over UDP
EthernetUDP Udp;

void setup() {
  pinMode(2,INPUT);

  // You can use Ethernet.init(pin) to configure the CS pin
  Ethernet.init(10);  // Most Arduino shields

  // start the Ethernet
  Ethernet.begin(mac, ip);


  // Open serial communications and wait for port to open:
  Serial.begin(9600);

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
  AtemSwitcher.begin(switcherIp);
  //AtemSwitcher.serialOutput(2);
  AtemSwitcher.connect();

  // start UDP
  Udp.begin(localPort);
  
  Serial.print("Tally hub address:");
  Serial.print(Ethernet.localIP());
  Serial.print(":");
  Serial.println(Udp.localPort());

  delay(2000);
  
}

void loop() {


    // Check for packets, respond to them etc. Keeping the connection alive!
    if(!USEATEMMOCK) {
      AtemSwitcher.runLoop();
    }
    Ethernet.maintain();
    
    if (AtemSwitcher.isConnected()){
      //Serial.println("ATEM connected");
      for (int i = 1; i <= CHANNELS; i++) {
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

void send(int channel, int state) {
    // send a reply to the IP address and port that sent us the packet we received
    Udp.beginPacket(IP_Remote, 8889);

    sprintf(WriteBuffer, "TALLY%d|%d", channel, state);
    //Serial.println(WriteBuffer);
    Udp.write(WriteBuffer);
    Udp.endPacket();
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


