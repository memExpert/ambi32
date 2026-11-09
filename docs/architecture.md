# Architecture

The ambi device, built on an STM32F411, receives frames from the PC over USB and drives ARGB strips with minimal
latency at a frame rate close to the monitor's refresh rate.

## Goals and fixed decisions

| Topic | Decision |
|---|---|
| MCU | STM32F411CEU6 (512 KB flash, 128 KB RAM); prototype on a WeAct Black Pill V3.0 |
| Hardware revisions | Two: for 5 V and 12 V strips. They differ in the PD controller and the converter. One firmware; the board reports its revision itself (divider on an ADC pin or an OTP record) |
| Outputs | 4 universal connectors V+ / DATA / CLK / GND, each on its own SPI (SPI1-4, SPI5 spare). Strip type, color order, RGB/RGBW and length are set per output. Only clocked strips use CLK |
| Segment map | The PC sees one chain of LEDs around the screen; the firmware cuts it into outputs (offset, length, reversed order). Works for "one strip on 4 outputs" and for different strips on different outputs |
| USB-C ports | A: data and logic power; B: strip power via PD; A is a fallback source with a brightness limit |
| Without a PC | "Loading" effect: a group of pixels runs around the circle, brightest in the middle of the group, fading to its edges |
| Priority | Lowest latency and a frame rate close to the monitor's refresh rate |
| Brightness | Limited automatically: by the connected strip, the charger's power and the user settings |
| Software stack | TinyUSB; umcub bootloader (MCUboot based); CMake + arm-none-eabi-gcc toolchain, as in umcub |
| Hosts | Windows and Linux now; Android boxes and TVs next, with groundwork laid from the start |
| PC software | HyperHDR as the main host; an own utility for configuration and updates |
| Several devices | Reserve a unique serial number and timestamps in frames; implement later |
| License | Open project: VID 0x1209 + PID from pid.codes; Apache-2.0 for the firmware, CERN-OHL for the board |

## Frame path

```
PC (Windows, Linux; later Android)
  HyperHDR (screen capture, AWA)           own utility (settings, update, later frames)
        | USB CDC                                 | USB vendor bulk
STM32F411 - TinyUSB - super-loop without RTOS
  Adalight / AWA parser                    native protocol v0
  (HyperHDR handshake, statistics)         (frames, settings, firmware)
        \_______________________________________/
                 triple buffer  <---- "loading" effect (no frames for 3 s)
                 (receive, ready, output; a new frame replaces the old one)
                       |
                 16-bit pipeline <---- power budget (PD, INA226, settings)
                 (LUT, current limit, RGBW, dithering)
                       |
     SPI1 + DMA    SPI2 + DMA    SPI3 + DMA    SPI4 + DMA      (encoder per strip type)
                       |
  strips: type, color order and length set per output
  (APA102 / SK9822, WS2812B, SK6812, WS2815, HD108)
```

MVP path: HyperHDR -> AWA -> buffer -> pipeline -> SPI1 -> APA102. The other blocks follow the roadmap (TODO.md).

A frame goes USB -> triple buffer -> 16-bit pipeline -> strip encoder -> DMA -> SPI. No RTOS: a super-loop with
`tud_task()`, DMA interrupts at the highest priority.

Output scheduler rules:

- A frame received completely -> all outputs start in parallel. A busy output starts right after its current transfer.
- No frame queue: receive into one buffer, output from another, the ready frame waits in the third. A newer ready
  frame replaces the older one. If the PC sends faster, USB throttles it by itself (NAK).
- Dithering between frames runs only if it finishes before the expected next frame (the MCU measures the frame period).
- Interpolation between frames (as in FadeCandy) is off by default: it adds up to one frame of latency. Optional
  "smooth" mode for movies.
- No frames for 3 s -> smooth transition (~0.5 s) into the "loading" effect. A frame arrives -> back immediately,
  without a transition.

## Firmware modules

| Module | Responsibility |
|---|---|
| usb_desc.c | Descriptors: CDC + Vendor (+ MS OS 2.0; later optionally HID LampArray); serial number from the 96-bit UID |
| in_awa.c | Adalight / AWA parsing, HyperHDR handshake, statistics line |
| in_native.c | Native protocol over the vendor interface |
| framebuf.c | Triple buffer, source selection (the native protocol has priority), timeout |
| pipeline.c | LUT, power regulator, RGBW, dithering, interpolation (optional) |
| effect.c | "Loading" effect |
| out_spi.c | Output = SPI + DMA; encoder for one-wire or clocked strips |
| enc_*.c | Encoders apa102 / sk9822, ws281x, sk6812, hd108 |
| power.c | I2C to the PD controllers and INA226, current budget |
| config.c | Settings in flash (two sectors alternately, CRC) |
| update.c | Wrapper over `umcub_slot_*` |

Groundwork for several devices: a unique USB serial number and a `host_time_us` field in frames of the native
protocol.

The source tree and layering of these modules: see the repository layout in README.md.

## Latency

APA102 with 130 LEDs is updated in 0.36 ms at 12 MHz SPI, 15 times faster than a 180 Hz frame. The device adds
about 1-1.5 ms; most of the latency is the screen capture on the PC.

| Latency stage | Estimate |
|---|---|
| USB: 130 x 3 ≈ 400 bytes | ~0.3-1 ms |
| Processing on the MCU | < 0.05 ms |
| Output to the strip at 12 MHz | 0.36 ms |
| Total on the device side | ~1-1.5 ms (a frame at 180 Hz is 5.6 ms) |

The speed reserve goes into temporal dithering at ~2 kHz between frames: a new frame waits at most one strip update
(0.36 ms).

## Bootloader (umcub) and updates

The main update path is from the running application over the vendor interface, without re-enumeration or a driver
change; the bootloader is the recovery path.

What [umcub](https://github.com/memExpert/umcub) provides:

- MCUboot based bootloader; the Black Pill F411 is a reference board (UART and USB CDC/DFU), about 20 KB on the F411.
- Protocols: SMP, "lite" with CRC, DFU through dfu-util, and updates by the application itself through `umcub_slot_*`.
- ECDSA-P256 signature checked on every boot. In swap modes the new image is a test image until the application
  confirms it, otherwise it is reverted.
- Bootloader entry: a button at reset, `umcub_enter_bootloader()` from the application, no valid image.
- Apache-2.0; a Python utility and library for own tools.

Update flow:

1. The signed image comes over the vendor interface; the application writes it into the second slot and reboots.
2. MCUboot checks the signature and swaps the slots.
3. The new firmware checks itself (USB up, config read) and confirms; otherwise revert.
4. Recovery: KEY (PA0) held at reset or no valid image -> the bootloader comes up as CDC (driverless on every OS),
   the firmware is flashed with the umcub utility.

Draft flash layout of the F411CE (sectors 4 x 16 KB, 64 KB, 3 x 128 KB) — to be checked against the umcub defconfig
for the F411:

| Sectors | Size | Purpose |
|---|---|---|
| 0-1 | 32 KB | umcub bootloader |
| 2-3 | 32 KB | Settings (two sectors alternately) |
| 4 | 64 KB | Reserve |
| 5 | 128 KB | Primary application slot |
| 6 | 128 KB | Secondary slot (update) |
| 7 | 128 KB | Scratch for swap |

Erasing a 128 KB sector on the F4 stalls the CPU for about a second: the strips go dark during an update, the
utility waits with a long timeout. On the F411CC (256 KB) there is hardly room for swap with revert.

The MVP is flashed without umcub: the F411 ROM DFU bootloader (BOOT0) + dfu-util, or ST-LINK, at 0x08000000. The
linker script takes the address as a parameter from the start, so moving to the umcub slot is one setting (or via
`umcub_app_img0_slot0.ld`).
