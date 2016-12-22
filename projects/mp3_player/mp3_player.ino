#include <Bounce2.h>

//#include <MP3Player.h>
#include "MP3Player.h"
#include <SoftwareSerial.h>

const byte rxPin = 3; // Wire this to Tx Pin of ESP8266
const byte txPin = 4; // Wire this to Rx Pin of ESP8266

      // We'll use a software serial interface to connect to ESP8266
SoftwareSerial softSerial(rxPin, txPin);

void commandReceived(byte cmd, byte par1, byte par2);
void printUnknownCommand(byte cmd, byte par1, byte par2);

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
int play=0;
int firststart = 0;
Bounce bouncer = Bounce();

// d7 -> d12 folder
// d6 high is reapeat low only one folder 
bool bts[6];

void setup() {
  for (int i = 0; i < 6; i++) {
    pinMode(i + 6, INPUT_PULLUP);
    bts[i] = digitalRead(i + 6);
  }
  pinMode(6, INPUT_PULLUP);
  pinMode(12,INPUT_PULLUP);
  bouncer.attach(12);
  
  state = 0;
  play = 0;
  firststart = 0;
  Serial.begin(9600);
  softSerial.begin(9600);

  softSerial.listen();

  delay(500);
  // clear everything on the input buffer
  while (softSerial.available() > 0) softSerial.read();

  dfPlayerInit(softSerial);
  dfPlayerSetCallbackFunc(commandReceived);
  dfPlayerSetSpecialCallbackFunc(DFP_ERROR, errorReceived);
  dfPlayerSetSpecialCallbackFunc(DFP_REPLY, statusReceived);
  dfPlayerSetSpecialCallbackFunc(DFP_U_END_PLAY, trackFinished);
  dfPlayerSetSpecialCallbackFunc(DFP_SET_FOLDER_TRK, startFolderCallbacks);
  dfPlayerSetSpecialCallbackFunc(DFP_QUERY_TF_CUR, startFolderCallbacks);
  dfPlayerSetSpecialCallbackFunc(DFP_PLUG_IN, cardInserted);
  dfPlayerSetSpecialCallbackFunc(DFP_QUERY_TF_FILES, readInitialCardInfos);
  dfPlayerSetSpecialCallbackFunc(DFP_NUM_OF_FOLDERS, readInitialCardInfos);

  // Init command 
  dfPlayerSend(DFP_INIT, 0, 0);

  while (!dfPlayerLoop()) // Wait until initialization parameters are received (10 bytes)
    delay(30); // Pretty long delays between succesive commands needed (not always the same)
  delay(30);
  dfPlayerSend(DFP_VOLUME, 0, 15);
  while (!dfPlayerLoop()) // Wait until initialization parameters are received (10 bytes)
    delay(30); // Pretty long delays between succesive commands needed (not always the same)
  delay(30);
  // query files (this also triggers query folders in the callback)
  dfPlayerSend(DFP_QUERY_TF_FILES, 0, 0);
}

void loop() {
  // put your main code here, to run repeatedly:
  dfPlayerLoop();
  checkbt();
  bouncer.update();
  bool fell = bouncer.fell();
  if(fell) {
    if(play==0) {
      play = 1;
      if(firststart==0) {
          firststart=1;
          playFolder(current_folder);
        }
        else {
          resume();
        }
    }
    else {
      play=0;
      pause();
    }
  }
}

void pause() {
  delay(20);
  dfPlayerSend(DFP_PAUSE, 0, 0);
  delay(20);
}

void resume() {
  delay(20);
  dfPlayerSend(DFP_PLAYBACK,0,0);
  delay(20);
}

void playFolder(byte folder) {
  current_folder = folder;
  delay(20);
  dfPlayerSend(DFP_SET_FOLDER_TRK, 0, current_folder);
  delay(20);
}

void checkbt() {
  for (int i = 0; i < 6; i++) {
    bool a = digitalRead(i + 6);
    if (bts[i] != a) {
      bts[i] = a;
      if (!a) {
        playFolder(i + 1);
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
  if (par >= current_folder_start + current_folder_size) {
    //low = playFolder again
    //high = stop at the last track
    if (digitalRead(6)) {
      playFolder(current_folder_start);
    } else {
      delay(20);
      dfPlayerSend(DFP_STOP, 0, 0);
      delay(20);
    }
  }
}

void startFolderCallbacks(byte cmd, byte par1, byte par2) {
  int par = ((int)par1) << 8 | ((int)par2);

  switch (cmd) {
  case DFP_SET_FOLDER_TRK:
    current_folder_size = par;
    delay(20);
    dfPlayerSend(DFP_QUERY_TF_CUR, 0, 0);
    break;
  case DFP_QUERY_TF_CUR:
    current_track = par;

    // Set Volume and start playing
    delay(20);
    dfPlayerSend(DFP_VOLUME, 0, 15);
    delay(20);
    dfPlayerSend(DFP_PLAY_FOLDER_L, 0x00, current_folder);
    break;
  default:
    printUnknownCommand(cmd, par1, par2);
  }
}

void readInitialCardInfos(byte cmd, byte par1, byte par2) {
  int par = ((int)par1) << 8 | ((int)par2);
  switch (cmd) {
  case DFP_QUERY_TF_FILES:
    all_files_size = par;
    delay(20);
    dfPlayerSend(DFP_NUM_OF_FOLDERS, 0, 0);
    break;
  case DFP_NUM_OF_FOLDERS:
    num_folders = par;
    printInitialCardInfos();
    break;
  default:
    printUnknownCommand(cmd, par1, par2);
  }
}

void cardInserted(byte cmd, byte par1, byte par2) {
  state |= STATE_BIT_CARD_INSERTED;
  delay(500);
  dfPlayerSend(DFP_QUERY_TF_FILES, 0, 0);
}

// ===================================================================
// Callback for general status Messages
// ===================================================================
void commandReceived(byte cmd, byte par1, byte par2) {
  char ch[10];
  int par = ((int)par1) << 8 | ((int)par2);

  switch (cmd) {
  case DFP_INIT:
    state |= STATE_BIT_INITIALIZED;
    break;
  case DFP_PLUG_OUT:
    state &= ~STATE_BIT_CARD_INSERTED;
    break;
  default:
    printUnknownCommand(cmd, par1, par2);
  }
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

void printUnknownCommand(byte cmd, byte par1, byte par2) {
  char ch[10];
  Serial.print("Got Unknown Response from Device : 0x");
  printHex(cmd, 2);
  Serial.print(", 0x");
  printHex(par1, 2);
  Serial.print(", 0x");
  printHex(par2, 2);
  Serial.println("");
}

void printInitialCardInfos() {
  char ch[10];
  Serial.print("Card infos: Card has ");
  itoa(all_files_size, ch, 10);
  Serial.print(ch);
  Serial.print(" Files in ");
  itoa(num_folders, ch, 10);
  Serial.print(ch);
  Serial.println(" Folder(s).");

}
