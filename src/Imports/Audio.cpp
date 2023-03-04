#include "ourLibrary.h"

//Function for audio generation
void sampleISR() {
    static uint32_t phaseAcc[polyphony];
    uint32_t Vout = 0;
    //Polyphonic keypresses
    for(int i = 0; i < polyphony; i++) {
        //If the wave is sawtooth - simple addition
        if(waveType == 0) {
            phaseAcc[i] += currentStepSize[i];
        }
        //After calculating accumulator values for different wave types
        Vout += floor(((phaseAcc[i] >> 24) - 128)/polyphony);
    }
    //Volume control here
    analogWrite(OUTR_PIN, Vout + 128);
}
