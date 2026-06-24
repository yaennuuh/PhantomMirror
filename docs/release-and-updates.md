# Release And Updates

Phantom Mirror uses portable ZIP releases. An installer is optional.

This document describes the technical updater flow. The repository-wide policy
for official releases, third-party builds, and checksums is documented in
`RELEASES.md`.

## Release Asset

The in-app updater expects this GitHub release asset name:

```text
Phantom-Mirror-windows-x64.zip
```

The ZIP is produced by:

```powershell
cmake --build build --config Release
.\scripts\package-windows.ps1 -BuildDir build -Config Release
```

The packaging script can also emit SHA256 checksums for release verification.
Official release automation uploads both the ZIP and `SHA256SUMS.txt`.

## Update Repository

Official GitHub release builds should set the update repository to the same
repository that publishes the release. The bundled GitHub Actions workflow does
that automatically with:

```text
PHANTOM_MIRROR_UPDATE_REPO=${{ github.repository }}
```

For custom or local builds, you can point the app at any GitHub owner/repo that
publishes the expected release asset. Example:

```text
https://api.github.com/repos/owner/repo/releases/latest
```

Override this at configure time:

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DPHANTOM_MIRROR_UPDATE_REPO=owner/repo
```

Local builds now default to no configured update repository. That avoids
accidentally pointing unofficial or pre-release builds at the official release
channel.

## Versioning

The app compares its compiled version with the latest GitHub release tag.
Release tags should use normal semantic versions:

```text
v0.1.0
v0.2.0
v1.0.0
```

For local release builds, pass the app version explicitly:

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DPHANTOM_MIRROR_APP_VERSION=0.2.0
```

The GitHub Actions release workflow derives `PHANTOM_MIRROR_APP_VERSION` from the
tag automatically.

You can also run the `release-build` workflow manually in GitHub Actions and
provide the semantic version there. The workflow then creates or updates the
matching release tag and uploads the artifacts.

## Install Flow

When a newer release is available, Phantom Mirror downloads the ZIP to the user's temp
directory, writes a temporary PowerShell script, exits, lets the script replace
the files in the app directory, then starts `Phantom Mirror.exe` again.

This means users can install Phantom Mirror by extracting the ZIP anywhere writable.
If they extract it into `Program Files`, updates may require elevated write
permissions.
