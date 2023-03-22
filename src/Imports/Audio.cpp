#include "ourLibrary.h"

//Function for audio generation
void sampleISR() {
    static uint32_t phaseAcc[POLYPHONY];
    int32_t Vout = 0;
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
        if (octaveOffset > 0) {
            phaseAcc[i] += (currentStepSize[i] >> octaveOffset);
        } else {
            phaseAcc[i] += (currentStepSize[i] << -octaveOffset);
        }
        //If the wave is sawtooth - simple addition
        if(WAVETYPE == 0) {
            Vout += ((phaseAcc[i] >> 24) - 128)/POLYPHONY;
        } else if(WAVETYPE == 1) { //Pulse wave
            if ((phaseAcc[i] >> 24) < 128) {
                Vout -= 127/POLYPHONY;
            } else {
                Vout += 127/POLYPHONY;
            }
        } else if(WAVETYPE == 2) { //Sine wave
            uint8_t sineIndx = (phaseAcc[i] >> 24) % 256;
            //int32_t sineVal = sinLUT[sineIndx];

            Vout += (sinLUT[sineIndx]/2) / (POLYPHONY);
        } else if(WAVETYPE == 3) { //Triangular wave
            if ((phaseAcc[i] >> 24) < 128) {
                Vout += ((phaseAcc[i] >> 24)) / POLYPHONY;
            } else {
                Vout += (255 - ((phaseAcc[i] >> 24))) / POLYPHONY;
            }
        }
    }
    //Volume control here
    analogWrite(OUTR_PIN, (((Vout + 128)*VOLUMEMOD) >> 3));
}