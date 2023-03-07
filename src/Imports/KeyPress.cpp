#include "ourLibrary.h"

void setupKeyScan() {
  keyArrayMutex = xSemaphoreCreateMutex();
  //Task scheduler for keyScan
  TaskHandle_t scanKeysHandle = NULL;
  xTaskCreate(
  scanKeysTask,		/* Function that implements the task */
  "scanKeys",		/* Text name for the task */
  512,      		/* Stack size in words, not bytes */
  NULL,			/* Parameter passed into the task */
  2,			/* Task priority */
  &scanKeysHandle);	/* Pointer to store the task handle */
  
  //Display update
  TaskHandle_t displayUpdateTaskHandle = NULL;
  xTaskCreate(
  displayUpdateTask,		/* Function that implements the task */
  "displayUpdate",		/* Text name for the task */
  64,      		/* Stack size in words, not bytes */
  NULL,			/* Parameter passed into the task */
  1,			/* Task priority */
  &displayUpdateTaskHandle);	/* Pointer to store the task handle */
}

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