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
    Serial.println(RX_Message[2]);
  }
}

void sendMessage(uint32_t id, uint8_t* data, uint8_t length) {
  uint8_t TX_Message[8];
  memcpy(TX_Message, data, length); // Copy data into txMessage array
  //CAN_TX(id, data); // Send the message to the specified ID
  xQueueSend(msgOutQ, TX_Message, portMAX_DELAY);
}

// void stateChange(uint8_t prevKeys[], uint8_t currKeys[]){
// 	int index = 0;
// 	if (prevKeys != currKeys){
// 		bool keyFound = false;
// 		int key;
// 		for (int i=0;i<3;i++){
// 			if (prevKeys[i]^currKeys[i]!=0){
// 				for (int b = 0; b < 8; b++) {
// 					uint8_t result =  prevKeys[b]^currKeys[b];
// 					if ((result & (8 >> b)) != 0) {
// 						Serial.println(result);
// 						index = b;
// 					}
// 				State[2] = index;
// 			}
// 		}
// 	}}
// }

void stateChange(uint8_t prevKeys[], uint8_t currKeys[]){
	uint8_t prevKeyRow;
	uint8_t currKeyRow;
	uint8_t columnIndex = 0;
	uint8_t keyNum = 0;

	for (int i=0; i<3;i++){
		prevKeyRow = prevKeys[i];
		currKeyRow = currKeys[i];
		columnIndex = 0;
		keyNum = 0;
		for (int columnIndex = 0; columnIndex < 4; columnIndex++) {
			keyNum = (i*4)+columnIndex;
			if (((prevKeyRow&1)^(currKeyRow&1))==1) {

				if ((prevKeyRow&1) == 0){ 	//key Pressed
					RX_Message[0] = 'R';
					RX_Message[2] = keyNum;
				}
				else{ 						//Key released
					RX_Message[0] = 'P';
					RX_Message[2] = keyNum;
				}
				sendCurrKeys();	
			}
			currKeyRow = currKeyRow >> 1;
			prevKeyRow = prevKeyRow >> 1;
		}
	}
}

void sendCurrKeys(){
	//TX_Message[1] = tempArray[0];
	//TX_Message[2] = 9;
	sendMessage(0x456,RX_Message,8);
}