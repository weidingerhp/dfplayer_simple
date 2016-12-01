// 
// 
// 

#include "MP3Player.h"
#include <SoftwareSerial.h>

SoftwareSerial *dfPlayerSerialPort = NULL;
dfPlayerCallbackFunc dfPlayerCallback = NULL;
byte dfPlayerInputBuffer[10];

dfPlayerCallbackFunc dfPlayerSpecialCallbacks[0x50];

# define DFP_Start_Byte 0x7E
# define DFP_Version_Byte 0xFF
# define DFP_Command_Length 0x06
# define DFP_End_Byte 0xEF
# define DFP_Acknowledge 0x01 //Returns info with command 0x41 [0x01: info, 0x00: no info]


void dfPlayerInit(SoftwareSerial &serial) {
	dfPlayerSerialPort = &serial;
	memset(dfPlayerSpecialCallbacks, 0, sizeof(dfPlayerSpecialCallbacks));
}


void dfPlayerSetCallbackFunc(dfPlayerCallbackFunc func) {
	dfPlayerCallback = func;
}

void dfPlayerSetSpecialCallbackFunc(byte code, dfPlayerCallbackFunc func) {
	if (code > 0x4f) return;
	dfPlayerSpecialCallbacks[code] = func;
}

int16_t dfPlayerCalculateCheckSum(byte cmd, byte par1, byte par2) {
	return -(DFP_Version_Byte + DFP_Command_Length + cmd + DFP_Acknowledge + par1 + par2);
}

bool dfPlayerLoop() {

	if (dfPlayerSerialPort == NULL) return false;
	if (dfPlayerCallback == NULL) return false;
	// according to test and "specs" the DFPlayer always sends 10 bytes on serial ... even if no params
	// are needed.
	if (dfPlayerSerialPort->available() >= 10) {
		for (byte k = 0; k < 10; k++) {
			dfPlayerInputBuffer[k] = dfPlayerSerialPort->read();
		}
		// TODO: we could check the checksum-word here

		byte code = dfPlayerInputBuffer[3];
		if (dfPlayerSpecialCallbacks[code] != NULL) {
			dfPlayerSpecialCallbacks[code](code, dfPlayerInputBuffer[5], dfPlayerInputBuffer[6]);
		} else {
			dfPlayerCallback(code, dfPlayerInputBuffer[5], dfPlayerInputBuffer[6]);
		}
		return true;
	}
}

bool dfPlayerSend(byte cmd, byte par1, byte par2) {
	if (dfPlayerSerialPort == NULL) return false;

	int16_t checksum = dfPlayerCalculateCheckSum(cmd, par1, par2);

	byte outputBuffer[] = { DFP_Start_Byte, DFP_Version_Byte, DFP_Command_Length, cmd, 
							DFP_Acknowledge, par1, par2, checksum >> 8, checksum & 0xFF, DFP_End_Byte };

	dfPlayerSerialPort->write(outputBuffer, 10);
	delay(20); // wait for 20msec (dfplayer is a little bit awkward about this... )
}
