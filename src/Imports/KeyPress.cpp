#include "ourLibrary.h"

//Read key key matrix columns
uint8_t readCols() {
    return (digitalRead(C3_PIN) << 3) | (digitalRead(C2_PIN) << 2) | (digitalRead(C1_PIN) << 1) | digitalRead(C0_PIN);
}

//Set row for key matrix
void setRow(uint8_t rowIdx) {
    //Disable row reading
    digitalWrite(REN_PIN, LOW);
    digitalWrite(RA0_PIN, rowIdx & 1);
    digitalWrite(RA1_PIN, rowIdx & 2);
    digitalWrite(RA2_PIN, rowIdx & 4);
    //Enable
    digitalWrite(REN_PIN, HIGH);
}

//Task for key scanning - period of 50ms
void scanKeysTask(void * pvParameters) {
    const TickType_t xFrequency = 50/portTICK_PERIOD_MS;
    TickType_t xLastWakeTime = xTaskGetTickCount();
    uint8_t currArray[8];
    uint8_t prevArray[8];
    //Determine what key was pressed
    uint8_t colsResult;
    uint8_t columnIndex;
    //Main loop
    while(1) {
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
        //Key scanning
        for(int i = 0; i < 8; i++) {
            //Activate row to read
            setRow(i);
            delayMicroseconds(3);
            //Read columns
            colsResult = ~readCols();
            currArray[i] = ~colsResult;
        }
        xSemaphoreTake(keyArrayMutex, portMAX_DELAY);
        std::copy(std::begin(keyArray), std::end(keyArray), std::begin(prevArray));
        xSemaphoreGive(keyArrayMutex);
        
        //stateChange(prevArray,currArray);
        //FOR VALIA/ANDREAS: prevArray IS THE PREVIOUS STATE. currArray IS THE NEW STATE.
        //prevArray = previous state
        //currArray = current state
        //Copy into keyArray - done at the end.
        xSemaphoreTake(keyArrayMutex, portMAX_DELAY);
        std::copy(std::begin(currArray), std::end(currArray), std::begin(keyArray));
        xSemaphoreGive(keyArrayMutex);
    }
}

//Allocate an accumulator to a key either the from CAN Bus or from itself
void allocAccumulator(uint8_t key, uint8_t octaveNum) {
    //Allocate freed accumulators to pressed keys
    uint16_t newKey = (octaveNum*12)+key;
    for(int i = 0; i < POLYPHONY; i++) {
        //If accumulator is free
        if(accumulatorMap[i] == NULL) {
            //Mark that key with the accumulator (fast lookup for deallocation)
            __atomic_store_n(&pianoKeyMap[newKey], i, __ATOMIC_RELAXED);
            //Write which key the previously free one is allocated to - stores octave information
            __atomic_store_n(&accumulatorMap[i], octaveNum, __ATOMIC_RELAXED);
            //Set the step size for the accumulator according to the key number
            __atomic_store_n(&currentStepSize[i], stepSizes[key], __ATOMIC_RELAXED);
            break;
        }
    }
}

//Deallocate an accumulator from CAN Bus or itself
void deallocAccumulator(uint8_t key, uint8_t octaveNum) {
  uint8_t newKey = (octaveNum*12)+key;
  //pianoKeyMap[newKey] gives the accumulator index mapped to newKey which ranges from 0-84
  __atomic_store_n(&accumulatorMap[pianoKeyMap[newKey]], NULL, __ATOMIC_RELAXED);
  __atomic_store_n(&currentStepSize[pianoKeyMap[newKey]], 0, __ATOMIC_RELAXED);
}