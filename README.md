# Oberon GPU Governor

A simple daemon for AMD Oberon based systems that automatically governs GPU voltage and frequency based on load and temperature.

Testing is only performed on the ASRock BC-250.

## Dependencies

- CMake
- A C++ toolchain
- libdrm

## Limitations

- No configuration options are available.
- Frequency ramp up under load is a bit slow.
