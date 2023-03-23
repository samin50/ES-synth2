# ES-synth-starter

  Use this project as the starting point for your Embedded Systems labs and coursework.
  
  [Lab Part 1](doc/LabPart1.md)
  
  [Lab Part 2](doc/LabPart2.md)

## Additional Information

  [Handshaking and auto-detection](doc/handshaking.md)
  
  [Double buffering of audio samples](doc/doubleBuffer.md)

# Report

### Display: A main menu screen that 

## Tasks

### Threads

- ***scanKeysTask*** :  

- ***displayUpdateTask*** : Sampled at 100ms based on specification, it displays all keys currently pressed, the volume level, octave and type of wave. There's a separate advanced menu accessible by rotating a knob (arrow displayed over knobs that need to be turned to go to or return from menu) which contains two features: ability to set keyboard as a Host (receiver) or change to sender, and the start or stop recording feature with a timer () and number of acuumulated notes displayed while recording and during playback.

- ***decodeTask*** : Decodes received messages and allocates pressed keys and dealocates released keys from the accumulaor.

- ***CANSend*** : Creates a 8-bit unsigned integer and allocates it the outgoing message from the queue. It checks the CAN_TX counting semaphore and send the message via CAN bus if a slot is free.  

- ***playbackTask*** : 

### Interupts

- ***CAN_RX_ISR*** : This interrupt is called when a message is received from another keyboard through the CAN bus. The message is added to a received message queue.

- ***CAN_TX_ISR*** : This is called when a CAN message is being sent through the CAN bus. It uses counting sephamores to ensure that there are enough free output slots. 

- ***sampleISR*** :


## Performance Testing
| Tasks                     | Priority (Low to High) | Minimum Initiation Interval τi (ms) | Worst-case Execution Time Ti (ms) | [τn/τi] Ti (ms) | CPU Utilisation (%) |
|---------------------------|------------------------|-------------------------------------|----------------------------------|-----------------|----------------------|
| playbackTask              | 1                      | 49.979                              | 0.002                            | 0.003           | 0.003                |
| displayUpdateTask         | 2                      | 82.568                              | 17.382                           | 17.382          | 17.382               |
| scanKeysTask               | 5                      | 49.886                              | 0.095                            | 0.0574          | 0.0574               |
| decodeTask               | 3                      | NA                             | 0.NA                            | N/A          | NA               |
| CANSend               | 4                      | NA                             | 0.NA                            | NA          | NA               |
| sampleISR Polyphony: 11   | Interrupt              | 0.0455                              | 0.049                            | 88.919          | 88.919               |
| sampleISR Polyphony: 10   | Interrupt              | 0.0455                              | 0.046                            | 83.475          | 83.475               |
| sampleISR Polyphony: 9    | Interrupt              | 0.0455                              | 0.043                            | 78.031          | 78.031               |
| sampleISR Polyphony: 8    | Interrupt              | 0.0455                              | 0.039                            | 70.777          | 70.777               |
| sampleISR Polyphony: 7    | Interrupt              | 0.0455                              | 0.037                            | 67.143          | 67.143               |
| sampleISR Polyphony: 6    | Interrupt              | 0.0455                              | 0.034                            | 61.699          | 61.699               |
| sampleISR Polyphony: 5    | Interrupt              | 0.0455                              | 0.031                            | 56.255          | 56.255               |
| sampleISR Polyphony: 4    | Interrupt              | 0.0455                              | 0.028                            | 50.811          | 50.811               |
| sampleISR Polyphony: 3    | Interrupt              | 0.0455                              | 0.025                            | 45.367          | 45.367               |
| sampleISR Polyphony: 2    | Interrupt              | 0.0455                              | 0.021                            | 38.108          | 38.108               |
| sampleISR Polyphony: 1    | Interrupt              | 0.0455                              | 0.018                            | 32.664          | 32.664               

## Shared Resources

## Task Dependencies


## Advanced Features

### Polyphony
This allows multiple key output with upto 8 keys (cpu limitation) at the same time.
### Multi-page menu
An additional menu allowing access to the advanced features
### Host/Sender Control 
Allows each keyboard to identify as a host or other (sender) through the advanced menu. The host's settings will apply to all non-host boards and sound will only play from the host's speakers.
### Recording and full playback of sequence
Option to record a sequence of notes with a timer on display and full playback through the host's advanced menu options.