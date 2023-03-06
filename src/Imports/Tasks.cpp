#include "ourLibrary.h"
#include <unordered_map>

//Task for key scanning - period of 50ms
void scanKeysTask(void * pvParameters) {
    const TickType_t xFrequency = 50/portTICK_PERIOD_MS;
    TickType_t xLastWakeTime = xTaskGetTickCount();
    uint8_t currArray[7];
    uint8_t prevArray[7];
    //Determine what key was pressed
    uint8_t colsResult;
    uint8_t columnIndex;
    uint8_t keyNum = 0;
    uint8_t polyCounter = 0;
    //Determine which keys were changed
    uint8_t pianoKeyMap[12] = {0};
    //Accumulator for polyphony
    uint8_t accumulatorMap[polyphony];
    uint8_t freeAccumulator = -1;
    //Initialise map
    for(int i = 0; i < polyphony; i++) {
        accumulatorMap[i] = NULL;
    }
    //Main loop
    while(1) {
        polyCounter = 0;
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
        //Key scanning
        for(int i = 0; i < 7; i++) {
            //Activate row to read
            setRow(i);
            delayMicroseconds(3);
            //Read columns
            colsResult = ~readCols();
            currArray[i] = ~colsResult;
            //For the piano keys (i < 3)
            if (i < 3) {
                columnIndex = 0;
                //Count the number of keys pressed in the current row
                while(colsResult) {
                    keyNum = (i*4)+columnIndex;
                    //Active low
                    if (colsResult & 1) {
                        polyCounter++; //Total piano key presses
                        //Set the key to being pressed
                        if (pianoKeyMap[keyNum] == 2) {
                            //If already assigned to accumulator previously
                            pianoKeyMap[keyNum] = 2;
                        } else {
                            //Is recently pressed
                            pianoKeyMap[keyNum] = 1;
                        }
                    } else {
                        //Is not pressed
                        pianoKeyMap[keyNum] = 0;
                    }
                    colsResult = colsResult >> 1;
                    columnIndex += 1;
                }
            }
        }
        //Piano key map has three states: 0 means not pressed, 1 means looking for accumulator, 2 means already assigned
        //Polyphony and the accumulators
        for(int i = 0; i < polyphony; i++) {
            //Free up accumulators if key no longer pressed
            if (pianoKeyMap[accumulatorMap[i]] == 0) {
                accumulatorMap[i] = NULL;
                __atomic_store_n(&currentStepSize[i], 0, __ATOMIC_RELAXED);
            }
        }
        //Allocate freed accumulators to pressed keys
        for(int i = 0; i < polyphony; i++) {
            //If accumulator is free
            if(accumulatorMap[i] == NULL) {
                //Find a key to allocate it to
                for(int j = 0; j < 12; j++) {
                    if(pianoKeyMap[j] == 1) {
                        //Mark that key as having an accumulator
                        pianoKeyMap[j] = 2;
                        //Write which key the previously free one is allocated to
                        accumulatorMap[i] = j;
                        //Set the step size for the accumulator according to the key pitch/
                        __atomic_store_n(&currentStepSize[i], stepSizes[j], __ATOMIC_RELAXED);
                        break;
                    }
                }
            }
        }
        xSemaphoreTake(keyArrayMutex, portMAX_DELAY);
        std::copy(std::begin(keyArray), std::end(keyArray), std::begin(prevArray));
        xSemaphoreGive(keyArrayMutex);

        sendCurrKeys(currArray);
        //FOR VALIA/ANDREAS: prevArray IS THE PREVIOUS STATE. currArray IS THE NEW STATE.
        //prevArray = previous state
        //currArray = current state
        //Copy into keyArray - done at the end.
        xSemaphoreTake(keyArrayMutex, portMAX_DELAY);
        std::copy(std::begin(currArray), std::end(currArray), std::begin(keyArray));
        xSemaphoreGive(keyArrayMutex);
    }
}

//Task for display updating - period 100ms
void displayUpdateTask(void * pvParameters) {
    const TickType_t xFrequency = 100/portTICK_PERIOD_MS;
    TickType_t xLastWakeTime = xTaskGetTickCount();
    //uint8_t RX_Message[8];
    //std::copy(std::begin(TX_Message), std::end(TX_Message), std::begin(RX_Message));
    ; //received msg
    while(1) {
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
        //Display button press
        uint32_t res = 0;
        uint8_t tempArray[7];
        //Copy from keyArray
        xSemaphoreTake(keyArrayMutex, portMAX_DELAY);
        std::copy(std::begin(keyArray), std::end(keyArray), std::begin(tempArray));
        xSemaphoreGive(keyArrayMutex);
        //Obtain hex code for keypresses
        for (int i = 0; i < 7; i++) {
            res = res | (tempArray[i] << i*4);
            //Serial.println(tempArray[i]);
        }

        // uint8_t tempMsg[8]; //copy of msg to be transmitted

        // uint32_t ID = 0x123;
        // std::copy(std::begin(TX_Message), std::end(TX_Message), std::begin(tempMsg));
        // CAN_TX(0x456,tempMsg);//tempMsg);
        // while (CAN_CheckRXLevel())
	    //     CAN_RX(ID, RX_Message);
        // Serial.println(RX_Message[0]);
        
        char buf[8];
        sprintf(buf, "%07X", ~res & 0x0FFFFFFF);
        std::string result(buf);
        //Update display
        u8g2.clearBuffer();         // clear the internal memory
        u8g2.setFont(u8g2_font_ncenB08_tr); // choose a suitable font
        u8g2.drawStr(2,10,"Hello World!");  // write something to the internal memory
        u8g2.setCursor(2,20);
        u8g2.print(result.c_str());
        u8g2.sendBuffer();          // transfer internal memory to the display
    }
}