#include "ourLibrary.h"

//Task for key scanning - period of 50ms
void scanKeysTask(void * pvParameters) {
    const TickType_t xFrequency = 50/portTICK_PERIOD_MS;
    TickType_t xLastWakeTime = xTaskGetTickCount();
    //uint8_t tempArray[7];
    while(1) {
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
        for(int i = 0; i < 7; i++) {
            setRow(i);
            delayMicroseconds(3);
            xSemaphoreTake(keyArrayMutex, portMAX_DELAY);
            keyArray[i] = readCols();
            xSemaphoreGive(keyArrayMutex);
        }
    }
    //Copy into keyArray
    //std::copy(std::begin(tempArray), std::end(tempArray), std::begin(keyArray));
}

//Task for display updating - period 100ms
void displayUpdateTask(void * pvParameters) {
    const TickType_t xFrequency = 100/portTICK_PERIOD_MS;
    TickType_t xLastWakeTime = xTaskGetTickCount();
    while(1) {
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
        //Display button press
        uint32_t res = 0;
        uint8_t tempArray[7];
        //Copy from keyArray
        xSemaphoreTake(keyArrayMutex, portMAX_DELAY);
        std::copy(std::begin(keyArray), std::end(keyArray), std::begin(tempArray));
        xSemaphoreGive(keyArrayMutex);
        //Obtain hex code for keypresses
        for (int i = 0; i < 7; i++) {
            res = res | (tempArray[i] << i*4); 
        }
        char buf[8];
        sprintf(buf, "%07X", ~res & 0x0FFFFFFF);
        std::string result(buf);
        //Update display
        u8g2.clearBuffer();         // clear the internal memory
        u8g2.setFont(u8g2_font_ncenB08_tr); // choose a suitable font
        u8g2.drawStr(2,10,"Hello World!");  // write something to the internal memory
        u8g2.setCursor(2,20);
        u8g2.print(result.c_str());
        u8g2.sendBuffer();          // transfer internal memory to the display
    }
}