# Releases

Only the maintainer publishes official Phantom Mirror releases.

## Official release channel

Official releases are published through GitHub Releases for this project and
other official sources referenced in `README.md`.

Artifacts from mirrors, forks, random file hosts, or third-party packaging
sites are not official Phantom Mirror releases.

## Release rules

- Official releases should be published through GitHub Releases.
- The repository release workflow is the authoritative packaging path for
  official releases.
- Release artifacts should not be manually redistributed from unknown sources.
- Third-party builds must not be presented as official Phantom Mirror releases.
- Unsigned Windows builds should be clearly labeled as unsigned.

## Checksums

Each release should publish a SHA256 hash for the ZIP and/or EXE artifacts.

When available, users should verify the published checksums before running a
downloaded release.

If code signing is added later, release notes should also document how to
verify the signature.

## Recommended maintainer flow

1. Merge release-ready changes into `main`.
2. Create and push a semantic-version tag such as `v0.1.0`.
3. Let the GitHub Actions release workflow build the Windows package.
4. Verify that the release contains the ZIP and `SHA256SUMS.txt`.
5. Confirm the release notes clearly state whether the build is unsigned.
