#include "ourLibrary.h"

std::string hexToBin(uint16_t hexVal) {
    std::stringstream binStream;
    binStream << std::hex << std::setfill('0') << std::setw(8) << hexVal;
    std::string hexStr = binStream.str();
    std::string binStr;
    for (char c : hexStr) {
        unsigned int n = 0;
        if (c >= '0' && c <= '9') {
            n = c - '0';
        } else if (c >= 'A' && c <= 'F') {
            n = c - 'A' + 10;
        } else if (c >= 'a' && c <= 'f') {
            n = c - 'a' + 10;
        }
        for (int i = 3; i >= 0; i--) {
            if ((n >> i) & 1) {
                binStr += '1';
            } else {
                binStr += '0';
            }
        }
    }
    return binStr;
}

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

        //Note display
        uint16_t notes_comb = res & 0x0FFF;
        std::string binNotes = hexToBin(notes_comb);
        std::string notesStr = "Keys:";
        if(binNotes[31] == '0')
            notesStr += " C";
        if(binNotes[30] == '0')
            notesStr += " C#";
        if(binNotes[29] == '0')
            notesStr += " D";
        if(binNotes[28] == '0')
            notesStr += " D#";
        if(binNotes[27] == '0')
            notesStr += " E";
        if(binNotes[26] == '0')
            notesStr += " F";
        if(binNotes[25] == '0')
            notesStr += " F#";
        if(binNotes[24] == '0')
            notesStr += " G";
        if(binNotes[23] == '0')
            notesStr += " G#";
        if(binNotes[22] == '0')
            notesStr += " A";
        if(binNotes[21] == '0')
            notesStr += " A#";
        if(binNotes[20] == '0')
            notesStr += " B";
        char keyBuff[9];
        sprintf(keyBuff, "%08X", ~res & 0xFFFFFFFF);
        std::string vol_perc = std::to_string(100*VOLUMEMOD/8) + "%";
        std::string volumeStr = "Vol: " + vol_perc;
        std::string masterStr = (ISMASTER == true) ? ":ON" : ":OFF";
        std::string octaveStr = "Oct: " + std::to_string(OCTAVE);
        std::string waveStr;
        switch (WAVETYPE)
        {
            case 0:
                waveStr = "Wave: Sawtooth";
                break;
            case 1:
                waveStr = "Wave: Pulse";
                break;
            case 2:
                waveStr = "Wave: Sine";
                break;
            case 3:
                waveStr = "Wave: Triangular";
                break;
            default:
                waveStr = "Wave: N/A";
                break;
        }
        //Update display
        u8g2.clearBuffer();         // clear the internal memory
        u8g2.setFont(u8g2_font_5x7_tr); // choose a suitable font
        //u8g2.drawStr(2,8,keyBuff);
        u8g2.drawStr(2,8,notesStr.c_str());
        u8g2.drawStr(2,16,volumeStr.c_str());
        u8g2.drawStr(2,24,masterStr.c_str());
        u8g2.drawStr(40,32,waveStr.c_str());
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
    uint8_t updateWave = max(0, min(1, WAVETYPE+rotationDirection((prevKeys[3]>>2)&0b11, (currKeys[3]>>2)&0b11)));
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