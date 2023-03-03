#ifndef OURLIBRARY_H
#define OURLIBRARY_H

#include <string>
#include <Arduino.h>
//Constants and Global variables
inline const uint32_t interval = 100; //Display update interval
inline uint8_t keyArray[7];
inline const uint32_t stepSizes [] = {85899345, 90975216, 96246312, 102103086, 108155085, 114597536, 121430439, 128653793, 136267598, 144271855, 152861790, 162037402};

//Functions

void testPrint();

//Shaheen

uint8_t readCols();
void setRow(uint8_t rowIdx);
void scanKeysTask(void * pvParameters);
std::string readKeys();

//Shaanuka

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