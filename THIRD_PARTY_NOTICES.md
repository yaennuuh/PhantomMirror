# Third-Party Notices

Phantom Mirror is source-available under the repository root `LICENSE.md` for the
original project code only. Third-party components remain under their own
licenses.

## Included in this repository

### Spout2

- Location: `third_party/Spout2`
- Upstream license: BSD 2-Clause
- Local license file: `third_party/Spout2/LICENSE`

Phantom Mirror builds against vendored Spout2 source files from this directory.
Only the third-party code inside `third_party/Spout2` is covered by the Spout2
license; the repository root `LICENSE.md` does not replace or narrow that
upstream license for the Spout2 code itself.

## Runtime dependencies

### Qt 6

- Used via shared libraries deployed next to the app on Windows
- License model used here: LGPL for the Qt modules consumed by this app
- Packaging note: keep Qt dynamically linked and include the required notices

Relevant project files:

- `docs/qt-lgpl-compliance.md`
- `scripts/package-windows.ps1`

### NDI Runtime

- Runtime library: `Processing.NDI.Lib.x64.dll`
- Loaded dynamically at runtime by `src/native/ndi_sdk.cpp`
- Not covered by `LICENSE.md`

NDI redistribution is subject to NDI's own terms. This repository is configured
to work without bundling the NDI runtime by default. If you choose to
redistribute NDI binaries in releases, you must review and satisfy the current
NDI redistribution requirements.

Official references:

- https://docs.ndi.video/all/developing-with-ndi/sdk/software-distribution
- https://docs.ndi.video/all/developing-with-ndi/sdk/dynamic-loading-of-ndi-libraries

## Repository policy

- Do not describe this repository as Open Source.
- Do not relabel third-party code under the repository root license.
- Keep upstream license files in place.
- Keep release documentation and official-release rules aligned with
  `LICENSE.md`, `BRANDING.md`, and `RELEASES.md`.
