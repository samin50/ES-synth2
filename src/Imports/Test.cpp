#include "ourLibrary.h"

void testPrint() {
    Serial.println("TEST!");
}

// //Task for key scanning - period of 50ms
// void scanKeysTask(void * pvParameters) {
//     const TickType_t xFrequency = 50/portTICK_PERIOD_MS;
//     TickType_t xLastWakeTime = xTaskGetTickCount();
//     uint8_t currArray[8];
//     uint8_t prevArray[8];
//     //Determine what key was pressed
//     uint8_t colsResult;
//     uint8_t columnIndex;
//     uint8_t keyNum = 0;
//     uint8_t polyCounter = 0;
//     //Determine which keys were changed
//     uint8_t pianoKeyMap[12] = {0};
//     //Accumulator for polyphony
//     uint8_t accumulatorMap[POLYPHONY];
//     uint8_t freeAccumulator = -1;
//     //Initialise map
//     for(int i = 0; i < POLYPHONY; i++) {
//         accumulatorMap[i] = NULL;
//     }
//     //Main loop
//     while(1) {
//         polyCounter = 0;
//         vTaskDelayUntil(&xLastWakeTime, xFrequency);
//         //Key scanning
//         for(int i = 0; i < 8; i++) {
//             //Activate row to read
//             setRow(i);
//             delayMicroseconds(3);
//             //Read columns
//             colsResult = ~readCols();
//             currArray[i] = ~colsResult;
            
//             //For the piano keys (i < 3)
//             if (i < 3) {
//                 columnIndex = 0;
//                 //Count the number of keys pressed in the current row
//                 while(colsResult) {
//                     keyNum = (i*4)+columnIndex;
//                     //Active low
//                     if (colsResult & 1) {
//                         polyCounter++; //Total piano key presses
//                         //Set the key to being pressed
//                         if (pianoKeyMap[keyNum] == 2) {
//                             //If already assigned to accumulator previously
//                             pianoKeyMap[keyNum] = 2;
//                         } else {
//                             //Is recently pressed
//                             pianoKeyMap[keyNum] = 1;
//                         }
//                     } else {
//                         //Is not pressed
//                         pianoKeyMap[keyNum] = 0;
//                     }
//                     colsResult = colsResult >> 1;
//                     columnIndex += 1;
//                 }
//             }
//         }
//         //Piano key map has three states: 0 means not pressed, 1 means looking for accumulator, 2 means already assigned
//         //Polyphony and the accumulators
//         for(int i = 0; i < POLYPHONY; i++) {
//             //Free up accumulators if key no longer pressed
//             if (pianoKeyMap[accumulatorMap[i]] == 0) {
//                 accumulatorMap[i] = NULL;
//                 __atomic_store_n(&currentStepSize[i], 0, __ATOMIC_RELAXED);
//             }
//         }
//         //Allocate freed accumulators to pressed keys
//         for(int i = 0; i < POLYPHONY; i++) {
//             //If accumulator is free
//             if(accumulatorMap[i] == NULL) {
//                 //Find a key to allocate it to
//                 for(int j = 0; j < 12; j++) {
//                     if(pianoKeyMap[j] == 1) {
//                         //Mark that key as having an accumulator
//                         pianoKeyMap[j] = 2;
//                         //Write which key the previously free one is allocated to
//                         accumulatorMap[i] = j;
//                         //Set the step size for the accumulator according to the key pitch/
//                         __atomic_store_n(&currentStepSize[i], stepSizes[j], __ATOMIC_RELAXED);
//                         break;
//                     }
//                 }
//             }
//         }
//         xSemaphoreTake(keyArrayMutex, portMAX_DELAY);
//         std::copy(std::begin(keyArray), std::end(keyArray), std::begin(prevArray));
//         xSemaphoreGive(keyArrayMutex);
        
//         stateChange(prevArray,currArray);
//         //FOR VALIA/ANDREAS: prevArray IS THE PREVIOUS STATE. currArray IS THE NEW STATE.
//         //prevArray = previous state
//         //currArray = current state
//         //Copy into keyArray - done at the end.
//         xSemaphoreTake(keyArrayMutex, portMAX_DELAY);
//         std::copy(std::begin(currArray), std::end(currArray), std::begin(keyArray));
//         xSemaphoreGive(keyArrayMutex);
//     }
// }