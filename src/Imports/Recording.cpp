#include "ourLibrary.h"
//Task for playback
void playbackTask(void * pvParameters) {
    const TickType_t xFrequency = 50/portTICK_PERIOD_MS; //Granuality in playback
    TickType_t xLastWakeTime = xTaskGetTickCount();
    while (1) {
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
        if (!ISPLAYBACK) {
            continue;
        }
        //Disable playback if the next key is disabled
        if (CURRENTKEY+1 >= LASTKEY) {
            __atomic_store_n(&ISPLAYBACK, false, __ATOMIC_RELAXED);
            //Wipe accumlators
            for(int i = 0; i < POLYPHONY; i++) {
                __atomic_store_n(&accumulatorMap[i], NULL, __ATOMIC_RELAXED);
                __atomic_store_n(&currentStepSize[i], 0, __ATOMIC_RELAXED); 
            }
            continue;
        }
        //Currently playing keys
        //If the next key is due to play
        if (keyMemory[CURRENTKEY].time > REFTIMER-millis()) {
            if (keyMemory[CURRENTKEY].eventType == 'P') {
                allocAccumulator(keyMemory[CURRENTKEY].key, keyMemory[CURRENTKEY].octave);
                __atomic_store_n(&CURRENTKEY, CURRENTKEY+1, __ATOMIC_RELAXED);
            } else if (keyMemory[CURRENTKEY].eventType == 'R') {
                deallocAccumulator(keyMemory[CURRENTKEY].key, keyMemory[CURRENTKEY].octave);
                __atomic_store_n(&CURRENTKEY, CURRENTKEY+1, __ATOMIC_RELAXED);
            }  
        }
    }
}
