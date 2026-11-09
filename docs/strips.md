# Strip output

Each output is a separate SPI with DMA, so the strip type is configured per output and the CPU takes no part in the
transfer. SPI1/4/5 are clocked from APB2 (96 MHz), SPI2/3 from APB1 (48 MHz); the dividers are powers of two.

## APA102 and SK9822

Frame: 4 bytes `0x00`; per LED `0xE0` + 5 bits of global brightness, then B, G, R; at the end 4 bytes `0x00` (reset
frame for the SK9822) + ceil(N/16) bytes `0x00`. This ending works with both chips. SPI mode 0, configurable color
order (BGR by default).

SK9822 is often sold as APA102. Differences that matter for the firmware:

| Property | APA102 | SK9822 |
|---|---|---|
| Data applied | Immediately on reception | At the start of the next start frame; without the reset frame the strip lags a whole frame |
| 5-bit global brightness | Extra PWM at ~580 Hz (colors 19.2 kHz); flicker possible at values < 31 | Sets the driver current, no flicker |
| Profile in the firmware | Global brightness always 31; smoothness at low levels comes from temporal dithering | 16 pipeline bits split into 5 bits of current x 8 bits of PWM; the power regulator dims by current without losing resolution |

### SPI clock for the current setup

Frame on the wire for 130 LEDs: 4 start bytes + 130 x 4 bytes + 4 reset-frame bytes + 9 end bytes = 537 bytes. SPI1
runs from 96 MHz, so 6, 12 and 24 MHz are available.

| SPI clock | Frame output, ms | Max strip updates, Hz |
|---|---|---|
| 6 MHz | 0.72 | ~1400 |
| 12 MHz (default) | 0.36 | ~2800 |
| 24 MHz | 0.18 | ~5600, risk of glitches |

12 MHz is chosen because at 24 MHz long APA102 strips usually fail after 150-250 LEDs
([PJRC](https://www.pjrc.com/category/article/page/2/)). The clock is a setting.

## Other strips

- **WS2812B, SK6812, WS2815** (one-wire). MOSI without a clock, 4 SPI bits per strip bit at 3 MHz: T0H ≈ 333 ns,
  T1H ≈ 667 ns, period 1.33 µs. About 12 bytes of RAM per RGB LED. Reset pause ≥ 280 µs of zeros; the last bit on
  MOSI must be 0.
- **HD108**. 16 bits per channel and a 5-bit gain per channel, clocked. The natural output for the 16-bit pipeline.
- The 12 V revision keeps mostly one-wire strips (WS2815, WS2811 at 12 V): clocked strips are almost all 5 V.
- **More than 5 outputs**. Parallel output TIM1 + DMA2 -> GPIO BSRR (the hubmartin library,
  [F411 fork](https://github.com/blaz-r/ws2812b_stm32f411)), up to 16 lines of one timing. F411 erratum: DMA2 may
  corrupt data when working with GPIO (AHB) and APB2 at the same time. Do not mix with SPI1/4/5 DMA, or apply the
  workaround from AN4031.

One-wire strips: how many LEDs per output fit into one monitor frame (30 µs per RGB LED + 300 µs reset; 40 µs per
RGBW LED + 80 µs). For minimal latency keep about half.

| Monitor | RGB (WS2812B, WS2815) | RGBW (SK6812) |
|---|---|---|
| 60 Hz | ~545 | ~415 |
| 144 Hz | ~220 | ~170 |
| 240 Hz | ~128 | ~100 |

## Signal levels

Buffer 74AHCT125 or 74HCT245 powered from 5 V, a ~33 Ohm series resistor in DATA and CLK, a short wire to the strip
with ground next to it.

## References

- [SK9822 vs APA102 analysis](https://hackaday.io/project/21293/instructions)
- [SK9822 vs APA102 (Pololu)](https://www.pololu.com/blog/697/new-sk9822-led-strips-and-panels)
- [APA102 PWM frequencies (periph.io)](https://pkg.go.dev/periph.io/x/periph/devices/apa102)
- [FadeCandy: interpolation and dithering](https://www.github.com/rewolff/fadecandy)
- [STM32F411 DMA2 erratum (ST forum)](https://community.st.com/s/question/0D50X00009XkgdQ/stm32f411-errata?)
