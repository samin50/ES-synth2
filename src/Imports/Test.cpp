#include "ourLibrary.h"

void testPrint() {
    Serial.println("TEST!");
}

//Task for key scanning - period of 50ms
void scanKeysTaskTemp(void * pvParameters) {
    const TickType_t xFrequency = 50/portTICK_PERIOD_MS;
    TickType_t xLastWakeTime = xTaskGetTickCount();
    uint8_t tempArray[7];
    uint8_t colsResult;
    uint8_t polyCounter = 0;
    uint8_t colIndex = 0;
    //Keeps track of which key is mapped to what accumulator
    uint8_t keyMap[12] = {0};
    uint8_t accumulatorMap[polyphony];
    uint8_t freeAccumulator = -1;
    //Initialise map
    for(int i = 0; i < polyphony; i++) {
        accumulatorMap[i] = NULL;
    }
    while(1) {
        polyCounter = 0;
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
        for(int i = 0; i < 7; i++) {
            setRow(i);
            delayMicroseconds(3);
            colsResult = readCols();
            tempArray[i] = colsResult;
            //Set step sizes for each accumulator
            if ((i < 3) && (polyCounter < polyphony)) {
                colIndex = 0;
                //Count the number of 1s pressed in the row
                while(colsResult) {
                    freeAccumulator = -1;
                    //Key is pressed
                    if (colsResult & 1) {
                        //Find a free accumulator
                        for(int j = 0; j < polyphony; j++) {
                            if (accumulatorMap[j] != NULL) {
                                freeAccumulator = j; 
                            }
                        }
                        //If an accumulator was found
                        if (freeAccumulator != -1) {
                            keyMap[(i*4)+colIndex] = 1;
                        }
                        //Gives the key number, 1-12
                        //keyMap[(i*4)+colIndex] = 1;
                       
                        polyCounter++;
                        //Set currentstepsize
                    }
                    colsResult = colsResult >> 1;
                    colIndex += 1;
                    //Free up any accumulators that are not being used
                }
            }
        }
        //Copy into keyArray
        xSemaphoreTake(keyArrayMutex, portMAX_DELAY);
        std::copy(std::begin(tempArray), std::end(tempArray), std::begin(keyArray));
        xSemaphoreGive(keyArrayMutex);
    }
}