# Start And Stop Example

This example demonstrates basic usage to configure and control the waveform generation process. Specifically, it shows how to set up the waveform generation system, initialize parameters, start the waveform output, and stop it. The methods demonstrated include `configure()`, `start()`, `stop()` and `reset()`.

## Prerequisites

Before running this example, ensure you have:

- An ESP32 development board.
- ESP-IDF development environment set up (version 5.4 or later).
- Familiarity with the ESP-IDF [DAC Constant Example](https://github.com/espressif/esp-idf/tree/v5.4/examples/peripherals/dac/dac_continuous/signal_generator).
- An oscilloscope to visualize the waveform output.

## Usage

1. **Clone the repository**:
   ```bash
   git clone https://github.com/tinyalg/waveu.git
   ```

2. **Change to this directory**:
   ```bash
   cd waveu/examples/start_n_stop
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

   To reproduce the figure in [A Hands-On Approach to Designing Waveforms with Microcontrollers – Your Spare Oscilloscope in Action](https://doi.org/10.5281/zenodo.14651345), ensure the following options are enabled:
   - **Enable debug output for waveform data generation on GPIO**.
   - **Enable debug output for DAC transfer on GPIO**.
   - **Select DAC channel working mode** as Alternate mode (CH0 and CH1 alternate data from buffer)

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

This example consists of the following key parts:

### 1. Initialization
The example begins by initializing the Waveu instance.

```cpp
tinyalg::waveu::ESP32Waveu<UserWaveConfig> waveu;
```

### 2. Prepare Arguments for Configuration
Next, prepare arguments for configuration. In this example, a frequency value of 312.5 Hz is used.

```cpp
UserWaveArgs waveArgs(312.5f);
```

### 3. Configuration
Then, configure Waveu with necessary arguments.

```cpp
waveu.configure(waveArgs);
```

### 4. Starting Waveform Generation
Now, the `start()` method begins the waveform generation process.

```cpp
waveu.start();
```

### 5. Stopping Waveform Generation
The `stop()` method gracefully halts the waveform generation, ensuring output of the current data buffer.

```cpp
waveu.stop();
```

### 6. Observing the Results
Connect an oscilloscope to verify the waveform output. Observe a continuous waveform at 261.63 Hz for 5 seconds. After calling stop(), the waveform generation pauses and resumes after another 5 seconds, demonstrating the basic start-stop functionality.

## Notes
- Ensure the channel configuration is correctly set in `menuconfig`.
- Adjust the frequency parameter as needed to fit your application or testing scenario.
