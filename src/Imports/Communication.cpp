#include "ourLibrary.H"

void CAN_RX_ISR (void) {
	uint8_t RX_Message_ISR[8];
	uint32_t ID;
	CAN_RX(ID, RX_Message_ISR);
	xQueueSendFromISR(msgInQ, RX_Message_ISR, NULL);
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
  //uint8_t txMessage[8];
  //memcpy(txMessage, data, length); // Copy data into txMessage array
  CAN_TX(id, data); // Send the message to the specified ID
}