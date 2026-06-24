# Security Policy

If you discover a security issue, please do not post exploit details publicly
in a GitHub issue if the report would make the issue easier to abuse.

Please contact the maintainer privately instead:

- yaennuuh@gmail.com

When reporting a security issue, include:

- affected version or commit
- reproduction steps
- impact assessment
- logs, screenshots, or proof of concept when safe to share privately

## Unsigned Windows builds

Windows releases may currently be unsigned if no paid code-signing certificate
is available.

That can cause Windows SmartScreen or "Unknown Publisher" warnings. A missing
signature does not by itself mean the application is malicious, but it also does
not create trust automatically.

Users should download releases only from official release sources and, when
available, verify published SHA256 checksums before running them.
