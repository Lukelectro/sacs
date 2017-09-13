#ifndef __MAIN_H_INCLUDED__   // if x.h hasn't been included yet...
#define __MAIN_H_INCLUDED__   //   #define this so the compiler knows it has been included

#define DOOROPEN 10000

// The #defines that allow one to determine which board is selected are in board.txt as build.board. Prepend with "ARDUINO_"
#if defined(ARDUINO_AVR_NANO) || defined(ARDUINO_AVR_PRO)
//AVR specific stuff, like e.g. pin defenitions, here.

#if DOOROPEN>8000
#error "door open too long, WDT problem"
#endif

#define SS_PIN 9//ENC28j60 is on 10
#define RST_PIN 5 // ENC on RST, RC522 on PA9
#define IRQ_PIN 2 
#ifndef DOORPIN
#define DOORPIN 3
#endif
#ifndef BLINKLED
#define BLINKLED 4 
#endif
#define RFIDREADLED 6 

#ifndef ENPIN
#define ENPIN -1
#endif
#ifndef STEPPIN
#define STEPPIN -1
#endif
#ifndef DIRPIN
#define DIRPIN -1
#endif


#define YESENABLETHEAVRWDT

#elif defined(ARDUINO_GENERIC_STM32F103C)
//STM32F103 pindefs etc. here
#define SS_PIN PA1//ENC28j60 is on PA4 (STM32F103), RFID-RC522 on PA1
#define RST_PIN PA9 // ENC on RST, RC522 on PA9
#define IRQ_PIN PA2 
#ifndef DOORPIN
#define DOORPIN PB8
#endif
#ifndef ENPIN
#define ENPIN PB7
#endif
#ifndef STEPPIN
#define STEPPIN PB5
#endif
#ifndef DIRPIN
#define DIRPIN PB6
#endif
#ifndef BLINKLED
#define BLINKLED PC13 // PC13
#endif
#define RFIDREADLED PB4 // not used?

#else
#error "possibly unsupported hardware, use STM32F103C or Arduino Nano, or carefully and knowingly modify main0.h"
#endif


#ifndef INITIAL_DIR
#define INITIAL_DIR 1 // what direction does stepper go first on Open?
#endif
#if INITIAL_DIR > 1 || INITIAL_DIR < 0
#error "INITIAL_DIR must be boolean"
#endif

#define INTERVAL 300 // what is this for? Was 300. Since I get timed out (RC=-4) at MQTT connection, trying 500.

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
#error "geen serveradres!"
#define SERVER 000,000,000,000
#endif

#ifndef IP
#define IP 000,000,000,000//spacetest
#endif

#define FORMAT "[%d, %d, %d, %d]"
#ifndef INTOPIC
#define INTOPIC "deur/space_2/open" 
#endif
#ifndef OUTTOPIC
#define OUTTOPIC "deur/space_2/rfid"
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
