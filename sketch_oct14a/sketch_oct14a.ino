/**
  This code sends a message every 10 seconds.

  The sipmlest way to use the SX126x library, I could come up with.

  The package sending has been verified using a HackRF, looking at the waterfall pattern in the spectrum analyzer.

  As this is not a demo of how to send correct LoRa packages, but just to prove that the hardware is working correctly,
  it should be treated as such.
  No receive code is provided as this is not a goal at this time.

  This code proves:
  - Communication with the SX126x module over SPI
  - The signaling between the ESP32C6 and the RA-01SH module works correctly
  - Radio transmissions over LoRa are sent out
  - Module operates stable when sending packages

  The library used with this Sketch is: https://github.com/chandrawi/LoRaRF-Arduino
 */


#include <SPI.h>
#include <SX126x.h>
#include <string.h>

SX126x LoRa;

#define SS 1    // GPIO 1
#define RST 9  // GPIO 9 
#define DIO1 4  // dio1 avail not dio0
#define BUSY 5 // Busy line

#define SCK   0 // gpio0 
#define MOSI  2 // gpio 2
#define MISO  3 // gpio 3

//433E6 for Asia
//866E6 for Europe
//915E6 for North America
#define BAND 866E6

uint32_t counter = 0;

SPIClass * SPILoRa = NULL;
void setup() {
  // // initialize serial Monitor
  Serial.begin(115200);
  while (!Serial);
  Serial.println("\n\nLoRa Sender");

  Serial.println("Setup SPI pins");
  pinMode(SS, OUTPUT); // Chip se/lect to output
  pinMode(SCK, OUTPUT);
  pinMode(RST, OUTPUT);
  pinMode(MOSI, OUTPUT);
  pinMode(MISO, INPUT);

  digitalWrite(RST,LOW);
  delay(500);
  digitalWrite(RST, HIGH);
  delay(100);

  Serial.printf("\nInitializing SPI");
  SPILoRa = new SPIClass(SPI);
  SPILoRa->begin(SCK, MISO, MOSI, SS);
  Serial.println(", done");

  //SPI.begin(SCK, MISO, MOSI);
  LoRa.setSPI(*SPILoRa, 10e6);

  // setup LoRa tranceiver module
  while (!LoRa.begin(SS, RST, BUSY, DIO1)) {
    Serial.println("Lora config retry.");
    delay(1000);
  }

  // set transmit power to +22 dBm for SX1262
  LoRa.setTxPower(22, SX126X_TX_POWER_SX1262);

  // set receive gain for power saving
  LoRa.setRxGain(LORA_RX_GAIN_POWER_SAVING);

  //433E6 for Asia
  //866E6 for Europe
  //915E6 for North America
  Serial.println("Initialize LoRa");
  LoRa.setFrequency(BAND);


  // set spreading factor 8, bandwidth 125 kHz, coding rate 4/5, and low data rate optimization off
  LoRa.setLoRaModulation(8, 125000, 5, false);

  // set explicit header mode, preamble length 12, payload length 15, CRC on and no invert IQ operation
  LoRa.setLoRaPacket(LORA_HEADER_EXPLICIT, 192, 15, true, false);

  LoRa.setSyncWord(0xF3F3);

  LoRa.wait();

  Serial.println("LoRa Initialized OK!");
}

// Send a message over Lora
void sendMsg(char* message, uint8_t counter) {
  // message and counter to transmit
  LoRa.beginPacket();
  LoRa.write(message, sizeof(message)); // write multiple bytes
  LoRa.write(counter);                // write single byte
  LoRa.endPacket();
  LoRa.wait();
}


void loop() {
  if (counter > 255) {
    counter = 0;
  }
  String str;

  // put your main code here, to run repeatedly:
  Serial.println( str + "Sending packet on Freq: " + (BAND / 1E6) + "MHz, cnt: " + counter);

  // Send LoRa packet to receiver
  sendMsg("Hello World, here I LoRa!!", (uint8_t) counter);

  counter = counter + 1;

  delay(10000);
}
