# Power and brightness limit

## Power regulator

Brightness is limited by the strictest of four levels: source, hardware, strip, user. Reduction takes effect in the
same frame; recovery is smooth.

| Level | Comes from | Example |
|---|---|---|
| Source | PD contract over I2C (voltage x current) or the Type-C current on the PC port; multiplied by efficiency, minus the logic consumption | A 100 W charger with a 3 A cable gives 60 W |
| Hardware | Limit of the converter, connector or feed point of each output, temperature | Reduction when the converter heats up |
| Strip | Strip profile: mA per channel, quiescent current per LED, LED count | APA102 ≈ 20 mA per channel |
| User | Maximum brightness in %, limit in watts, brightness per output | "No more than 30 W", night mode |

Algorithm:

1. Per frame, estimate the current from the strip model after gamma:
   I = N x I_quiescent + sum over channels (value / 255 x I_channel).
2. If I is above the budget, scale all channels by one factor so hues do not shift.
3. Reduction is instant, recovery takes 0.3-1 s so the brightness does not "breathe".
4. Charger unplugged or PD renegotiated: the budget is recomputed immediately. Brightness drops in advance, on a
   voltage sag of the port B bus.
5. The PC is told which limit is active (e.g. "limited by the charger: 60 W").

An INA226 current sensor on the strip bus adds self-calibration of the model, hard protection by the measured value
and automatic strip length detection (LEDs light up one by one while the current grows). The user still selects
the strip type: a strip cannot be identified electrically.

In the MVP (no PD) the budget is a constant in the settings (the current of the 5 V PSU).

## Supply and PD

At full white an LED draws about 0.3 W (WS2812B, APA102) or ~0.18 W (WS2815), so the PD power sets the limit. Regular
PD gives at most 100 W (20 V x 5 A, needs a 5 A cable with an e-marker; with a 3 A cable 60 W).

| Revision | Bus current at ~90 W | LEDs at full white from 100 W |
|---|---|---|
| 5 V (WS2812B, APA102) | ~18 A | ~300 |
| 12 V (WS2815) | ~7.5 A | ~500 |

Port roles and the power stage:

- The PC port gives at most 5 V x 3 A = 15 W. So A carries data and logic, B the strips. A power multiplexer with
  priority to B. Without B the 5 V revision lights with a brightness limit; on the 12 V revision the strips are off.
- The PD controller is driven over I2C, not by resistors: the MCU sees what the charger offers, takes the most
  powerful suitable option and knows its budget.
- The 12 V revision requests 15-20 V and steps down to 12 V: chargers rarely offer a fixed 12 V.
- 5 V at 18 A: a synchronous buck converter with external transistors, connectors rated 5 A and more per output (not
  3 A JST class), 5-7 W of heat in the converter. At 5 V each side is fed separately.
- Soft start on connection so the charger protection does not trip.

| Controller | Where | Feature |
|---|---|---|
| STUSB4500 | Port A (data) | Works on CC only, can run standalone, the contract is readable over I2C |
| HUSB238 | Port B; careful on A | Driven over I2C; has D+/D- pins for BC1.2 and the Apple divider |
| HUSB238A | Port B if EPR (28-48 V) is needed | EPR-capable sink |
| CH224K | Only a port without data | The K/D revisions support legacy protocols over D+/D- |

For the current setup, 39 W at full white fit into 60 W as well. EPR is needed only for large TVs on a 5 V strip.

## References

- [HUSB238 (Adafruit)](https://learn.adafruit.com/products/5807/guides)
- [CH224: versions and D+/D-](https://done.land/components/power/powersupplies/usb/usbtriggers/ch224/)
- [STUSB4500 standalone sink](https://www.futureelectronics.com/ftm/power-solutions/stmicroelectronics-stusb4500l-usb-type-c-controller/)
- [WS2815 consumption](https://auschristmaslighting.com/goto/post?id=125815)
