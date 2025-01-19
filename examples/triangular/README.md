# Triangular Example

This example demonstrates how to use a Lookup Table (LUT) to generate a triangular waveform.

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
   cd waveu/examples/triangular
   ```

3. **Run menuconfig**:
   ```bash
   idf.py menuconfig
   ```
   In `menuconfig`, configure the following
   settings under `[Component config > Waveu Configuration]`:

   - **Select active DAC channels**:
     - **[CH0 and CH1]**: Output to both channels.
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
   You should observe messages in the console confirming the start and stop of waveform generation.

6. **Connect your oscilloscope**:
   Attach the oscilloscope to the DAC output channel as configured in `menuconfig`.

## Example Breakdown

### 1. Define the Triangular Function

The triangular function generates LUT values. It calculates values based on the current phase, amplitude, and offset:

```cpp
virtual lut_type_t fill_function_triangular(LUTSize size, size_t index, float amplitude, float offset) {
   ...

   // Generate the triangular value
   float triangular_value = (phase < 0.5)
      ? (2 * phase)                    // Rising edge
      : (2 * (1 - phase));             // Falling edge

   // Scale and shift to amplitude and offset
   return static_cast<lut_type_t>((2 * triangular_value - 1) * amplitude + offset + 0.5);
}
```

### 2. Override `initialize()` Method

The `initialize()` method sets up the `PhaseGenerator` and LUT index calculation.

#### 2.1 Initialize the Phase Generator
Set the sample rate and create the phase generator.
```cpp
initializePhaseGenerator(sampleRate);
```

#### 2.2 Retrieve the LUT Index Function
Retrieve a function to map the current phase to an LUT index.
```cpp
_getIndex = LUTHelper::getIndexFunction(_lutSize);
```


### 3. Override `configure()` Method

The `configure()` method prepares waveform generation.

#### 3.1 Set the Frequency
Set the desired frequency and calculate the phase increment.
```cpp
setPhaseGeneratorFrequency(waveArgs.frequency);
```

#### 3.2 Adjust Amplitude and Offset
Ensure the amplitude and offset values are valid for the DAC.
```cpp
auto [adjustedAmplitude, adjustedOffset] 
    = LUTHelper::adjustAmplitudeAndOffset(waveArgs.amplitude, waveArgs.offset);
```

#### 3.3 Populate the LUT
Fill the LUT using the triangular function.
```cpp
populateLUTs(adjustedAmplitude, adjustedOffset);
```


### 4. Override `nextSample()` Method

The `nextSample()` method generates waveform samples dynamically.

#### 4.1 Update the Phase
Advance the phase to the next position.
```cpp
_phaseGenerator->updatePhase();
```

#### 4.2 Retrieve the Current Phase
Get the current phase value from the phase generator.
```cpp
uint32_t currentPhase = _phaseGenerator->getPhase();
```

#### 4.3 Map to LUT Index
Calculate the corresponding LUT index.
```cpp
int lutIndex = _getIndex(currentPhase, PhaseGenerator::N_BITS);
```

#### 4.4 Generate Voltage Value
Convert the phase or LUT index to a voltage value.
```cpp
lut_type_t digi_val = GET_LUT_VALUE(_lut, lutIndex);
```

#### 4.5 Return the Voltage
Return the voltage value for output.
```cpp
return digi_val;
```

### 5. Observe the Results

Connect an oscilloscope to the DAC pin and observe the waveform output. The example generates a continuous triangular waveform, defaulting to 293.66 Hz.


## Notes

- Configure the DAC output channel via `menuconfig`.
- Adjust frequency, amplitude, and offset in the `app_main()` function to match your testing requirements.
- This example demonstrates using an LUT for waveform generation and is ideal for learning signal generation techniques.
