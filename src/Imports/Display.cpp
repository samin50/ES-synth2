#include "ourLibrary.h"

//Task for display updating - period 100ms
void displayUpdateTask(void * pvParameters) {
    const TickType_t xFrequency = 100/portTICK_PERIOD_MS;
    TickType_t xLastWakeTime = xTaskGetTickCount();
    u8g2.setFont(u8g2_font_5x7_tr); // choose a suitable font
    while(1) {
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
        u8g2.clearBuffer();         // clear the internal memory
        if (SCREENNUM == 0) {
            printKey();
            mainScreen();
        } else if (SCREENNUM == 1) {
            settingsScreen();
        } else if (SCREENNUM == 2) {
            printKey();
            printTime();
            recordScreen();
        } else if (SCREENNUM == 3) {
            printKey();
            printTime();
            playScreen();
        }
        u8g2.sendBuffer();          // transfer internal memory to the display
    }
}

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

void printKey() {
     //Display button press
    uint32_t res = 0;
    uint8_t tempArray[8];
    std::string notesStr = "Keys: ";
    //Copy from keyArray
    // xSemaphoreTake(keyArrayMutex, portMAX_DELAY);
    std::string notes[12]= {" C", " C#"," D", " D#", " E", " F", "F#", " G", " G#", " A", " A#", " B"};
    Serial.print(" || ");
    std::copy(std::begin(accumulatorMap), std::end(accumulatorMap), std::begin(tempArray));
    for (int i =0; i<8;i++){
        if (accumulatorMap[i]!=0){
            notesStr += notes[accumulatorMap[i]%12];
        }
    }
    u8g2.drawStr(2,8,notesStr.c_str());

    // xSemaphoreGive(keyArrayMutex);
    //Obtain hex code for keypresses
    // for (int i = 0; i < 8; i++) {
    //     res = res | ((tempArray[i]%12)<<i*4);
    // }
    //Note display
    // uint16_t notes_comb = (res) & 0x0FFF;
    // std::string binNotes = hexToBin(res);
    // std::string notesStr = "Keys: ";
    // for (int i = 0; i<32; i++){
    //     Serial.print(binNotes[i]);
    // }
    // if(binNotes[0] == '1')
    //     notesStr += " C";
    // if(binNotes[1] == '1')
    //     notesStr += " C#";
    // if(binNotes[29] == '0')
    //     notesStr += " D";
    // if(binNotes[28] == '0')
    //     notesStr += " D#";
    // if(binNotes[27] == '0')
    //     notesStr += " E";
    // if(binNotes[26] == '0')
    //     notesStr += " F";
    // if(binNotes[25] == '0')
    //     notesStr += " F#";
    // if(binNotes[24] == '0')
    //     notesStr += " G";
    // if(binNotes[23] == '0')
    //     notesStr += " G#";
    // if(binNotes[22] == '0')
    //     notesStr += " A";
    // if(binNotes[21] == '0')
    //     notesStr += " A#";
    // if(binNotes[20] == '0')
    //     notesStr += " B";
    // u8g2.drawStr(2,8,notesStr.c_str());
}

void printTime() {
    uint32_t eventTime = millis()-REFTIMER;
    std::string timeStr = "Time: " + std::to_string(eventTime);
    u8g2.drawStr(2,24, timeStr.c_str());
    std::string returnStr = "<   Stop and Return";
    u8g2.drawStr(2,32,returnStr.c_str());
}

void mainScreen() {
    std::string vol_perc = std::to_string(100*VOLUMEMOD/8) + "%";
    std::string volumeStr = "Vol: " + vol_perc;
    std::string settingStr = "Set Host + Rec/Play";
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
    u8g2.drawStr(2,16,volumeStr.c_str());
    u8g2.drawStr(40,24,waveStr.c_str());
    u8g2.drawStr(2,24,octaveStr.c_str());
    u8g2.drawStr(2,32,settingStr.c_str());
    u8g2.drawStr(120,32,">");
}

void settingsScreen() {
    std::string masterStr = (ISMASTER) ? "Host: True" : "Host: False";
    std::string recordStr = "Start Recording";
    std::string playbackStr = "Start Playback";
    std::string returnStr = "<   Return";
    //Update display
    u8g2.drawStr(2,8,masterStr.c_str());
    u8g2.drawStr(2,16,recordStr.c_str());
    u8g2.drawStr(120,16,">");
    u8g2.drawStr(2,24,playbackStr.c_str());
    u8g2.drawStr(120,24,">");
    u8g2.drawStr(2,32,returnStr.c_str());
}

void recordScreen() {
    std::string keyStr = "Key Memory: " + std::to_string(CURRENTKEY) + "/" + std::to_string(MAXKEYS);
    u8g2.drawStr(2,16, keyStr.c_str());
}

void playScreen() {
    std::string keyStr = "Key Number: " + std::to_string(CURRENTKEY) + "/" + std::to_string(MAXKEYS);
    u8g2.drawStr(2,16, keyStr.c_str());
}