#include "ourLibrary.H"

void CAN_RX_ISR (void) {
	uint8_t RX_Message_ISR[8];
	uint32_t ID;
	CAN_RX(ID, RX_Message_ISR);
	xQueueSendFromISR(msgInQ, RX_Message_ISR, NULL);
}

void CANSend(void * pvParameters) {
	uint8_t msgOut[8];
	while (1) {
	xQueueReceive(msgOutQ, msgOut, portMAX_DELAY);
		xSemaphoreTake(CAN_TX_Semaphore, portMAX_DELAY);
		CAN_TX(0x456, msgOut);
	}
}

void CAN_TX_ISR(void) {
	xSemaphoreGiveFromISR(CAN_TX_Semaphore, NULL);
}

void decodeTask(void *pvParamters) {
  uint8_t RX_Message[8];
  while (1) {
    xQueueReceive(msgInQ, RX_Message, portMAX_DELAY);
    // Process the received message
    Serial.println(RX_Message[0]);
  }
}

void sendMessage(uint32_t id, uint8_t* data, uint8_t length) {
  uint8_t TX_Message[8];
  memcpy(TX_Message, data, length); // Copy data into txMessage array
  //CAN_TX(id, data); // Send the message to the specified ID
  xQueueSend(msgOutQ, TX_Message, portMAX_DELAY);
}