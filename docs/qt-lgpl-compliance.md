# Qt LGPL Compliance

Phantom Mirror is intended to remain source-available while using Qt under the
LGPL terms. This repository therefore uses the following guardrails:

- Only `Qt6::Widgets` is linked by the application. That pulls in Qt Core and
  Qt Gui as shared Qt runtime dependencies.
- CMake rejects known static Qt builds via `QT6_IS_SHARED_LIBS`,
  `QT_FEATURE_shared`, or a static `Qt6::Core` target.
- Windows packages are produced through `windeployqt`, so Qt is shipped as
  replaceable DLLs next to `Phantom Mirror.exe`.
- The package script writes `licenses/QT-LGPL-NOTICE.txt` and copies Qt license
  files from the local Qt installation when they are available.
- Do not add GPL-only Qt modules unless the app itself is relicensed
  accordingly or a commercial Qt license is used.

For binary distribution, keep these constraints:

```cmake
find_package(Qt6 REQUIRED COMPONENTS Widgets)
target_link_libraries(PhantomMirror PRIVATE Qt6::Widgets)
```

Do not use static Qt builds and do not add `BUILD_SHARED_LIBS=OFF` for Qt.

Package with:

```powershell
.\scripts\package-windows.ps1 -BuildDir build -Config Release
```

This is a technical compliance setup, not legal advice.
