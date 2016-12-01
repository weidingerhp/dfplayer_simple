// MP3Player.h

#ifndef _MP3PLAYER_h
#define DFP__MP3PLAYER_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include <SoftwareSerial.h>

// Call this after the software-serial port has been enabled and set up (9600 baud are required for DFPlayer)
extern void dfPlayerInit(SoftwareSerial &serial);

// Callback function delegate for next step. 
// function prototype would be: 
// void myCallbackFunc(byte cmd, byte par1, byte par2)
// where cmd would be the command (e.g. 0x40 for an error) and par1, par2 would be the parameter
typedef void (*dfPlayerCallbackFunc)(byte, byte, byte);

extern void dfPlayerSetCallbackFunc(dfPlayerCallbackFunc func);

// Here you can register a special function that will be called if a given code is received.
// eg: you can register a special handler for 0x3a (card removed)
extern void dfPlayerSetSpecialCallbackFunc(byte code, dfPlayerCallbackFunc func);

// this needs to be called in the loop function of your INO-File.
extern bool dfPlayerLoop();

extern bool dfPlayerSend(byte cmd, byte par1, byte par2);


// DFPLayer Commands (Taken from https://github.com/neskweek/DFPlayer/blob/master/DFPlayer_SoftwareSerial.h)
// enhanced with info from http://forum.banggood.com/forum-topic-59997.html

#define DFP_NEXT 			0x01 //1 command "Next"
#define DFP_PREVIOUS 		0x02 //1 command "Previous"
#define DFP_PLAY_TRACK		0x03 //1 command "Play specified track"(NUM) 0-2999
#define DFP_VOL_UP			0x04 //1 command "Increase volume"
#define DFP_VOL_DWN			0x05 //1 command "Decrease volume"
#define DFP_VOLUME			0x06 //1 command "Set specified volume"  range :0-30
#define DFP_EQUALIZER		0x07 //1 command "Set Specified EQ" (0/1/2/3/4/5) Normal/Pop/Rock/Jazz/Classic/Base
#define DFP_PLAY_MODE  		0x08 //1 command "Set specified playback mode" (0/1/2/3) Repeat/folder repeat/single repeat/ random
#define DFP_SOURCE			0x09 //1 command "Set Specified playback source"(0/1/2/3/4) U/TF/AUX/SLEEP/FLASH
#define DFP_STANDBY			0x0A //1 command "Enter into standby/low power loss"
#define DFP_NORMAL			0x0B //1 command "Normal working"
#define DFP_RESET			0x0C //1 command "Reset module"
#define DFP_PLAYBACK		0x0D //1 command "Playback"
#define DFP_PAUSE			0x0E //1 command "Pause"
#define DFP_FOLDER  		0x0F //1 command "Play track in specified folder"
#define DFP_VOL_ADJUST 		0x10 //1 command "Volume adjust set" {DH:Open volume adjust} {DL: set volume gain 0~31}
#define DFP_REPEAT 			0x11 //1 command "Repeat play" {1:start repeat play} {0:stop play}2).
#define DFP_TRACK_FOLDER	0x12 //22 Specify MP3 tracks folder	0-9999
#define DFP_ADVERT 			0x13 //2	Commercials	0-9999
//#define DFP_UNKNOWN			0x14 //2	Support 15 folder
#define DFP_BACKGROUND		0x15 //2	Stop playback, play background
#define DFP_STOP			0x16 //2	Stop playback
#define DFP_PLAY_FOLDER_L	0x17 //4 [DH]=x, [DL]=1~255, Loops all files in folder named "(num)"
#define DFP_RANDOM			0x18 //4 Random all tracks, always starts at track 
#define DFP_SINGLE_REPEAT	0x19 //3	Single repeat mode
#define DFP_PAUSE_EXT		0x1A //4 [DH]=X, [DL]=(0x01:pause, 0x00:stop pause)
//#define DFP_UNKNOWN			0x1B //4
//#define DFP_UNKNOWN			0x1C //4
//#define DFP_UNKNOWN			0x1D //4
//#define DFP_UNKNOWN			0x1E //4
//#define DFP_UNKNOWN			0x1F //4
//#define DFP_UNKNOWN			0x20 //4
//#define DFP_UNKNOWN			0x21 //4
//#define DFP_UNKNOWN			0x22 //4
//#define DFP_UNKNOWN			0x23 //4
//#define DFP_UNKNOWN			0x24 //4
//#define DFP_UNKNOWN			0x25 //4
//#define DFP_UNKNOWN			0x26 //4
//#define DFP_UNKNOWN			0x27 //4
//#define DFP_UNKNOWN			0x28 //4
//#define DFP_UNKNOWN			0x29 //4
//#define DFP_UNKNOWN			0x2A //4
//#define DFP_UNKNOWN			0x2B //4
//#define DFP_UNKNOWN			0x2C //4
//#define DFP_UNKNOWN			0x2D //4
//#define DFP_UNKNOWN			0x2E //4
//#define DFP_UNKNOWN			0x2F //4
//#define DFP_UNKNOWN			0x30 //4
//#define DFP_UNKNOWN			0x31 //4
//#define DFP_UNKNOWN			0x32 //4
//#define DFP_UNKNOWN			0x33 //4
//#define DFP_UNKNOWN			0x34 //4
//#define DFP_UNKNOWN			0x35 //4
//#define DFP_UNKNOWN			0x36 //4
//#define DFP_UNKNOWN			0x37 //4
//#define DFP_UNKNOWN			0x38 //4
//#define DFP_UNKNOWN			0x39 //4
#define DFP_PLUG_IN 		0x3A //4
#define DFP_PLUG_OUT 		0x3B //4
#define DFP_U_END_PLAY 		0x3C //2U device finished playing last track
#define DFP_TF_END_PLAY		0x3D //2TF device finished playing last track
#define DFP_FLASH_END_PLAY 	0x3E //2STAY
#define DFP_INIT 			0x3F //2Send initialization parameters 0 - 0x0F(each bit represent one device of the low-four bits)
#define DFP_ERROR 			0x40 //2Returns an error, request retransmission
#define DFP_REPLY 			0x41 //2Reply
#define DFP_STATUS 			0x42 //2Query the current status
#define DFP_QUERY_VOLUME	0x43 //2Query the current volume
#define DFP_QUERY_EQ		0x44 //2Query the current EQ
#define DFP_QUERY_PLAYMODE 	0x45 //2Query the current playback mode
#define DFP_QUERY_VERSION 	0x46 //2Query the current software version
#define DFP_QUERY_U_FILES 	0x47 //2Query the total number of TF card files
#define DFP_QUERY_TF_FILES 	0x48 //2Query the total number of U-disk files
#define DFP_QUERY_F_FILES	0x49 //2Query the total number of flash files
#define DFP_QUERY_KEEPON 	0x4A //2Keep on
#define DFP_QUERY_U_CUR		0x4B //2Queries the current track of U-Disk
#define DFP_QUERY_TF_CUR 	0x4C //2Queries the current track of TF card
#define DFP_QUERY_F_CUR 	0x4D //2Queries the current track of Flash
#define DFP_SET_FOLDER_TRK	0x4E //[DH]=0, [DL]=(NUM) Change to first track in folder (NUM), Returns number of tracks in folder
#define DFP_NUM_OF_FOLDERS	0x4F //Total number of folders including root directory
#endif

