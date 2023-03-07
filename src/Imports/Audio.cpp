#include "ourLibrary.h"

//Function for audio generation
void sampleISR() {
    static uint32_t phaseAcc[POLYPHONY];
    uint32_t Vout = 0;
    uint8_t octaveOffset;
    //Polyphonic keypresses
    for(int i = 0; i < POLYPHONY; i++) {
        //Obtain octave information from accumulatorMap
        octaveOffset = 4-accumulatorMap[i];
        //If the wave is sawtooth - simple addition
        if(WAVETYPE == 0) {
            phaseAcc[i] += (currentStepSize[i] >> octaveOffset);
        }
        //After calculating accumulator values for different wave types
        Vout += floor(((phaseAcc[i] >> 24) - 128)/POLYPHONY);
    }
    //Volume control here
    analogWrite(OUTR_PIN, Vout + 128);
}
