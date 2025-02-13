# UDP Example

This example demonstrates how to send waveforms over UDP.

## Prerequisites

Before running this example, ensure you have:

- An **ESP32 development board**.
- ESP-IDF development environment set up (**version 5.4 or later**).

## Usage

1. **Clone the repository**:
   ```bash
   git clone https://github.com/tinyalg/waveu.git
   ```

2. **Change to this directory**:
   ```bash
   cd waveu/examples/udp
   ```
   Alternatively, open this directory in VSCode with the ESP-IDF extension for easier navigation and editing.

3. **Run menuconfig**:

   In VSCode, open the ESP-IDF Terminal.
   ```bash
   idf.py menuconfig
   ```
4. **Flash the example**:
   ```bash
   idf.py build flash
   ```

5. **Monitor the output**:
   ```bash
   idf.py monitor
   ```

6. **Run the Viewer**:
   Run UDP 4-Channel Waveform Viewer on PC.

## Example Breakdown


## Notes

- Adjust frequency, amplitude, and offset in the `app_main()` function to match your testing requirements.
