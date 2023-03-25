#include "ourLibrary.h"
//File handles all menus that are displayed

//Task for display updating - period 100ms
void displayUpdateTask(void * pvParameters) {
    const TickType_t xFrequency = 100/portTICK_PERIOD_MS;
    TickType_t xLastWakeTime = xTaskGetTickCount();
    u8g2.setFont(u8g2_font_5x7_tr); // choose a suitable font
    while(1) {
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
        //Time analysis
        #ifdef TEST_MODE
            return;
        #endif
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

void printKey() {
    //Display button press
    std::string notesStr = "";
    std::string notes[12]= {"C", "C#","D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
    //Copy from Accumulator
    for (int i = 0; i < POLYPHONY; i++){
        if (accumulatorMap[i] != NULL) {
            notesStr += notes[accumulatorMap[i]%12]; //Adds key to display string
            notesStr += std::to_string(accumulatorMap[i]/12); //Adds octave to display string
            notesStr += " ";
        }
    }
    u8g2.setFont(u8g2_font_4x6_tr); //text size reduced for notes to fit all on the screen
    u8g2.drawStr(2,6,notesStr.c_str());
    u8g2.setFont(u8g2_font_5x7_tr);
}

void printTime() {
    uint32_t eventTime = millis()-REFTIMER;
    std::string timeStr = "Time: " + std::to_string(eventTime);
    u8g2.drawStr(2,24, timeStr.c_str());
    std::string returnStr = "<   Stop and Return";
    u8g2.drawStr(2,32,returnStr.c_str());
}

void mainScreen() {
    std::string vol_perc = std::to_string(100*VOLUMEMOD/64) + "%";
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
    std::string keyStr = "Key Memory: " + std::to_string(CURRENTKEY) + "/" + std::to_string(MAXKEYS-1);
    u8g2.drawStr(2,16, keyStr.c_str());
}

void playScreen() {
    std::string keyStr = "Key Number: " + std::to_string(CURRENTKEY) + "/" + std::to_string(MAXKEYS-1);
    u8g2.drawStr(2,16, keyStr.c_str());
}