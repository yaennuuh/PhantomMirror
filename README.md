# Phantom Mirror

Current version: `0.1.0`

License: `Source-available (restricted)`

Phantom Mirror is a Windows desktop overlay app built with **C++17 + Qt Widgets**.
It creates a fullscreen, transparent, click-through overlay and renders either a
local **Spout2** sender or a network **NDI** source on top of the desktop.

## Features

- Fullscreen transparent overlay on a selectable monitor
- Click-through window behavior on Windows
- Native Spout2 receiver by sender name
- Native NDI receiver for sources on the same LAN
- Preserves RGBA alpha for transparent overlays
- System tray controls
- Spout2 settings and status test
- Virtual viewport sizing and anchoring

## Requirements

- Windows 10/11
- Visual Studio 2022 C++ toolchain
- CMake 3.24+
- Qt 6 Widgets
- A Spout2 sender, for example OBS Studio with `win-spout`
- NDI Runtime 5/6 for network NDI receive support

## Source availability, trust, and official releases

Phantom Mirror is source-available, but not released under a classic
OSI-conformant Open-Source license.

The code is public so users can inspect it, review how the app works, and
contribute improvements through pull requests.

Official builds and releases are published only by the maintainer. Builds from
forks, mirrors, or third parties are not official Phantom Mirror releases.

The name `Phantom Mirror`, the logo, icon, app name, UI branding, graphics, and
other project identity elements are not licensed for reuse. Please do not claim
that Phantom Mirror is a registered trademark.

Windows builds may currently be unsigned because no paid code-signing
certificate is available yet. Users should download releases only from official
GitHub Releases or other official sources named by the maintainer. When SHA256
checksums are published, verify them before running a release.

The official repository release workflow publishes the packaged ZIP together
with `SHA256SUMS.txt` in the same GitHub Release.

## Licensing

Phantom Mirror is source-available under `LICENSE.md`.

Allowed under the project license:

- inspect the source code
- build and run the project locally
- make private local modifications
- submit contributions through pull requests

Not allowed without separate written permission:

- publicly redistribute modified or unmodified builds
- republish the project as your own release
- commercially redistribute, resell, host, or rebrand it
- reuse the Phantom Mirror project identity

Third-party code and runtimes keep their own licenses:

- `third_party/Spout2`: BSD 2-Clause
- Qt 6 runtime: LGPL obligations apply to binary distribution
- NDI runtime: separate redistribution terms apply

See `LICENSE.md`, `BRANDING.md`, `CONTRIBUTING.md`, `SECURITY.md`,
`RELEASES.md`, and `THIRD_PARTY_NOTICES.md`.

## Unsigned Windows builds

Windows may show a SmartScreen or "Unknown Publisher" warning for official
releases while the project does not yet use a paid code-signing certificate.

That warning can happen because the build is unsigned. It does not, by itself,
prove that the release is harmful, and it also does not mean Windows has
established trust for the build.

Official releases are published through the project's GitHub Releases flow.
Release checksums should be verified when they are published. If a release does
not yet include a checksum file, future official releases are intended to do
so.

## Configure

Point `CMAKE_PREFIX_PATH` to your installed **MSVC Qt 6** directory. The path
must contain `lib/cmake/Qt6`, for example:

```text
C:\Qt\6.11.1\msvc2022_64
```

```bash
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DCMAKE_PREFIX_PATH=C:\Qt\6.11.1\msvc2022_64
```

If Qt is installed in a different version folder on your machine, replace
`6.11.1` with that version. On this system, `6.8.0` does not exist, which is
why CMake could not find `Qt6Config.cmake`.

You can verify the path with:

```powershell
Get-ChildItem C:\Qt\*\msvc2022_64\lib\cmake\Qt6\Qt6Config.cmake
```

If you prefer, you can point CMake directly at the package config directory:

```bash
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DQt6_DIR=C:\Qt\6.11.1\msvc2022_64\lib\cmake\Qt6
```

Create your local runtime config:

```bash
copy app-config.example.json app-config.json
```

## Run In Visual Studio / Debug

```bash
cmake --build build --config Debug
```

The raw `build\Debug\Phantom Mirror.exe` is not a standalone app bundle. If you
start it directly from Explorer without the Qt debug DLLs in `PATH`, Windows
will show errors such as missing `Qt6Widgetsd.dll`.

Use the Debug build from Visual Studio, or package a Release build for local
manual testing.

## Build Release

```bash
cmake --build build --config Release
```

## Package For Local Use

```powershell
.\scripts\package-windows.ps1 -BuildDir build -Config Release
```

The package uses `windeployqt`, ships Qt as DLLs, and includes Qt LGPL notice
files. By default, it does not bundle the NDI runtime. See
`docs/qt-lgpl-compliance.md`.

Start the packaged app with:

```powershell
& ".\package\Phantom Mirror.exe"
```

If you intentionally want to bundle an installed NDI runtime in a private/local
package, first build with NDI bundling enabled and then package with the extra
runtime copy step:

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DCMAKE_PREFIX_PATH=C:\Qt\6.11.1\msvc2022_64 -DPHANTOM_MIRROR_BUNDLE_NDI_RUNTIME=ON
cmake --build build --config Release
.\scripts\package-windows.ps1 -BuildDir build -Config Release -IncludeNdiRuntime
```

Review current NDI redistribution obligations before publishing such a package.

## Updates

Phantom Mirror checks GitHub Releases for a newer tag and downloads the release asset
named `Phantom-Mirror-windows-x64.zip`. By default it checks:

```text
the current official GitHub repository configured at build time
```

For a release build, compile the app with the released version:

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DCMAKE_PREFIX_PATH=C:\Qt\6.11.1\msvc2022_64 -DPHANTOM_MIRROR_APP_VERSION=0.1.0
cmake --build build --config Release
.\scripts\package-windows.ps1 -BuildDir build -Config Release
```

The official GitHub Actions release workflow injects the current repository
name into `PHANTOM_MIRROR_UPDATE_REPO`, so official releases can check for
updates from the same repository that publishes the release assets.

Local builds default to no update repository. If you use the updater outside
the official release workflow, configure `PHANTOM_MIRROR_UPDATE_REPO`
explicitly.

If you want the app to check another GitHub release repo:

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DCMAKE_PREFIX_PATH=C:\Qt\6.11.1\msvc2022_64 -DPHANTOM_MIRROR_UPDATE_REPO=owner/repo
```

## Documentation

- `docs/spout2-overlay.md`: current Spout2 runtime architecture and setup
- `docs/qt-lgpl-compliance.md`: Qt LGPL build and packaging guardrails
- `docs/release-and-updates.md`: GitHub release and in-app updater flow
- `docs/repository-migration.md`: move the project into a new repo with fresh history
- `RELEASES.md`: official release and checksum policy
- `.github/workflows/release-build.yml`: official automated release workflow

## Configuration

`app-config.json`:

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
    },
    "hotkey": {
      "enabled": false,
      "modifiers": [],
      "key": ""
    }
  },
  "debug": {
    "enabled": false
  }
}
```

`overlay.monitor` stores the selected display ID from Qt, typically something
like `\\\\.\\DISPLAY1` on Windows. Legacy or missing values fall back to the
current primary monitor.

`overlay.includeInScreenCapture` defaults to `false`, which keeps the overlay
out of supported Windows/OBS display capture paths. Set it to `true` if the
overlay should be visible in screen recordings.

Overlay content is always fitted proportionally into the monitor or virtual
viewport so the full image stays visible.

## OBS Setup

Phantom Mirror no longer ships or requires a Phantom Mirror OBS plugin. Use an existing
Spout2 output plugin for OBS, such as `win-spout`, enable its Spout2 output,
and set the Phantom Mirror sender name to the OBS Spout sender name.

Default expected sender:

```text
OBS Spout2 Output
```

## NDI Setup

Install the NDI Runtime or NDI Tools on the Phantom Mirror machine. The public
source repository and default package flow do not bundle the proprietary NDI
runtime. If another computer on the same network is sending an NDI source,
switch `Active input` to `NDI (network)` in Settings, refresh the source list,
and select that source.

Transparent NDI overlays are supported, but only if the sending application
actually transmits an alpha channel. If the NDI source is RGB-only, Phantom
Mirror will render it opaque and show that status in Settings.

## Tray Controls

- `Settings`: edit Spout2, NDI, and overlay settings
- `Setup Hilfe`: show the OBS/Spout2 onboarding guide
- `Start Overlay`
- `Stop Overlay`
- `Reconnect Spout`
- `Quit`
