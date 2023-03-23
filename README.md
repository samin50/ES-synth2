# ES-synth-starter

  Use this project as the starting point for your Embedded Systems labs and coursework.
  
  [Lab Part 1](doc/LabPart1.md)
  
  [Lab Part 2](doc/LabPart2.md)

## Additional Information

  [Handshaking and auto-detection](doc/handshaking.md)
  
  [Double buffering of audio samples](doc/doubleBuffer.md)

# Report



| Tasks                     | Priority (Low to High) | Minimum Initiation Interval τi (ms) | Worst-case Execution Time Ti (ms) | [τn/τi] Ti (ms) | CPU Utilisation (%) |
|---------------------------|------------------------|-------------------------------------|----------------------------------|-----------------|----------------------|
| playbackTask              | 1                      | 49.979                              | 0.002                            | 0.003           | 0.003                |
| displayUpdateTask         | 2                      | 82.568                              | 17.382                           | 17.382          | 17.382               |
| scanKeyTask               | 3                      | 49.886                              | 0.095                            | 0.0574          | 0.0574               |
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
| sampleISR Polyphony: 1    | Interrupt              | 0.0455                              | 0.018                            | 32.664          | 32.664               |
