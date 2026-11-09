# TODO

## Roadmap

First a working device with HyperHDR, then infrastructure, power and other hosts.

1. **MVP.** CDC + AWA/Adalight + handshake, APA102/SK9822 on SPI1, "loading" effect, constant current limit.
   Measure latency and fps with HyperHDR.
2. **Native protocol and settings.** Vendor interface (WinUSB/libusb), settings in flash, PC utility, telemetry.
3. **umcub.** Bootloader, image signing, update from the application, recovery mode by button.
4. **Color quality and power.** 16-bit pipeline, dithering, SK9822 profile with global brightness, power regulator +
   INA226.
5. **Power.** PD on port B (HUSB238 over I2C), power multiplexer, converter; first own board (5 V revision).
6. **Outputs and strip types.** 4 outputs, segment map, encoder for one-wire strips, 12 V revision.
7. **Android.** Raspberry Pi + HyperHDR scenario, then an own app (MediaProjection + USB host).
8. **Several devices and options.** Sync by timestamps, HID LampArray for Windows Dynamic Lighting, own screen
   capture if needed.

## Open questions and checks

The first thing to measure on the prototype: does the HyperHDR grabber hold 180 frames/s — it decides whether an own
capture is needed.

- HyperHDR grabber on Windows and Linux: actual fps and capture latency at 180 Hz.
- APA102 or SK9822 in fact: die size of the chip; without the reset frame an SK9822 lags a frame.
- Does 12 MHz work over 15-20 cm of wire; without a buffer use 6 MHz.
- Quiescent current per LED and current per channel for the regulator model — measure.
- Slot layout in the umcub defconfig for the F411.
- Pinout of the 4 outputs (SPI2-5) on the 48-pin package: alternate functions and DMA channels without conflicts —
  from the datasheet and RM0383.
- How to read the Type-C current on port A of the own board (Rp level on CC).
- Is EPR needed — only for a setup on a large TV with a 5 V strip.
- Get an own PID from pid.codes once the repository is public.
- Licenses of borrowed code: HyperSerialPico and HyperHDR — MIT, TinyUSB — MIT, umcub — Apache-2.0; do not take
  Prismatik code (GPL-3).
