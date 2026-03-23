# Code Signing Policy

## Overview

InstaDrums uses free code signing provided by [SignPath Foundation](https://signpath.org/) via [SignPath.io](https://signpath.io/).

## Signing Certificate

- **Certificate provider:** SignPath Foundation
- **Signing service:** SignPath.io
- **Certificate type:** Open Source Code Signing (EV equivalent for Windows)

## Build Process

All signed binaries are built exclusively through GitHub Actions CI:
- Source code: https://github.com/hariel1985/InstaDrums
- Build workflow: `.github/workflows/build.yml`
- No binaries are signed outside of the CI pipeline

## Team Roles

| Role | Responsibility |
|------|---------------|
| **Author** | Writes and submits code via pull requests |
| **Reviewer** | Reviews pull requests before merge |
| **Approver** | Approves signing requests on SignPath.io |

## Privacy

InstaDrums does not collect, transmit, or store any user data. The plugin operates entirely offline and does not make any network connections.

## Transparency

- All source code is publicly available under GPL-3.0
- Build artifacts are generated from the public CI pipeline
- Signing requests are logged and auditable via SignPath.io
