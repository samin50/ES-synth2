#include "Imports/ourLibrary.h"

//Function to set outputs using key matrix
void setOutMuxBit(const uint8_t bitIdx, const bool value) {
      digitalWrite(REN_PIN,LOW);
      digitalWrite(RA0_PIN, bitIdx & 0x01);
      digitalWrite(RA1_PIN, bitIdx & 0x02);
      digitalWrite(RA2_PIN, bitIdx & 0x04);
      digitalWrite(OUT_PIN,value);
      digitalWrite(REN_PIN,HIGH);
      delayMicroseconds(3);
      digitalWrite(REN_PIN,LOW);
}

void setup() {
  //Set pin directions
  pinMode(RA0_PIN, OUTPUT);
  pinMode(RA1_PIN, OUTPUT);
  pinMode(RA2_PIN, OUTPUT);
  pinMode(REN_PIN, OUTPUT);
  pinMode(OUT_PIN, OUTPUT);
  pinMode(OUTL_PIN, OUTPUT);
  pinMode(OUTR_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  pinMode(C0_PIN, INPUT);
  pinMode(C1_PIN, INPUT);
  pinMode(C2_PIN, INPUT);
  pinMode(C3_PIN, INPUT);
  pinMode(JOYX_PIN, INPUT);
  pinMode(JOYY_PIN, INPUT);
  //Initialise CAN
  CAN_Init(false);
  setCANFilter(MASTER_ID,0x7ff);
  CAN_RegisterRX_ISR(CAN_RX_ISR);
  CAN_RegisterTX_ISR(CAN_TX_ISR);
  CAN_Start();
  CAN_TX_Semaphore = xSemaphoreCreateCounting(3,3);
  msgInQ = xQueueCreate(36,8);
  msgOutQ = xQueueCreate(36,8);
  xTaskCreate(decodeTask, "Decode", 256, NULL, 3, NULL);
  xTaskCreate(CANSend, "CANSend", 256, NULL, 4, NULL);
  //Initialise display
  TaskHandle_t displayUpdateTaskHandle = NULL;
  xTaskCreate(displayUpdateTask, "displayUpdate", 128, NULL, 1,	&displayUpdateTaskHandle);
  setOutMuxBit(DRST_BIT, LOW);  //Assert display logic reset
  delayMicroseconds(2);
  setOutMuxBit(DRST_BIT, HIGH);  //Release display logic reset
  u8g2.begin();
  setOutMuxBit(DEN_BIT, HIGH);  //Enable display power supply

  //Keyscanner
  TaskHandle_t scanKeysHandle = NULL;
  xTaskCreate(scanKeysTask, "scanKeys", 512, NULL, 2,	&scanKeysHandle);
  keyArrayMutex = xSemaphoreCreateMutex();
  //Hardware Timer for sound
  TIM_TypeDef *Instance = TIM1;
  HardwareTimer *sampleTimer = new HardwareTimer(Instance);
  sampleTimer->setOverflow(22000, HERTZ_FORMAT);
  sampleTimer->attachInterrupt(sampleISR);
  sampleTimer->resume();
  //Initialise UART
  Serial.begin(9600);
  Serial.println("Hello World");
  //Initialise accumulator map
  //Initialise map
  for(int i = 0; i < POLYPHONY; i++) {
      accumulatorMap[i] = NULL;
  }
  //Start tasks
  vTaskStartScheduler();
}

void loop() {
  // put your main code here, to run repeatedly:
  static uint32_t next = millis();
  static uint32_t count = 0;

  while (millis() < next);  //Wait for next interval
  next += INTERVAL;

  //Toggle LED
  digitalToggle(LED_BUILTIN);
  
}