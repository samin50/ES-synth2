#include "ourLibrary.h"

//Function for audio generation
void sampleISR() {
    static uint32_t phaseAcc[POLYPHONY];
    uint32_t Vout = 0;
    int8_t octaveOffset;
    float volumeMod = VOLUMEMOD/10;
    //If not master, do not play
    if (!ISMASTER) {
        return;
    }
    //Polyphonic keypresses
    for(int i = 0; i < POLYPHONY; i++) {
        //Obtain octave information from accumulatorMap
        octaveOffset = 4-accumulatorMap[i];
        //If the wave is sawtooth - simple addition
        if(WAVETYPE == 0) {
            if (octaveOffset > 0) {
                phaseAcc[i] += (currentStepSize[i] >> octaveOffset);
            } else {
                phaseAcc[i] += (currentStepSize[i] << -octaveOffset);
            }
            Vout += floor(((phaseAcc[i] >> 24) - 128)/POLYPHONY);
        } else if(WAVETYPE == 1) { //Pulse wave
            if (octaveOffset > 0) {
                phaseAcc[i] += (currentStepSize[i] >> octaveOffset);
            } else {
                phaseAcc[i] += (currentStepSize[i] << -octaveOffset);
            }

            if ((phaseAcc[i] >> 24) < 128) {
                Vout -= 16;
            } else {
                Vout += 16;
            }
        } else if(WAVETYPE == 2) { //Sine wave
            if(octaveOffset > 0) {
                phaseAcc[i] += (currentStepSize[i] >> octaveOffset);
            } else {
                phaseAcc[i] += (currentStepSize[i] << -octaveOffset);
            }
            
            uint8_t sineIndx = (phaseAcc[i] >> 24) % 256;
            int32_t sineVal = sinLUT[sineIndx];

            Vout += (sineVal - 128) / POLYPHONY / 2;
        } else if(WAVETYPE == 3) { //Triangular wave
            if(octaveOffset > 0) {
                phaseAcc[i] += (currentStepSize[i] >> octaveOffset);
            } else {
                phaseAcc[i] += (currentStepSize[i] << -octaveOffset);
            }

            if ((phaseAcc[i] >> 24) < 128) {
                Vout += floor(((phaseAcc[i] >> 24) * 2) / POLYPHONY);
            } else {
                Vout += floor((505 - ((phaseAcc[i] >> 24) * 2)) / POLYPHONY);
            }
        }
    }
    //Volume control here
    analogWrite(OUTR_PIN, (((Vout + 128)*VOLUMEMOD) >> 3));
}