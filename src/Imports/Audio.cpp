#include "ourLibrary.h"

//Function for audio generation - 22000 times a second
//Desired vibrato frequency is ~6Hz, increment sample by 1 every
void sampleISR() {
    static uint32_t phaseAcc[POLYPHONY];
    int32_t Vout = 0;
    int16_t octaveOffset;
    float volumeMod = VOLUMEMOD/10;
    //If not master, do not play
    if (!ISMASTER) {
        return;
    }
    //Polyphonic keypresses
    for(int i = 0; i < POLYPHONY; i++) {
        //Obtain octave information from accumulatorMap and use pitch bend
        octaveOffset = 4-accumulatorMap[i];
        if (octaveOffset > 0) {
            phaseAcc[i] += ((currentStepSize[i] >> octaveOffset)*JOYSTICKY)/8;
        } else {
            phaseAcc[i] += ((currentStepSize[i] << -octaveOffset)*JOYSTICKY)/8;
        }
        uint8_t sineIndx = (phaseAcc[i] >> 24) % 256;
        switch (WAVETYPE) {
            case 0:
            {
            //Sawtooth
                Vout += ((phaseAcc[i] >> 24) - 128)/POLYPHONY;
                break;
            }
            case 1:
            {
            //Pulse wave
                if ((phaseAcc[i] >> 24) < 128) {
                    Vout -= 127/POLYPHONY;
                } else {
                    Vout += 127/POLYPHONY;
                }
                break;
            }
            case 2:
            {
            //Sine wave
                Vout += (sinLUT[sineIndx]/2) / (POLYPHONY);
                break;
            }
            case 3: 
            {
            //Triangular wave
                if ((phaseAcc[i] >> 24) < 128) {
                    Vout += ((phaseAcc[i] >> 24)) / POLYPHONY;
                } else {
                    Vout += (255 - ((phaseAcc[i] >> 24))) / POLYPHONY;
                }
                break;
            }
        }
    }
    //Volume control here
    analogWrite(OUTR_PIN, (Vout + 128)*VOLUMEMOD >> 3);
}