#ifndef OURLIBRARY_H
#define OURLIBRARY_H

#include <string>
#include <Arduino.h>
#include <STM32FreeRTOS.h>
#include <U8g2lib.h>
#include <ES_CAN.h>

//For note display
#include <sstream>
#include <iomanip>

//Constants and Global variables
//Settings
inline volatile uint32_t dur, start1, end1; // for time analysis
inline volatile const int32_t sinLUT[256] = {
    0, 6, 12, 18, 24, 30, 36, 42, 48, 54, 60, 66, 
72, 78, 84, 89, 95, 101, 106, 112, 117, 123, 128, 
133, 138, 143, 148, 153, 158, 163, 167, 172, 176, 
181, 185, 189, 193, 197, 200, 204, 207, 211, 214, 
217, 220, 223, 225, 228, 230, 233, 235, 237, 239, 
240, 242, 243, 245, 246, 247, 248, 248, 249, 249, 
249, 249, 249, 249, 249, 248, 248, 247, 246, 245, 
243, 242, 240, 239, 237, 235, 233, 230, 228, 226, 
223, 220, 217, 214, 211, 207, 204, 200, 197, 193, 
189, 185, 181, 176, 172, 167, 163, 158, 153, 148, 
143, 138, 133, 128, 123, 117, 112, 106, 101, 95, 
89, 84, 78, 72, 66, 60, 54, 48, 42, 36, 30, 24, 
18, 12, 6, 0, -6, -12, -18, -24, -30, -36, -42, 
-48, -54, -60, -66, -72, -78, -84, -89, -95, -101, 
-106, -112, -117, -123, -128, -133, -138, -143, -148, 
-153, -158, -163, -167, -172, -176, -181, -185, -189, 
-193, -197, -200, -204, -207, -211, -214, -217, -220, 
-223, -225, -228, -230, -233, -235, -237, -239, -240, 
-242, -243, -245, -246, -247, -248, -248, -249, -249, 
-249, -249, -249, -249, -249, -248, -248, -247, -246, 
-245, -243, -242, -240, -239, -237, -235, -233, -230, 
-228, -226, -223, -220, -217, -214, -211, -207, -204, 
-200, -197, -193, -189, -185, -181, -176, -172, -167, 
-163, -158, -153, -148, -143, -138, -133, -128, -123, 
-117, -112, -106, -101, -95, -90, -84, -78, -72, -66, 
-60, -54, -48, -42, -36, -30, -24, -18, -12, -6};
inline volatile int8_t WAVETYPE; //0 is sawtooth, 1 is pulse, 2 is sine, 3 is triangular
inline const uint8_t POLYPHONY = 8; //How many simulataneous keys allowed
inline const uint32_t INTERVAL = 100; //LED update interval
//Display
inline U8G2_SSD1305_128X32_NONAME_F_HW_I2C u8g2(U8G2_R0);
inline std::string keyInfo;
//Key scan variables
inline uint8_t keyArray[7];
inline SemaphoreHandle_t keyArrayMutex;
//CAN variables
inline QueueHandle_t msgInQ;
inline QueueHandle_t msgOutQ;
inline SemaphoreHandle_t CAN_TX_Semaphore;
//Polyphony and audio settings
inline const uint32_t stepSizes [] = {50953930, 54077542, 57201155, 60715219, 64229283, 68133799, 72233540, 76528508, 81018701, 85899345, 90975216, 96246312};
inline volatile uint32_t currentStepSize[POLYPHONY];
inline volatile uint8_t accumulatorMap[POLYPHONY]; //Accumulator map - contains information mapping accumulators and key presses to allow polyphony
inline volatile uint8_t pianoKeyMap[84]; //Keeps track of which key is allocated to what accumulator - 7 octaves support so 84 keys total
inline SemaphoreHandle_t accumulatorMapMutex;
//Buttons
inline volatile int8_t VOLUMEMOD = 64;
inline volatile int8_t OCTAVE = 5; //Octave number
inline volatile uint8_t MASTER_ID = 100;
inline volatile bool ISMASTER = true; //Is the master (is responsible for playing keys?)
//Joystick
inline volatile int32_t JOYSTICKX;
inline volatile int32_t JOYSTICKY;
//Storage and recording
struct keyRecord {
    char eventType = 'R';
    uint8_t octave = 0;
    uint8_t key = 0;
    uint32_t time = 0;
};
inline volatile uint8_t SCREENNUM = 0; 
inline volatile bool ISRECORDING = false;
inline volatile bool ISPLAYBACK = false;
inline volatile uint32_t REFTIMER;
inline const uint16_t MAXKEYS = 512;
inline volatile keyRecord keyMemory[MAXKEYS];
inline volatile uint16_t CURRENTKEY = 0;
inline volatile uint16_t LASTKEY = 0;

//Shaheen

uint8_t readCols();
void setRow(uint8_t rowIdx);
void allocAccumulator(uint8_t key, uint8_t octaveNum);
void deallocAccumulator(uint8_t key, uint8_t octaveNum);
void scanKeysTask(void * pvParameters);
void displayUpdateTask(void * pvParameters);
void playbackTask(void * pvParameters);
void updateButtons(uint8_t prevKeys[], uint8_t currKeys[]);
int8_t rotationDirection(uint8_t prevState, uint8_t currState);
void printTime();
void mainScreen();
void settingsScreen();
void recordScreen();
void playScreen();
void readJoystick();

//Shaanuka
void CAN_RX_ISR();
void decodeTask(void *pvParamters);
void sendMessage(uint32_t id, uint8_t* data, uint8_t length);
void sendCurrKeys();
void CANSend(void * pvParameters);
void CAN_TX_ISR();
void stateChange(uint8_t prevKeys[], uint8_t currKeys[]);

//Valia
void printKey();
std::string hexToBin(uint16_t hexVal);

//Andreas
void sampleISR();

//Pin definitions
//Row select and enable
const int RA0_PIN = D3;
const int RA1_PIN = D6;
const int RA2_PIN = D12;
const int REN_PIN = A5;

//Matrix input and output
const int C0_PIN = A2;
const int C1_PIN = D9;
const int C2_PIN = A6;
const int C3_PIN = D1;
const int OUT_PIN = D11;

//Audio analogue out
const int OUTL_PIN = A4;
const int OUTR_PIN = A3;

//Joystick analogue in
const int JOYY_PIN = A0;
const int JOYX_PIN = A1;

//Output multiplexer bits
const int DEN_BIT = 3;
const int DRST_BIT = 4;
const int HKOW_BIT = 5;
const int HKOE_BIT = 6;

#endif
