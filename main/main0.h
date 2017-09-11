#ifndef __MAIN_H_INCLUDED__   // if x.h hasn't been included yet...
#define __MAIN_H_INCLUDED__   //   #define this so the compiler knows it has been included
#define SS_PIN 9//ENC28j60 is on 10
#define RST_PIN 5
#define IRQ_PIN 2
#ifndef DOORPIN
#define DOORPIN 3
#endif
#ifndef BLINKLED
#define BLINKLED 4
#endif
#define RFIDREADLED 6
#define INTERVAL 300
#ifndef NODE_PASSWD
#define NODE_PASSWD "xxx"
#endif
#ifndef DEBUG
#define DEBUG 1
#endif
#ifndef MACADDRESS
#define MACADDRESS 0xDE, 0xED, 0xBA, 0xFD, 0xFD, 0xEC
#endif
#ifndef SERVER

#define SERVER 000,000,000,000
#endif
#ifndef IP

#define IP 000,000,000,000//spacetest
#endif

#define FORMAT "[%d, %d, %d, %d]"
#ifndef INTOPIC
#define INTOPIC "deur/space2/open" 
#endif
#ifndef OUTTOPIC
#define OUTTOPIC "deur/space2/rfid"
#endif
#define DOOROPEN 5000
#if DOOROPEN>8000
#error "door open too long, WDT problem"
#endif
//mqtt props
#define QOS 0//quality of service mqtt
#define RETAIN 0//

int freeRam ();
void mqtt_callback(char* topic, byte* payload, unsigned int length);
void setup(void);
void irq_callback();
void clearIRQ(MFRC522 mfrc522);
void activateRec(MFRC522 mfrc522);
void reconnect();
byte copy_to_mqtttag(byte *buffer, byte bufferSize);
void IRQ_invoked();
void sometimes();
void loop(void);
#endif
