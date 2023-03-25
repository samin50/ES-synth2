#include "ourLibrary.h"
//File handles all audio generation and the reallocation of accumulators for polyphony

void sampleISR() {
    static uint32_t phaseAcc[POLYPHONY];
    int32_t Vout = 0;
    int16_t octaveOffset;
    //If not master, do not play
    if (!ISMASTER) {
        return;
    }
    //Polyphonic keypresses
    for(int i = 0; i < POLYPHONY; i++) {
        //Do not process if accumulator is not set
        #ifndef TEST_MODE
            if (accumulatorMap[i] == NULL) {
                continue;
            }
        #endif
        //Obtain octave information from accumulatorMap
        octaveOffset = 4-(accumulatorMap[i]/12);
        if (octaveOffset > 0) {
            phaseAcc[i] += ((currentStepSize[i] >> octaveOffset)*JOYSTICKY)/8;
        } else {
            phaseAcc[i] += ((currentStepSize[i] << -octaveOffset)*JOYSTICKY)/8;
        }
        //Change sound profile
        switch (WAVETYPE) {
            case 0:
            {
            //Sawtooth
                Vout += ((phaseAcc[i] >> 24) - 128)/POLYPHONY;
                break;
            }
            case 1:
            {
            //Pulse wave
                if ((phaseAcc[i] >> 24) < 128) {
                    Vout -= 127/POLYPHONY;
                } else {
                    Vout += 127/POLYPHONY;
                }
                break;
            }
            case 2:
            {
            //Sine wave
                uint8_t sineIndx = (phaseAcc[i] >> 24) % 256;
                Vout += sinLUT[sineIndx]/POLYPHONY;
                break;
            }
            case 3: 
            {
            //Triangular wave
                if ((phaseAcc[i] >> 24) < 128) {
                    Vout += ((phaseAcc[i] >> 24)) / POLYPHONY;
                } else {
                    Vout += (255 - ((phaseAcc[i] >> 24))) / POLYPHONY;
                }
                break;
            }
        }
    }
    //Volume control here
    analogWrite(OUTR_PIN, ((Vout + 128)*(VOLUMEMOD+JOYSTICKX))/64);
}

//Allocate an accumulator to a key either the from CAN Bus or from itself
void allocAccumulator(uint8_t key, uint8_t octaveNum) {
    //Allocate freed accumulators to pressed keys
    uint16_t newKey = (octaveNum*12)+key;
    //Do not allow multiple identical keys into the accumulator else a key release event will cause a stuck key
    if (pianoKeyMap[newKey] != NULL) {
        return;
    }
    //Check for free accumulators
    for(int i = 0; i < POLYPHONY; i++) {
        //If accumulator is free
        if(accumulatorMap[i] == NULL) {
            //Mark that key with the accumulator (fast lookup for deallocation)
            __atomic_store_n(&pianoKeyMap[newKey], i, __ATOMIC_RELAXED);
            //Write which key the previously free one is allocated to - stores octave information
            __atomic_store_n(&accumulatorMap[i], newKey, __ATOMIC_RELAXED);
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
                    //Disable recording at record limit
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
    __atomic_store_n(&pianoKeyMap[newKey], NULL, __ATOMIC_RELAXED); 
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