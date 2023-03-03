#include "ourLibrary.h"

uint8_t readCols() {
    return (digitalRead(C3_PIN) << 3) | (digitalRead(C2_PIN) << 2) | (digitalRead(C1_PIN) << 1) | digitalRead(C0_PIN);
}

void setRow(uint8_t rowIdx) {
    //Disable row reading
    digitalWrite(REN_PIN, LOW);
    digitalWrite(RA0_PIN, rowIdx & 1);
    digitalWrite(RA1_PIN, rowIdx & 2);
    digitalWrite(RA2_PIN, rowIdx & 4);
    //Enable
    digitalWrite(REN_PIN, HIGH);
}

void scanKeysTask(void * pvParameters) {
    for(int i = 0; i < 7; i++) {
        setRow(i);
        delayMicroseconds(3);
        keyArray[i] = readCols();
    }
}

std::string readKeys() {
    scanKeysTask(NULL);
    uint16_t res = 0;
    for (int i = 0; i < 3; i++) {
        res = res | (keyArray[i] << i*4); 
    }
    char buf[3];
    sprintf(buf, "%03X", ~res & 0x00000FFF);
    std::string result(buf);
    return result;
}