#include "ourLibrary.h"
//File handles all key inputs

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
        //Key scanning
        for(int i = 0; i < 8; i++) {
            //Activate row to read
            setRow(i);
            delayMicroseconds(3);
            //Read columns
            colsResult = ~readCols();
            currArray[i] = ~colsResult;
        }
        //Copy the previous key array to detect changes in state
        xSemaphoreTake(keyArrayMutex, portMAX_DELAY);
        std::copy(std::begin(keyArray), std::end(keyArray), std::begin(prevArray));
        xSemaphoreGive(keyArrayMutex);
        #ifdef TEST_MODE
            //Worst case scenario
            for(int i = 0; i < 8; i++) {
                prevArray[i] = u_int8_t(255);
            }
        #endif
        //Detect changes in key presses
        updateButtons(prevArray, currArray);
        stateChange(prevArray,currArray);
        readJoystick();
        //Copy into key array the new state
        xSemaphoreTake(keyArrayMutex, portMAX_DELAY);
        std::copy(std::begin(currArray), std::end(currArray), std::begin(keyArray));
        xSemaphoreGive(keyArrayMutex);
        //Timing analysis
        #ifdef TEST_MODE
            return;
        #endif
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
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
        __atomic_store_n(&VOLUMEMOD, max(0, min(64, VOLUMEMOD+(K0*8))), __ATOMIC_RELAXED);
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
        if(K3 == -1) {
            __atomic_store_n(&SCREENNUM, 0, __ATOMIC_RELAXED); 
            __atomic_store_n(&LASTKEY, CURRENTKEY, __ATOMIC_RELAXED);
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

//Allocates and deallocates accumulators based off of the change in keypresses
void stateChange(uint8_t prevKeys[], uint8_t currKeys[]){
	//Disable registering keypresses during playback
	if (ISPLAYBACK) {
		return;
	}
	uint8_t prevKeyRow;
	uint8_t currKeyRow;
	uint8_t columnIndex = 0;
	uint8_t keyNum = 0;
	uint8_t TX_Message[8] = {0};
	for (int i = 0; i < 3; i++){
		prevKeyRow = prevKeys[i];
		currKeyRow = currKeys[i];
		columnIndex = 0;
		keyNum = 0;
		for (int columnIndex = 0; columnIndex < 4; columnIndex++) {
			keyNum = (i*4)+columnIndex;
			if (((prevKeyRow&1)^(currKeyRow&1))==1) {
				//State change in keys
				if ((prevKeyRow&1) == 0) { 
					//key Released 	
					TX_Message[0] = 'R';
					//If master, dealloc accumulator directly 
					if (ISMASTER) {
						deallocAccumulator(keyNum, OCTAVE);
					}
				} else {			   
					//key Pressed	
					TX_Message[0] = 'P';
					//If master, alloc accumulator directly
					if (ISMASTER) {
						allocAccumulator(keyNum, OCTAVE);
					}
				}
				TX_Message[1] = OCTAVE;
				TX_Message[2] = keyNum;
				//If not master, send to master
				if (!ISMASTER) {
					xQueueSend(msgOutQ, TX_Message, portMAX_DELAY);
				}
			}
			currKeyRow = currKeyRow >> 1;
			prevKeyRow = prevKeyRow >> 1;
		}
	}
}

//Read and joystick value and store for pitch bending
void readJoystick() {
    //Volume mod
    int32_t joyXRead = (544-int(analogRead(JOYX_PIN)))/32; 
    __atomic_store_n(&JOYSTICKX, joyXRead, __ATOMIC_RELAXED);
    //Pitch bend
    //Offset of 64 to avoid scaling volume to 0
    int32_t joyYRead = max(1, int((1087-analogRead(JOYY_PIN))/64)); 
    __atomic_store_n(&JOYSTICKY, joyYRead, __ATOMIC_RELAXED);
}
