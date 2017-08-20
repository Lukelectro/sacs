/*
    main.ino

    Copyright 2017 marcell marosvolgyi <marcell@stereodyne.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <SPI.h>
#include <avr/wdt.h>
#include <UIPEthernet.h>
#include <PubSubClient.h>
#include <MFRC522.h>
#include "main.h"

//them globals
MFRC522 mfrc522(SS_PIN, RST_PIN);

//definitely to unload the irq subroutine, but maybe
//mqtt callback too. Have to look into this flow...
struct Flags
{
  bool newgoodread;
  bool doread;
  bool opendoor;
};

const byte ip[4] = { IP };
const uint8_t mac[6] = { MACADDRESS };
const byte server[4] = { SERVER };
const int qos = QOS;//quality of service of the mqtt subscription
const int retain = RETAIN;

//have to think about the best strategy for the qos and retain...

Flags flags;
bool toggleled = false;
long previousMillis;
char mqttstring[20];

EthernetClient ethClient;
PubSubClient mqttclient(server, 1883, mqtt_callback, ethClient);

ISR(WDT_vect) {
  //
}

int freeRam () {
  //https://playground.arduino.cc/Code/AvailableMemory
  extern int __heap_start, *__brkval;
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}

void mqtt_callback(char* topic, byte* payload, unsigned int length) {

  Serial.println(F("received callback invoked"));

  if (strcmp(topic, INTOPIC) == 0) {
    Serial.println(F("received deur open"));
    flags.opendoor = true;
  }
}

void irq_callback() {
  flags.doread = true;
  //we clear the IRQflag in the main loop
}

void clearIRQ(MFRC522 mfrc522) {
  mfrc522.PCD_WriteRegister(mfrc522.ComIrqReg, 0x7F);
}

void activateRec(MFRC522 mfrc522) {
  // The function sending to the MFRC522 the needed
  //commands to activate the reception
  mfrc522.PCD_WriteRegister(mfrc522.FIFODataReg, mfrc522.PICC_CMD_REQA);
  mfrc522.PCD_WriteRegister(mfrc522.CommandReg, mfrc522.PCD_Transceive);
  mfrc522.PCD_WriteRegister(mfrc522.BitFramingReg, 0x87);
}

void setup() {

  flags.doread = false;
  flags.newgoodread = false;
  flags.opendoor = false;

  Serial.begin(9600);
  SPI.begin();
  mfrc522.PCD_Init();
  //mfrc522.PCD_SetAntennaGain(0x07);
  //read and printout the MFRC522 version (valid values 0x91 & 0x92)
  Serial.print(F("MFRC Ver: 0x"));
  byte readReg = mfrc522.PCD_ReadRegister(mfrc522.VersionReg);
  Serial.println(readReg, HEX);

  //if(Ethernet.begin(mac) == 0) {
  if (Ethernet.begin(mac) == 0) {
    Serial.println(F("Ethernet configuration using DHCP failed"));
    Ethernet.begin(mac, ip);
    //for(;;);
  } else {
    Serial.println(F("DHCP success..."));
  }

  //the interrupt stuff from
  //https://github.com/miguelbalboa/rfid/blob/master/examples/MinimalInterrupt/MinimalInterrupt.ino

  pinMode(DOORPIN, OUTPUT);
  pinMode(BLINKLED, OUTPUT);
  pinMode(RFIDREADLED, OUTPUT);
  pinMode(IRQ_PIN, INPUT_PULLUP);

  // Allow the ... irq to be propagated to the IRQ pin
  // For test purposes propagate the IdleIrq and loAlert
  //regVal = 0xA0; //rx irq
  //A0 = 0b10100000 IRqInv en RxIEN zetten  (Datasheet NXP)
  mfrc522.PCD_WriteRegister(mfrc522.ComIEnReg, 0xA0);

  // Activate the interrupt
  attachInterrupt(digitalPinToInterrupt(IRQ_PIN), irq_callback, FALLING);
  delay(500);
  activateRec(mfrc522);
  wdt_enable(WDTO_8S);
}

void reconnect() {
  // Loop until we're reconnected
  while (!mqttclient.connected()) {
    Serial.print(F("Attempting MQTT connection..."));
    // Attempt to connect
    if (mqttclient.connect("ac")) {
      Serial.println(F("connected"));
      // Once connected, publish an announcement...
      //mqttclient.publish("vrdr","hi");
      // ... and resubscribe
      mqttclient.subscribe(INTOPIC, qos);
    } else {
      Serial.print(F("failed, rc="));
      Serial.print(mqttclient.state());
      Serial.println(F(" try again in 5 seconds"));
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

//not sure about the data types here...
byte copy_to_mqtttag(byte *buffer, byte bufferSize) {
  //we dont want more than 4 anyway
  if (bufferSize < 4) return -1;

  byte ret = snprintf(mqttstring, sizeof(mqttstring), FORMAT,
                      buffer[0], buffer[1], buffer[2], buffer[3] );
  return 0;
}

void IRQ_invoked() {
  digitalWrite(RFIDREADLED, HIGH);

  if (mfrc522.PICC_ReadCardSerial() && mfrc522.uid.size) {
    /* Show some details of the PICC// (that is: the tag/card)
      uidByte is a pointer to a buffer which is not always cleared
      despite a new trigger. This means, someone without access trying
      to enter might get access through previous tag id. I clear the
      buffer here in copy function UGLY, this should be prevented, or
      the library should clearitsself.
    */
    byte call = copy_to_mqtttag(mfrc522.uid.uidByte, mfrc522.uid.size);
    //if call!=0 handle it somehow...TODO
    flags.newgoodread = (call == 0);
    mfrc522.uid.size = 0; // we're done with the UID, clear it.y
#if DEBUG==1
    Serial.print(F("Card UID:"));
    if (flags.newgoodread)
      Serial.println(mqttstring);
    else
      Serial.println(F("Bad read"));
    Serial.println();
#endif
  }
  clearIRQ(mfrc522);
  //mfrc522.PICC_HaltA();
}

void loop() {
  //hw interrupt sets me
  //
  if (flags.doread) {
    //I set newfoodread flag
    IRQ_invoked();
    flags.doread = false;
  }
  // The receiving block needs regular retriggering
  //(tell the tag it should transmit??)
  // (mfrc522.PCD_WriteRegister(mfrc522.FIFODataReg,mfrc522.PICC_CMD_REQA);)
  //
  unsigned long lastActivate = 0;
  if (millis() - lastActivate > 5000) {
    activateRec(mfrc522);
    lastActivate = millis();
  }

  if (!mqttclient.connected()) {
    reconnect();
  }

  mqttclient.loop();

  //the interrupt triggered IRQ_invoked, subroutine decides
  if (flags.newgoodread) {
    mqttclient.publish(OUTTOPIC , mqttstring, retain);
    flags.newgoodread = false;
  }

  unsigned long lastOpenStart = 0;
  if (flags.opendoor) {
    if (lastOpenStart == 0) {
      digitalWrite(DOORPIN, HIGH);
      lastOpenStart = millis() & 1;
    };
    if (millis() - lastOpenStart > DOOROPEN) {
      digitalWrite(DOORPIN, LOW);
      //undo_stuff();
      flags.opendoor = false;
      lastOpenStart = 0;
    }
  }

  wdt_reset();
  delay(100);
}
