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
        
        char buf[9];
        sprintf(buf, "%08X", ~res & 0xFFFFFFFF);
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
