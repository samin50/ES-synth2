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
  #ifdef TEST_MODE
    CAN_Init(true);
  #endif
  #ifndef TEST_MODE
    CAN_Init(false);
  #endif
  setCANFilter(MASTER_ID,0x7ff);
  CAN_RegisterRX_ISR(CAN_RX_ISR);
  CAN_RegisterTX_ISR(CAN_TX_ISR);
  CAN_Start();
  CAN_TX_Semaphore = xSemaphoreCreateCounting(3,3);
  msgInQ = xQueueCreate(36,8);
  msgOutQ = xQueueCreate(36,8);
  #ifndef TEST_MODE
    xTaskCreate(decodeTask, "Decode", 256, NULL, 3, NULL);
    xTaskCreate(CANSend, "CANSend", 256, NULL, 2, NULL);
  #endif
  //Initialise display
  TaskHandle_t displayUpdateTaskHandle = NULL;
  #ifndef TEST_MODE
    xTaskCreate(displayUpdateTask, "displayUpdate", 128, NULL, 1,	&displayUpdateTaskHandle);
  #endif
  setOutMuxBit(DRST_BIT, LOW);  //Assert display logic reset
  delayMicroseconds(2);
  setOutMuxBit(DRST_BIT, HIGH);  //Release display logic reset
  u8g2.begin();
  setOutMuxBit(DEN_BIT, HIGH);  //Enable display power supply
  //Keyscanner
  TaskHandle_t scanKeysHandle = NULL;
  #ifndef TEST_MODE
    xTaskCreate(scanKeysTask, "scanKeys", 512, NULL, 4,	&scanKeysHandle);
  #endif
  keyArrayMutex = xSemaphoreCreateMutex();
  //Playback
  TaskHandle_t playbackTaskHandle = NULL;
  #ifndef TEST_MODE
    xTaskCreate(playbackTask, "playback", 256, NULL, 5,	&playbackTaskHandle);
  #endif
  //Hardware Timer for sound
  #ifndef TEST_MODE
    TIM_TypeDef *Instance = TIM1;
    HardwareTimer *sampleTimer = new HardwareTimer(Instance);
    sampleTimer->setOverflow(22000, HERTZ_FORMAT);
    sampleTimer->attachInterrupt(sampleISR);
    sampleTimer->resume();
  #endif
  //Initialise UART
  Serial.begin(9600);
  //Initialise accumulator map
  for(int i = 0; i < POLYPHONY; i++) {
      accumulatorMap[i] = NULL;
      currentStepSize[i] = 0;
  }
  for(int i = 0; i < 84; i++) {
      pianoKeyMap[i] = NULL;
  }
  //Start tasks
  #ifndef TEST_MODE
    vTaskStartScheduler();
  #endif
  #ifdef TEST_MODE
    timingAnalysis();
  #endif
}

//Timing analysis function
void timingAnalysis() {
  //Timing analysis
  SCREENNUM = 2;
  WAVETYPE = 3;
  //Create worst case scenario
  for(int i = 0; i < POLYPHONY; i++) {
    currentStepSize[i] = stepSizes[i%12];
    accumulatorMap[i] = i;
  }
  float totalUsage = 0;
  //All 5 tasks and interupts
  for (int j = 0; j < 6; j++) {
    VOLUMEMOD = 5;
    JOYSTICKX = 5;
    uint32_t duration = 0;
    uint32_t startTime = micros();
    for (int i = 0; i < TEST_ITERATIONS; i++) {
      if (j == 0) {
        decodeTask(NULL);
      } else if (j == 1) {
        // CAN_RegisterTX_ISR(CAN_TX_ISR);
        // CANSend(NULL);
        CAN_RegisterRX_ISR(CAN_RX_ISR);
      } else if (j == 2) {
        displayUpdateTask(NULL);
      } else if (j == 3) {
        scanKeysTask(NULL);
      } else if (j == 4) {
        playbackTask(NULL);
      } else if (j == 5) {
        sampleISR();
      }
    }
    duration = (micros()-startTime)/TEST_ITERATIONS;
    //Specific output for each task
    #ifndef STAT_ONLY
      Serial.print(duration);
      Serial.print(", ");
      if (j == 0) {
        Serial.print("NA");
        Serial.println(": Decode task time (us, %)");
      } else if (j == 1) {
        Serial.print("NA");
        Serial.println(": CANSEND task time (us, %)");
      } else if (j == 2) {
        Serial.print(float(duration/1000));
        Serial.println(": displayUpdateTask task time (us, %)");
      } else if (j == 3) {
        Serial.print(float(duration/200));
        Serial.println(": scanKeysTask task time (us, %)");
      } else if (j == 4) {
        Serial.print(float(duration/200));
        Serial.println(": playbackTask task time (us, %)");
      } else if (j == 5) {
        Serial.print(float((duration*100)/45));
        Serial.println(": SampleISR time (us, %)");
      }
    #endif
    #ifdef STAT_ONLY
        if (j == 2) {
          totalUsage += float(duration/1000);
          Serial.println(float(duration/1000));
        } else if (j == 3) {
          totalUsage += float(duration/200);
          Serial.println(float(duration/200));
        } else if (j == 4) {
          totalUsage += float(duration/200);
          Serial.println(float(duration/200));
        } else if (j == 5) {
          totalUsage += float((duration*100)/45);
          Serial.println(float((duration*100)/45));
        }
      #endif
    }
  Serial.print(POLYPHONY);
  Serial.print(", ");
  Serial.println(totalUsage);
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