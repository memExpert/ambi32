# USB and protocols

The device is a composite USB full-speed device: CDC with Adalight/AWA for compatibility with HyperHDR, and a vendor
interface for the native protocol. No kernel drivers are needed anywhere.

- **CDC**: a virtual COM port, supported by Windows, Linux and macOS out of the box. USB FS throughput on the STM32F4
  is ~0.9 MB/s in practice; the CDC baud rate means nothing.
- **Vendor**: bulk endpoints; Windows installs WinUSB by itself from the MS OS 2.0 descriptors, Linux uses libusb + a
  udev rule. Optionally WebUSB for configuration from a browser. On Android this interface is simpler than CDC.
- **HID** does not fit for frames (~64 KB/s). HID LampArray for Windows Dynamic Lighting can be added later.

## USB IDs

For now a test PID of pid.codes under VID 0x1209; an own PID once the public repository exists. The serial number
comes from the chip UID.

## Adalight / AWA

Checked against the sources of [HyperHDR](https://github.com/awawa-dev/HyperHDR) (DriverSerialAdalight.cpp,
ProviderSerial.cpp, EspTools.h) and [HyperSerialPico](https://github.com/awawa-dev/HyperSerialPico) (main.h,
framestate.h, statistics.h).

Handshake (HyperHDR option "Esp8266/ESP32/Rp2040 handshake"):

- On open HyperHDR sends 14 bytes: `41 77 41 2A A2 15` + `hyperhdr`. This is a header with N-1 = 0x2AA2 and a
  "wrong" crc: `0x15` = welcome + statistics reset, `0x35` = statistics only.
- The device must answer within 1 s with text longer than 16 bytes containing `Awa driver`. HyperSerialPico answers
  with the statistics line and `\r\nWelcome!\r\nAwa driver 11.\r\n`.
- For unknown VID/PID (without the force-detection option) HyperHDR toggles DTR/RTS instead of this command (ESP
  reset) and waits for the same welcome. So the device also sends the welcome when the line state changes
  (`tud_cdc_line_state_cb`).
- On close HyperHDR sends the same command with `0x35` and waits for more than 32 bytes with a line feed. Line format
  as in HyperSerialPico: `HyperHDR frames: N (FPS), receiv.: N, good: N, incompl.: N\r\n`.
- HyperHDR's port auto-detection prefers known ESP and RP2040 VID/PIDs. Our port is selected manually or with force
  detection enabled.

## Native protocol v0 (draft)

No checksum is needed in the native protocol: USB bulk has a CRC and retransmission. Adalight has checksums because
of UART bridges.
