# Waveu: Crafting Waveforms with ESP32

Waveu empowers developers to design, configure, and experiment with waveforms using the ESP32. Whether you're learning or prototyping, Waveu makes waveform generation intuitive and accessible.

## Quick Start

Waveu's example code can be compiled and run on your ESP32. Simply connect the DAC pin to an oscilloscope, and you're ready to generate diverse waveforms! Explore the examples in the [examples directory](examples):

- **[Sawtooth Wave](examples/sawtooth)**  
  Demonstrates the `PhaseGenerator` class for generating a sawtooth waveform.

- **[Triangular Wave](examples/triangular)**  
  Learn how to use a Lookup Table (LUT) to create triangular waveforms.

- **[Start and Stop](examples/start_n_stop)**  
  A basic example showcasing how to configure and control waveform generation.


## Hardware Requirements

To use Waveu, you'll need:

- **ESP32 development board**  
- **PC and USB cable** (for programming)  
- **Oscilloscope** (to visualize waveform output)  


## Prerequisites

Before diving into Waveu, ensure you have:

1. **ESP-IDF setup**  
   Install the [ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/get-started/index.html) version 5.4 or later.

2. **Familiarity with DAC examples**  
   We recommend reviewing the ESP-IDF example:  
   [DAC Continuous Signal Generator](https://github.com/espressif/esp-idf/tree/v5.4/examples/peripherals/dac/dac_continuous/signal_generator).


## Features

- **Real-Time Waveform Generation**  
  Generate and visualize waveforms instantly using an ESP32 DAC, perfect for prototyping and experimentation.

- **Easy Configuration**  
  Effortlessly adjust frequency, amplitude, and offset to create customized waveforms tailored to your needs.

- **Seamless Integration**  
  Works out of the box with DAC peripherals, ensuring smooth and reliable operation.

- **Hands-On Learning**  
  Explore and understand waveform behavior in real time using an oscilloscope, making it ideal for education and innovation.


## Learn the Vision

Waveu is built on the vision of making waveform generation accessible and enjoyable. Learn how the project transforms an ESP32 and an oscilloscope into a powerful tool for exploration and learning:  
[A Hands-On Approach to Designing Waveforms with Microcontrollers – Your Spare Oscilloscope in Action](https://doi.org/10.5281/zenodo.14651345).


## Troubleshooting

If you encounter any issues:

- Double-check your hardware connections and software setup.
- Verify your ESP-IDF installation and configuration.
- Explore the [Issues page](https://github.com/tinyalg/waveu/issues) for known bugs or report your own.

#####

Enjoy creating waveforms with Waveu!
