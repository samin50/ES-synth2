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
        updateButtons(prevArray, currArray);
        stateChange(prevArray,currArray);
        readJoystick();
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
            //If recording - store as successful keypress, no need for atomic access as this is only thread with access to keymemory, and alloc dealloc does not occur at the same time
            if (ISRECORDING) {
                 __atomic_store_n(&LASTKEY, CURRENTKEY+1, __ATOMIC_RELAXED);
                if (CURRENTKEY+1 < MAXKEYS) {
                    keyMemory[CURRENTKEY].eventType = 'P';
                    keyMemory[CURRENTKEY].octave = octaveNum;
                    keyMemory[CURRENTKEY].key = key;
                    keyMemory[CURRENTKEY].time = REFTIMER-millis();
                    //Increment current key
                    __atomic_store_n(&CURRENTKEY, CURRENTKEY+1, __ATOMIC_RELAXED);
                } else {
                    //Disable reccording at record limit
                    __atomic_store_n(&ISRECORDING, false, __ATOMIC_RELAXED);
                }
            }
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
    if (ISRECORDING) {
         __atomic_store_n(&LASTKEY, CURRENTKEY+1, __ATOMIC_RELAXED);
        if (CURRENTKEY+1 < MAXKEYS) {
            keyMemory[CURRENTKEY].eventType = 'R';
            keyMemory[CURRENTKEY].octave = octaveNum;
            keyMemory[CURRENTKEY].key = key;
            keyMemory[CURRENTKEY].time = REFTIMER-millis();
            //Increment current key
            __atomic_store_n(&CURRENTKEY, CURRENTKEY+1, __ATOMIC_RELAXED);
        } else {
            //Disable recording at record limit
            __atomic_store_n(&ISRECORDING, false, __ATOMIC_RELAXED);

        }
    }
}

//Update settings and change screens
void updateButtons(uint8_t prevKeys[], uint8_t currKeys[]) {
    int8_t K0 = rotationDirection((prevKeys[4]>>2)&0b11, (currKeys[4]>>2)&0b11);
    int8_t K1 = rotationDirection(prevKeys[4]&0b11, currKeys[4]&0b11);
    int8_t K2 = rotationDirection((prevKeys[3]>>2)&0b11, (currKeys[3]>>2)&0b11);
    int8_t K3 = rotationDirection(prevKeys[3]&0b11, currKeys[3]&0b11);
    if (SCREENNUM == 0) { //Main screen
        //Volume key is first knob
        __atomic_store_n(&VOLUMEMOD, max(0, min(8, VOLUMEMOD+K0)), __ATOMIC_RELAXED);
        //Octave key is second knob
        __atomic_store_n(&OCTAVE,  max(0, min(6, OCTAVE+K1)), __ATOMIC_RELAXED);
        //Wave key is third knob
        __atomic_store_n(&WAVETYPE, max(0, min(3, WAVETYPE+K2)), __ATOMIC_RELAXED);
        //Navigate to settings page
        if(K3 == 1) {
            __atomic_store_n(&SCREENNUM, 1, __ATOMIC_RELAXED);
        }
        if (K3 == -1) {
            __atomic_store_n(&SCREENNUM, 0, __ATOMIC_RELAXED);
        }
    } else if (SCREENNUM == 1) { //Settings screen
        //Master control is first knob
        if(K0 == 1) {
            __atomic_store_n(&ISMASTER, true, __ATOMIC_RELAXED);
        }
        if(K0 == -1) {
            __atomic_store_n(&ISMASTER, false, __ATOMIC_RELAXED);
        }
        //Record screen is second knob
         if(K1 == 1) {
            //Begin record
            __atomic_store_n(&SCREENNUM, 2, __ATOMIC_RELAXED);
            __atomic_store_n(&CURRENTKEY, 0, __ATOMIC_RELAXED);
            __atomic_store_n(&REFTIMER, millis(), __ATOMIC_RELAXED);
            __atomic_store_n(&ISRECORDING, true, __ATOMIC_RELAXED);
        }
        //Playback screen is third knob
         if(K2 == 1) {
            //Start playback
            __atomic_store_n(&CURRENTKEY, 0, __ATOMIC_RELAXED);
            __atomic_store_n(&SCREENNUM, 3, __ATOMIC_RELAXED);
            __atomic_store_n(&REFTIMER, millis(), __ATOMIC_RELAXED);
            __atomic_store_n(&ISPLAYBACK, true, __ATOMIC_RELAXED);
        }
        //Return
        if(K3 == -1) {
            __atomic_store_n(&SCREENNUM, 0, __ATOMIC_RELAXED);
        }
    } else if (SCREENNUM == 2) { //Record screen
         //Return
        if(K3 == -1) {
            __atomic_store_n(&SCREENNUM, 0, __ATOMIC_RELAXED);
            __atomic_store_n(&ISRECORDING, false, __ATOMIC_RELAXED);
        }
    } else if (SCREENNUM == 3) { //Playback screen
        //Return
        if(K3 == -1 && !ISPLAYBACK) {
            __atomic_store_n(&SCREENNUM, 0, __ATOMIC_RELAXED);
            //__atomic_store_n(&ISPLAYBACK, false, __ATOMIC_RELAXED);
        }
    }
}

//Knob rotation direction
int8_t rotationDirection(uint8_t prevState, uint8_t currState) {
    //Implement state transition
    if ((prevState == 0 && currState == 1) || (prevState == 3 && currState == 2)) {
        return 1;
    } else if ((prevState == 0 && currState == 2) || (prevState == 2 && currState == 3)) {
        return -1;
    }
    return 0;
}

//Read and joystick value and store for pitch bending
void readJoystick() {
    uint32_t joyXRead = max(1, int((1023-analogRead(JOYX_PIN))/8));
    __atomic_store_n(&JOYSTICKX, joyXRead, __ATOMIC_RELAXED);
    //Pitch bend
    uint32_t joyYRead = max(1, int((1087-analogRead(JOYY_PIN))/64)); //Offset of 64 to avoid scaling volume to 0
    __atomic_store_n(&JOYSTICKY, joyYRead, __ATOMIC_RELAXED);
}