/*
 Name:		SimpleMp3Player.ino
 Created:	25.11.2016 20:35:57
 Author:	hpw
*/

/*
 Info: 
 most complete info about dfplayer is on
 http://forum.banggood.com/forum-topic-59997.html

 layout of files on SD-Card should be:
 /
 + 01
 |  + 001.mp3
 |  + 002.mp3
 |  + 003.mp3
 + 02
 |  + 001.mp3

 a.s.o. (decimal numbers required - folders 1-99, files 001.mp3 - 255.mp3)


 Commands elaborated so far:
  + Playback from SD-Card (TF-card in this terminology) CMD: 
*/

// the setup function runs once when you press reset or power the board
#include "MP3Player.h"
#include <SoftwareSerial.h>

const byte rxPin = 3; // Wire this to Tx Pin of ESP8266
const byte txPin = 4; // Wire this to Rx Pin of ESP8266

					  // We'll use a software serial interface to connect to ESP8266
SoftwareSerial softSerial(rxPin, txPin);

void commandReceived(byte cmd, byte par1, byte par2);

int state = 0;

void setup()
{
	Serial.begin(9600);
	softSerial.begin(9600);

	softSerial.listen();
	
	delay(500);
	// clear everything on the input buffer
	while (softSerial.available() >0) softSerial.read();

	dfPlayerInit(softSerial);
	dfPlayerSetCallbackFunc(commandReceived);

	// Init command
	dfPlayerSend(DFP_INIT, 0, 0);

	while (!dfPlayerLoop()) // Wait until initialization parameters are received (10 bytes)
		delay(30); // Pretty long delays between succesive commands needed (not always the same)
	state = 1;
	dfPlayerSend(DFP_VOLUME, 0, 15);
	while (!dfPlayerLoop()) // Wait until initialization parameters are received (10 bytes)
		delay(30); // Pretty long delays between succesive commands needed (not always the same)
	dfPlayerSend(DFP_QUERY_TF_FILES, 0, 0);
}


void loop()
{
	dfPlayerLoop();
	
}

void commandReceived(byte cmd, byte par1, byte par2) {
	delay(30);
	Serial.print("Got Response from Device : 0x");
	printHex(cmd);
	Serial.print(", 0x");
	printHex(par1);
	Serial.print(", 0x");
	printHex(par2);
	Serial.println("");

	if (cmd == 0x3a) {
		dfPlayerSend(DFP_SET_FOLDER_TRK, 0x00, 0x02);
	}
	if (cmd == DFP_SET_FOLDER_TRK) {
		dfPlayerSend(DFP_VOLUME, 0, 15);
		dfPlayerSend(DFP_PLAY_FOLDER_L, 0x00, 0x02);
	}

}

void printHex(byte b) {
	char ch[3];
	if (b < 0x10) Serial.print("0");
	itoa(b, ch, 16);
	Serial.print(ch);
}
