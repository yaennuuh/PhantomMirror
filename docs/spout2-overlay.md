# Phantom Mirror Input Overlay

Version: `0.1.0`

## Overview

Phantom Mirror renders either one named Spout2 sender from the same Windows PC
or one NDI source discovered on the local network as a fullscreen, transparent,
click-through overlay. The current runtime has no effects engine, no
Streamer.bot integration, no Phantom Mirror OBS plugin, no WHIP/WebRTC receiver, and
no raw HTTP frame path beyond NDI receive.

## Runtime Flow

```text
OBS / any Spout2 sender
  publishes sender name, e.g. "OBS Spout2 Output"

Remote PC / any NDI sender
  advertises an NDI source on the LAN

Phantom Mirror native receiver
  connects to the configured Spout sender name or NDI source name
  renders a transparent topmost overlay window

Phantom Mirror Qt settings
  stores input mode, sender/source names, virtual viewport, and debug flag
  displays receiver status, source resolution, FPS, and frame count
```

## Configuration

`app-config.json` uses this shape:

```json
{
  "overlay": {
    "input": "spout",
    "monitor": "\\\\.\\DISPLAY1",
    "includeInScreenCapture": false,
    "alwaysOnTop": true,
    "clickThrough": true,
    "virtualViewport": {
      "enabled": false,
      "width": 1920,
      "height": 1080,
      "anchor": "middle-center"
    },
    "spoutReceiver": {
      "enabled": false,
      "senderName": "OBS Spout2 Output"
    },
    "ndiReceiver": {
      "enabled": false,
      "sourceName": "",
      "preferExactName": true
    }
  },
  "debug": {
    "enabled": false
  },
  "onboarding": {
    "completed": false
  }
}
```

## Input Settings

- `monitor`: target display ID resolved from the current Qt screen list. Legacy
  or missing values fall back to the primary monitor.
- `input`: selects the active transport, `spout` or `ndi`.
- `includeInScreenCapture`: when `false`, Phantom Mirror asks Windows to omit
  the overlay from supported display-capture paths. Default is `false`.
- `spoutReceiver.enabled`: starts or stops the local Spout2 receiver when Spout is active.
- `spoutReceiver.senderName`: exact Spout2 sender name to receive.
- `ndiReceiver.enabled`: starts or stops the network NDI receiver when NDI is active.
- `ndiReceiver.sourceName`: advertised NDI source name to receive.
- `ndiReceiver.preferExactName`: match the configured NDI source name exactly.

## OBS Setup

Phantom Mirror does not install an OBS plugin. Install or enable an existing Spout2
output plugin in OBS, such as `win-spout`, then set Phantom Mirror's sender name to
the OBS sender name.

New users see an onboarding dialog on first launch. It explains the OBS Spout2
setup, how to copy the sender name into Phantom Mirror, and how to test the receiver.
The same guide is available later from the tray menu via `Setup Hilfe`.

Default expected sender:

```text
OBS Spout2 Output
```

For remote-LAN receive, switch Settings to `NDI (network)` and select a discovered
NDI source from another computer on the same network.

NDI transparency is preserved when the sender provides an alpha-capable stream.
If the source does not include alpha, Phantom Mirror renders it opaque and reports
that in the receiver status.

## Native Implementation

- Qt Widgets owns the tray menu, settings dialog, JSON config, and lifecycle.
- The native Spout bridge is built from `src/native/spout_overlay.cpp`.
- The native NDI bridge is built from `src/native/ndi_overlay.cpp`.
- Spout2 SDK sources are vendored under `third_party/Spout2`.
- NDI is loaded dynamically from an installed runtime and packaged beside the app.
- The overlay preserves alpha and applies the configured virtual viewport and
  fit mode.
- The overlay windows request `WDA_EXCLUDEFROMCAPTURE` via
  `SetWindowDisplayAffinity` so OBS/Windows display capture should omit them
  while they remain visible on the physical monitor. Older Windows builds or
  capture paths can ignore this, so this is best-effort protection.

## Validation

Use:

```bash
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DCMAKE_PREFIX_PATH=C:\Qt\6.8.0\msvc2022_64
cmake --build build --config Release
```
