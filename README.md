# ambi

Ambilight firmware for the STM32F411: receives frames from the PC over USB (Adalight/AWA for HyperHDR) and drives
ARGB strips over SPI + DMA with minimal latency. Prototype board: WeAct Black Pill V3.0 (STM32F411CEU6).

Status: early development, the MVP is in progress (see [TODO.md](TODO.md)).

## Documentation

- [docs/architecture.md](docs/architecture.md) — goals, frame path, firmware modules, latency, bootloader and updates
- [docs/protocol.md](docs/protocol.md) — USB interfaces, Adalight/AWA with the HyperHDR handshake, native protocol
- [docs/strips.md](docs/strips.md) — strip types, encoders, SPI clocks, signal levels
- [docs/power.md](docs/power.md) — brightness/power regulator, supply and USB PD
- [docs/hardware.md](docs/hardware.md) — current setup, MVP wiring and peripheral setup on the Black Pill
- [docs/hosts.md](docs/hosts.md) — HyperHDR settings, other PC programs, Android

## Repository layout

```
port/            hardware: include/ (contract), arch/cortexm/, stm32f4/ (LL + CMSIS)
src/util/        crc, ring buffers, math (no hardware)
src/pixel/       frame buffer, pipeline, effect, strip encoders (no hardware)
src/proto/       input protocols (Adalight/AWA)
src/usb/         TinyUSB descriptors and callbacks
src/app/         main loop, output scheduler
boards/<board>/  board header and board code
cmake/           toolchain file and family build descriptions
third_party/     submodules: TinyUSB, CMSIS, ST CMSIS device and LL drivers
tests/host/      host tests
tools/           host scripts
```

## Getting the sources

```sh
git clone --recursive https://github.com/memExpert/ambi32.git
```

## Building

Needs CMake 3.20+, Ninja and the GNU Arm toolchain (`arm-none-eabi-gcc` with newlib-nano).

```sh
cmake --preset blackpill && cmake --build --preset blackpill              # Release
cmake --preset blackpill-debug && cmake --build --preset blackpill-debug  # Debug, log on USART1
```

The output is `build/<preset>/ambi.elf` and `ambi.bin` (linked at `AMBI_BOARD_APP_ADDR` of the board header).
Another board: `-DAMBI_BOARD=<directory under boards/>`.

Host tests of the hardware-free code:

```sh
cmake -S tests/host -B build/host -G Ninja && cmake --build build/host && ctest --test-dir build/host
```

`-DAMBI_HOST_SANITIZE=ON` adds ASan/UBSan (needs libasan and libubsan).

## License

Apache-2.0, see [LICENSE](LICENSE). Third-party components keep their own licenses (see `third_party/`).
