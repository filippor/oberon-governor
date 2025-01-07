# Oberon GPU Governor

A simple daemon for AMD Oberon based systems that automatically governs GPU voltage and frequency based on load and temperature.

Testing is only performed on the ASRock BC-250.

## Dependencies

- CMake
- A C++ toolchain
- libdrm

## Configuration

- A configuration file is installed to /etc/oberon-config.yaml. This allows the maximum and minimum voltage and frequency to be set.

## Limitations

- Frequency ramp up under load is a bit slow.
