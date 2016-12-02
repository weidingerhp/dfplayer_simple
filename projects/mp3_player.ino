#include <MP3Player.h>
#include "MP3Player.h"
#include <SoftwareSerial.h>

const byte rxPin = 3; // Wire this to Tx Pin of ESP8266
const byte txPin = 4; // Wire this to Rx Pin of ESP8266

            // We'll use a software serial interface to connect to ESP8266
SoftwareSerial softSerial(rxPin, txPin);

void commandReceived(byte cmd, byte par1, byte par2);
#define STATE_BIT_INITIALIZED 1
#define STATE_BIT_PLAYING 2
#define STATE_BIT_CARD_INSERTED 4
// 2+1 25 3 
int state = 0;
int current_track;
int current_folder_start;
int current_folder_size;
int all_files_size;
int num_folders;
int current_folder = 2;

// d7 -> d12 folder
// d6 high is reapeat low only one folder 
bool bts[8];


void setup() {
  for(int i = 0;i < 7; i++)
  {
    pinMode(i+7,INPUT_PULLUP);
    bts[i] = digitalRead(i+7);
  }
  pinMode(6,INPUT_PULLUP);

  state = 0;
  Serial.begin(9600);
  softSerial.begin(9600);

  softSerial.listen();
  
  delay(500);
  // clear everything on the input buffer
  while (softSerial.available() >0) softSerial.read();

  dfPlayerInit(softSerial);
  dfPlayerSetCallbackFunc(commandReceived);
  dfPlayerSetSpecialCallbackFunc(DFP_ERROR, errorReceived);
  dfPlayerSetSpecialCallbackFunc(DFP_REPLY, statusReceived);
  dfPlayerSetSpecialCallbackFunc(DFP_U_END_PLAY, trackFinished);

  // Init command
  dfPlayerSend(DFP_INIT, 0, 0);

  while (!dfPlayerLoop()) // Wait until initialization parameters are received (10 bytes)
    delay(30); // Pretty long delays between succesive commands needed (not always the same)
  delay(30);
  dfPlayerSend(DFP_VOLUME, 0, 15);
    
}

void loop() {
  // put your main code here, to run repeatedly:
    dfPlayerLoop();
    checkbt();
}

void playFolder(byte folder)
{
  current_folder = folder;
  delay(20);
  dfPlayerSend(DFP_SET_FOLDER_TRK, 0, current_folder);
  delay(20);
}

void checkbt()
{
  for(int i = 0; i < 7; i++)
  {
    bool a = digitalRead(i+7);
    if(bts[i] != a)
    {
      bts[i] = a;
      if(!a)
      {
        playFolder(i+1);
      }
    }
    
  }
}

// ===================================================================
// Special Callbacks
// ===================================================================
void errorReceived(byte cmd, byte par1, byte par2) {
  int par = ((int)par1) << 8 | ((int)par2);
  Serial.print("Error Code 0x");
  printHex(par, 4);
  Serial.println(" received !!");
}

void statusReceived(byte cmd, byte par1, byte par2) {
  int par = ((int)par1) << 8 | ((int)par2);
  Serial.print("Status Code 0x");
  printHex(par, 4);
  Serial.println(" received.");
}
void trackFinished(byte cmd, byte par1, byte par2) {
  int par = ((int)par1) << 8 | ((int)par2);
  if(par >= current_folder_start +current_folder_size)
  {
    //low = playFolder again
    //high = stop at the last track
    if(digitalRead(6))
    {
        playFolder(current_folder_start);
    }
    else
    {
      delay(20);
      dfPlayerSend(DFP_STOP, 0, 0);
      delay(20);
    }
    
  } 
  
  Serial.print("Status Code 0x");
  printHex(par, 4);
  Serial.println(" received.");
}
// ===================================================================
// Callback for general status Messages
// ===================================================================
void commandReceived(byte cmd, byte par1, byte par2) {
  char ch[10];
  delay(30);
  Serial.print("Got Response from Device : 0x");
  printHex(cmd, 2);
  Serial.print(", 0x");
  printHex(par1, 2);
  Serial.print(", 0x");
  printHex(par2, 2);
  Serial.println("");
  int par = ((int) par1) << 8 | ((int) par2);

  switch (cmd) {
  case DFP_INIT:
    state |= STATE_BIT_INITIALIZED;
    break;
  case DFP_QUERY_TF_FILES:
    Serial.print("Card has ");
    itoa(par, ch, 10);
    Serial.print(ch);
    Serial.println(" tracks.");
    all_files_size = par;
    dfPlayerSend(DFP_NUM_OF_FOLDERS, 0, 0);
    break;
  case DFP_NUM_OF_FOLDERS:
    Serial.print("Card has ");
    itoa(par, ch, 10);
    Serial.print(ch);
    Serial.println(" folders.");
    num_folders = par;
    delay(20);
    
    break;
  case DFP_PLUG_IN:
    state |= STATE_BIT_CARD_INSERTED;
    delay(500);
    dfPlayerSend(DFP_QUERY_TF_FILES, 0, 0);
    break;
  case DFP_PLUG_OUT:
    state &= ~STATE_BIT_CARD_INSERTED;
    break;
  case DFP_SET_FOLDER_TRK:
    Serial.print("Folder has ");
    itoa(par, ch, 10);
    Serial.print(ch);
    Serial.println(" tracks.");
    current_folder_size = par;
    dfPlayerSend(DFP_QUERY_TF_CUR, 0, 0);
    break;
  case DFP_QUERY_TF_CUR:
    Serial.print("Current file on disk is ");
    itoa(par, ch, 10);
    Serial.print(ch);
    Serial.println(".");
    current_track = par;
    dfPlayerSend(DFP_VOLUME, 0, 15);
    delay(20);
    dfPlayerSend(DFP_PLAY_FOLDER_L, 0x00, current_folder);
    break;

    default:
    Serial.println("Unhandled Response !!");
  }

  //if (cmd == 0x3a) {
  //  dfPlayerSend(DFP_SET_FOLDER_TRK, 0x00, 0x02);
  //}
  //if (cmd == DFP_SET_FOLDER_TRK) {
  //  dfPlayerSend(DFP_VOLUME, 0, 15);
  //  dfPlayerSend(DFP_PLAY_FOLDER_L, 0x00, 0x02);
  //}

}


// ===================================================================
// Helper Functions
// ===================================================================
void printHex(int val, byte len) {
  char ch[10];
  int digit = 1 << (4 * (len - 1));
  while ((val < digit) && (digit > 0x0f)) {
    Serial.print("0");
    digit = digit >> 4;
  }
  itoa(val, ch, 16);
  Serial.print(ch);
}













