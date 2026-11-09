# PC and Android hosts

On Windows and Linux the main host is HyperHDR over CDC and AWA; the own utility is needed only for configuration and
updates. Android is the next stage.

## HyperHDR settings for the device

- LED device Adalight, AWA protocol on, handshake on (or force detection), refresh time 0, any speed.
- Color order RGB: the device applies the strip order.
- Smoothing off or minimal — it adds latency.
- Capture: DirectX on Windows, PipeWire on Linux. Check whether the grabber holds 180 frames/s.

Other programs work too through Adalight: [Hyperion.ng](https://www.github.com/hyperion-project/hyperion.ng/releases)
(active, supports AWA), [Prismatik](https://github.com/psieg/Lightpack) (last release 2022, GPL-3), AmbiBox (not
developed). Own utility: a prototype in Python + pyusb; for recovery flashing, the umcub Python library.

## Android boxes and TVs

One application for boxes and TVs: the Android TV API is the same. The difference is the hardware: on some TVs video
is a separate hardware layer and is captured black, cheap TVs cannot do 4K and capture at once, TV USB ports are
weaker. Capture on Android runs at 10-60 frames/s; the 180 Hz target is out of reach there.

| Option | How | Limits |
|---|---|---|
| Grabber -> network -> HyperHDR | Ready app [Hyperion Android Grabber](https://github.com/evanwhitt/hyperion-android-reborn/wiki); HyperHDR on a Raspberry Pi or PC sends frames to the device over USB | Works without our code; needs an extra computer |
| Own Android app | Capture via MediaProjection, sending via the USB host API to the vendor interface; the device plugs straight into the box | The app has to be written; the same capture limits |
| HDMI capture | HDMI splitter + USB capture card + HyperHDR on a Raspberry Pi | The only path for Netflix and other DRM apps ([discussion](https://techenclave.com/t/ambient-lighting-for-android-tv/266768)) |

Groundwork already now: the protocol does not depend on the host, the vendor interface is reachable from Android
without drivers, VID/PID are fixed (device filter for auto-starting the app).

Later: [HID LampArray](https://learn.microsoft.com/windows/uwp/devices-sensors/lighting-dynamic-lamparray) for
Windows Dynamic Lighting.
