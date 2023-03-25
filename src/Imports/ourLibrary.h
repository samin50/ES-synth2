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

//Timing analysis
#define TEST_MODE
//#define STAT_ONLY
#define TEST_ITERATIONS 32
//Constants and Global variables
//Settings
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
inline volatile const int32_t sinLUT[256] = {
0, 3, 6, 9, 12, 15, 18, 21, 24, 27, 30, 33, 
36, 39, 42, 44, 47, 50, 53, 56, 58, 61, 64, 
66, 69, 71, 74, 76, 79, 81, 84, 86, 88, 90, 
92, 94, 96, 98, 100, 102, 103, 105, 107, 109, 110, 
111, 112, 114, 115, 116, 117, 118, 119, 119, 120, 120, 
120, 120, 120, 120, 120, 119, 119, 118, 117, 116, 115, 
114, 112, 111, 109, 107, 105, 103, 100, 98, 96, 94, 
91, 88, 86, 83, 80, 76, 73, 69, 65, 61, 57, 
53, 49, 44, 40, 35, 31, 26, 21, 17, 12, 7, 3, 
-3, -7, -12, -17, -21, -26, -31, -35, -40, -44, -49, 
-53, -57, -61, -65, -69, -73, -76, -80, -83, -86, -88, 
-91, -94, -96, -98, -100, -102, -103, -105, -107, -109, -110, 
-111, -112, -114, -115, -116, -117, -118, -119, -119, -120, -120, 
-120, -120, -120, -120, -120, -119, -119, -118, -117, -116, -115, 
-114, -112, -111, -109, -107, -105, -103, -100, -98, -96, -94, 
-91, -88, -86, -83, -80, -76, -73, -69, -65, -61, -57, -53, 
-49, -44, -40, -35, -31, -26, -21, -17, -12, -7, -3};

inline const uint32_t stepSizes[12] = {50953930, 54077542, 57201155, 60715219, 64229283, 68133799, 72233540, 76528508, 81018701, 85899345, 90975216, 96246312};
inline volatile uint32_t currentStepSize[POLYPHONY];
inline volatile uint8_t accumulatorMap[POLYPHONY]; //Accumulator map - contains information mapping accumulators and key presses to allow polyphony
inline volatile uint8_t pianoKeyMap[84]; //Keeps track of which key is allocated to what accumulator - 7 octaves support so 84 keys total
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
inline volatile uint32_t REFTIMER = 0;
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
void timingAnalysis();

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
