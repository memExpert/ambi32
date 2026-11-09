# Hardware and MVP on the WeAct Black Pill

The MVP goal is a working ambilight with HyperHDR: CDC + AWA/Adalight with the handshake, APA102/SK9822 on SPI1 +
DMA, the "loading" effect and a constant current limit. No PD, no umcub, no native protocol.

## Current setup

Strip "APA102" (possibly SK9822 in fact), 60 LEDs/m, 32" monitor: 42 + 42 + 23 + 23 = 130 LEDs in one strip around
the screen. 15-20 cm from the board to the start of the strip. Target: at least 180 Hz. A 100 W charger is available,
but PD is postponed: the strip runs from a 5 V PSU.

Power: 130 x 60 mA ≈ 7.8 A at 5 V (about 39 W) at full white. Feed at two points at least: at the start of the strip
and in the opposite corner.

## Wiring

| Net | Goes to |
|---|---|
| PA5 (SPI1 SCK) | Strip CI through a 74AHCT125 buffer and ~33 Ohm |
| PA7 (SPI1 MOSI) | Strip DI through the buffer and ~33 Ohm |
| Board GND | PSU GND and strip GND (common ground) |
| PSU +5 V | Strip only: at the start and in the opposite corner; ~1000 µF capacitor at the strip input |
| Board supply | USB only. Never feed PSU +5 V into the board's 5V/VBUS: it back-powers the PC port |
| PA0 (KEY) | Reserved for umcub entry |
| PC13 (LED) | Frame reception indicator |

Without a buffer, 3.3 V on the inputs of a 5 V strip is marginal: then start at 6 MHz. The PSU should give at least
8-10 A for full white, otherwise set the current limit in the settings. The WeAct V3.0 has a footprint for an SPI
flash on PA4-PA7 (to be confirmed on the board): it must be empty.

## Peripheral setup

- **Clock.** HSE 25 MHz -> PLLM 25, PLLN 192, PLLP 2 -> SYSCLK 96 MHz; PLLQ 4 -> USB 48 MHz. AHB /1, APB1 /2
  (48 MHz), APB2 /1 (96 MHz). Flash 3 wait states, voltage scale 1. 100 MHz together with 48 MHz for USB is not
  possible.
- **SPI1.** Master, mode 0, transmit only (BIDIMODE + BIDIOE), 8 bits, divider /8 = 12 MHz, TXDMAEN. PA5/PA7 are AF5.
- **DMA.** SPI1_TX is DMA2 Stream 3 Channel 3 (alternative Stream 5 Channel 3; check against the RM0383 request
  table). Memory -> peripheral, MINC, transfer-complete interrupt. Two frame buffers: encode the next while the
  current one goes out.
- **USB.** PA11/PA12 are AF10, enable the OTG FS clock. In TinyUSB 0.21 VBUS sensing is off by default and the driver
  forces B-valid itself — what the WeAct needs, as it has no VBUS line on PA9. Init:
  `tusb_init(0, &(tusb_rhport_init_t){ .role = TUSB_ROLE_DEVICE, .speed = TUSB_SPEED_AUTO });`,
  `OTG_FS_IRQHandler` calls `tud_int_handler(0)`, the loop calls `tud_task()`.

## Dependencies

| Dependency | Version | What is used |
|---|---|---|
| [TinyUSB](https://github.com/hathach/tinyusb) | 0.21.0 | `src/` via `tinyusb_target_add()`; DCD `src/portable/synopsys/dwc2/dcd_dwc2.c` + `dwc2_common.c`; `CFG_TUSB_MCU=OPT_MCU_STM32F4` |
| [cmsis_device_f4](https://github.com/STMicroelectronics/cmsis_device_f4) | v2.6.11 | `stm32f411xe.h`, `startup_stm32f411xe.s`, `system_stm32f4xx.c`; define `STM32F411xE` |
| [cmsis_core](https://github.com/STMicroelectronics/cmsis_core) | v5.9.0_20250520 | `Include/core_cm4.h` and neighbours |
| [stm32f4xx_hal_driver](https://github.com/STMicroelectronics/stm32f4xx_hal_driver) | v1.8.5 | LL drivers only (no HAL) |
| [umcub](https://github.com/memExpert/umcub) | after the MVP | As a git submodule |

## Flashing the MVP

Through the ROM DFU: hold BOOT0, press NRST, then `dfu-util -a 0 -s 0x08000000:leave -D fw.bin`. For tests without
HyperHDR, a short Python + pyserial script sends AWA frames in the format from protocol.md.

## "Loading" effect

- A group ~8-12 LEDs wide, brightness profile cos² or Gaussian: maximum in the middle, falling to the edges.
- Fractional position (between LEDs) for smooth movement; one round in ~2-3 s; rendering at ~200 Hz.
- Start: no frames for 3 s, transition ~0.5 s. Exit: the first frame, without a transition.
- Low-brightness tails are stepped at 8 bits — smoothed by dithering or (on the SK9822) by the global brightness.
