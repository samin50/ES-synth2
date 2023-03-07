#include "ourLibrary.h"

//Task for display updating - period 100ms
void displayUpdateTask(void * pvParameters) {
    const TickType_t xFrequency = 100/portTICK_PERIOD_MS;
    TickType_t xLastWakeTime = xTaskGetTickCount();
    while(1) {
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
        //Display button press
        uint32_t res = 0;
        uint8_t tempArray[8];
        //Copy from keyArray
        xSemaphoreTake(keyArrayMutex, portMAX_DELAY);
        std::copy(std::begin(keyArray), std::end(keyArray), std::begin(tempArray));
        xSemaphoreGive(keyArrayMutex);
        //Obtain hex code for keypresses
        for (int i = 0; i < 8; i++) {
            res = res | (tempArray[i] << i*4);
        }
        //Strings
        char keyBuff[9];
        sprintf(keyBuff, "%08X", ~res & 0xFFFFFFFF);
        std::string volumeStr = "Vol: " + std::to_string(VOLUMEMOD);
        std::string masterStr = (ISMASTER == true) ? "M:T" : "M:F";
        std::string octaveStr = "Oct: " + std::to_string(OCTAVE);
        std::string waveStr = "W: " + std::to_string(WAVETYPE);
        //Update display
        u8g2.clearBuffer();         // clear the internal memory
        u8g2.setFont(u8g2_font_5x7_tr); // choose a suitable font
        u8g2.drawStr(2,8,keyBuff);
        u8g2.drawStr(2,16,volumeStr.c_str());
        u8g2.drawStr(2,24,masterStr.c_str());
        u8g2.drawStr(40,24,waveStr.c_str());
        u8g2.drawStr(2,32,octaveStr.c_str());
        u8g2.sendBuffer();          // transfer internal memory to the display
    }
}

void updateButtons(uint8_t prevKeys[], uint8_t currKeys[]) {
    //Volume key is first knob
    uint8_t updateVolume = max(0, min(8, VOLUMEMOD+rotationDirection((prevKeys[4]>>2)&0b11, (currKeys[4]>>2)&0b11)));
    __atomic_store_n(&VOLUMEMOD, updateVolume, __ATOMIC_RELAXED);
    //Octave key is second knob
    uint8_t updateOctave = max(0, min(6, OCTAVE+rotationDirection(prevKeys[4]&0b11, currKeys[4]&0b11)));
    __atomic_store_n(&OCTAVE, updateOctave, __ATOMIC_RELAXED);
    //Wave key is third knob
    uint8_t updateWave = max(0, min(0, WAVETYPE+rotationDirection((prevKeys[3]>>2)&0b11, (currKeys[3]>>2)&0b11)));
    __atomic_store_n(&WAVETYPE, updateWave, __ATOMIC_RELAXED);
    //Master control is fourth knob
    int8_t updateMaster = rotationDirection(prevKeys[3]&0b11, currKeys[3]&0b11);
    if(updateMaster == 1) {
        __atomic_store_n(&ISMASTER, true, __ATOMIC_RELAXED);
    }
    if (updateMaster == -1) {
        __atomic_store_n(&ISMASTER, false, __ATOMIC_RELAXED);
    }
}

int8_t rotationDirection(uint8_t prevState, uint8_t currState) {
    //Implement state transition
    if ((prevState == 0 && currState == 1) || (prevState == 3 && currState == 2)) {
        return 1;
    } else if ((prevState == 0 && currState == 2) || (prevState == 2 && currState == 3)) {
        return -1;
    }
    return 0;
}