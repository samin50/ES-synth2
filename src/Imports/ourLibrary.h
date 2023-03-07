#ifndef OURLIBRARY_H
#define OURLIBRARY_H

#include <string>
#include <Arduino.h>
#include <STM32FreeRTOS.h>
#include <U8g2lib.h>
#include <ES_CAN.h>

//Constants and Global variables
//Settings
inline volatile uint8_t WAVETYPE = 0; //0 is sawtooth
inline const uint8_t POLYPHONY = 8; //How many simulataneous keys allowed
inline const uint32_t INTERVAL = 100; //Display update interval
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
inline const uint32_t stepSizes [] = {85899345, 90975216, 96246312, 102103086, 108155085, 114597536, 121430439, 128653793, 136267598, 144271855, 152861790, 162037402};
inline volatile uint32_t currentStepSize[POLYPHONY];
inline volatile uint8_t accumulatorMap[POLYPHONY]; //Accumulator map - contains information mapping accumulators and key presses to allow polyphony
inline volatile uint8_t pianoKeyMap[84]; //Keeps track of which key is allocated to what accumulator - 7 octaves support so 84 keys total
//Buttons
inline volatile uint8_t VOLUMEMOD = 5;
inline volatile uint8_t OCTAVE = 4; //Octave number
inline volatile uint8_t MASTER_ID = 100;
inline volatile uint8_t ISMASTER = true; //Is the master (is responsible for playing keys?)


//Shaheen

uint8_t readCols();
void setRow(uint8_t rowIdx);
void sampleISR();
void allocAccumulator(uint8_t key, uint8_t octaveNum);
void deallocAccumulator(uint8_t key, uint8_t octaveNum);
void scanKeysTask(void * pvParameters);
void displayUpdateTask(void * pvParameters);
void updateButtons(uint8_t prevKeys[], uint8_t currKeys[]);
int8_t rotationDirection(uint8_t prevState, uint8_t currState);

//Shaanuka
void CAN_RX_ISR();
void decodeTask(void *pvParamters);
void sendMessage(uint32_t id, uint8_t* data, uint8_t length);
void sendCurrKeys();
void CANSend(void * pvParameters);
void CAN_TX_ISR();
void stateChange(uint8_t prevKeys[], uint8_t currKeys[]);
//Valia

//Andreas

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