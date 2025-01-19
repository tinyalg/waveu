# Sawtooth Example

This example demonstrates how the `PhaseGenerator` class generates a sawtooth waveform. It serves as a minimal working example of waveform generation, even without using a Look-Up Table (LUT).

## Prerequisites

Before running this example, ensure you have:

- An **ESP32 development board**.
- ESP-IDF development environment set up (**version 5.4 or later**).
- Familiarity with the ESP-IDF [DAC Continuous Signal Generator Example](https://github.com/espressif/esp-idf/tree/v5.4/examples/peripherals/dac/dac_continuous/signal_generator).
- An **oscilloscope** to visualize the waveform output.

## Usage

1. **Clone the repository**:
   ```bash
   git clone https://github.com/tinyalg/waveu.git
   ```

2. **Change to this directory**:
   ```bash
   cd waveu/examples/sawtooth
   ```
   Alternatively, open this directory in VSCode with the ESP-IDF extension for easier navigation and editing.

3. **Run menuconfig**:

   In VSCode, open the ESP-IDF Terminal.
   ```bash
   idf.py menuconfig
   ```
   In `menuconfig`, configure the following
   settings under `[Component config > Waveu Configuration]`:

   - **Select active DAC channels**:
     - **CH0 and CH1**: Output to both channels.
     - **CH0 only**: Output to DAC Channel 0.
     - **CH1 only**: Output to DAC Channel 1.  

4. **Flash the example**:
   ```bash
   idf.py build flash
   ```

5. **Monitor the output**:
   ```bash
   idf.py monitor
   ```
   You should observe messages in the console confirming the start of waveform generation.
   ```plaintext
   I (291) UserWaveConfig: Frequency=200.00
   I (291) Waveu-ESP32Config: waveformDataOutputTask to on core 0 at priority 10.
   I (291) Waveu: LEN_DATA_BUFFER=16000, SAMPLE_RATE=1000000, TIMER_PERIOD=16000
   I (301) Waveu: Started waveformDataGenerationTask on core 1 at priority 5.
   ```

6. **Connect your oscilloscope**:
   Attach the oscilloscope to the DAC output channel as configured in `menuconfig`.

## Example Breakdown

This example consists of the following key parts:

### 1. Override `initialize()` Method

The `initialize()` method is overridden to set up the `PhaseGenerator` and specify the waveform frequency.

#### 1.1 Initialize the PhaseGenerator
Create an instance of the `PhaseGenerator`, which manages phase increments and the current phase.
`sampleRate` specifies the number of samples per second used for waveform generation,
which is provided as an argument to the initialize() method.
```cpp
_phaseGenerator = new PhaseGenerator(sampleRate);
```

#### 1.2 Set the Frequency
Define the desired frequency and calculate the phase increment.
```cpp
float frequency = 200.0f;
_phaseGenerator->setFrequency(frequency);
```

### 2. Override `nextSample()` Method
The `nextSample()` method is overridden to update the phase and return the corresponding voltage value.

#### 2.1 Update the Phase
Advance to the next phase position.
```cpp
_phaseGenerator->updatePhase();
```

#### 2.2 Retrieve the Current Phase
Retrieve the updated phase value.
```cpp
uint32_t currentPhase = _phaseGenerator->getPhase();
```

#### 2.3 Convert the Phase to a Voltage Value
Convert the phase value into an 8-bit voltage value suitable for the DAC.
```cpp
uint8_t digi_val = (currentPhase >> (PhaseGenerator::N_BITS - 8)) & 0xFF;
```

#### 2.4 Return the Voltage Value
Return the computed voltage value.
```cpp
return (uint8_t)digi_val;
```

#### 2.5 Optional Triangular Wave
To generate a triangular wave instead of a sawtooth wave, modify step 2.3 by enabling the triangular wave logic. Set `USE_TRIANGULAR_WAVE` to `true` and apply the following logic:
```cpp
if (USE_TRIANGULAR_WAVE) { 
    uint32_t msb = (currentPhase >> (PhaseGenerator::N_BITS - 1)) & 1;
    digi_val = msb ? digi_val : 255 - digi_val; 
}
```

### 3. Observing the Results
Connect an oscilloscope to verify the waveform output. Observe a continuous sawtooth waveform at 200 Hz.

### 4. How the Sawtooth Wave is Generated
- A sawtooth wave increases linearly from `0` to the maximum value (e.g., `255` for an 8-bit DAC) and resets to `0` at the start of each cycle.
- The fixed-point representation ensures high precision for phase calculations, while the downscaling to 8 bits ensures compatibility with DAC output.


## Notes

- Make sure to configure the DAC output channel in `menuconfig`.
- Adjust the frequency in the `initialize()` method to suit your application or testing needs.
- This example is ideal for learning how to generate simple waveforms programmatically.
