# QAccelPlot Licensing

QAccelPlot is offered under a public open-source license or, alternatively,
under a separately negotiated commercial license. You may choose the option
that applies to your use.

This document summarizes the licensing options. The full open-source license
texts, or your commercial license agreement if you have one, define your actual
rights and obligations.

## Open-source option

The public source files are licensed under:

```text
GPL-3.0-only WITH Universal-FOSS-exception-1.0
```

The base license is the GNU General Public License version 3 only. The
Universal FOSS Exception is an additional permission under section 7 of
GPLv3. It permits QAccelPlot to be statically or dynamically linked with other
software distributed with complete corresponding source under a license that
is approved by the Open Source Initiative (OSI) or categorized as free by the
Free Software Foundation (FSF).

When the exception applies:

- Independent portions of the qualifying FOSS application may remain under
  their own open-source license.
- QAccelPlot, its source code, and modifications to QAccelPlot remain governed
  by QAccelPlot's GPLv3 terms and the applicable exception.
- Copyright, license, source-availability, and notice obligations must still be
  satisfied.

The exact, controlling texts are:

- [`LICENSES/GPL-3.0-only.txt`](LICENSES/GPL-3.0-only.txt)
- [`LICENSES/Universal-FOSS-exception-1.0.txt`](LICENSES/Universal-FOSS-exception-1.0.txt)

The root [`LICENSE`](LICENSE) is an additional copy of GPLv3 provided for
GitHub license detection and conventional source distributions.

## Commercial option

A separate commercial license is intended for organizations or individuals
who want to distribute QAccelPlot as part of a proprietary or closed-source
product without relying on and complying with the open-source terms.

Commercial rights are granted only through a separate agreement with the
copyright holder. See
[`COMMERCIAL-LICENSING.md`](COMMERCIAL-LICENSING.md) for contact information.

## Common situations

| Situation | Applicable route |
| --- | --- |
| A GPLv3 application distributes QAccelPlot | GPLv3 |
| An LGPL, MIT, BSD, Apache, or other qualifying FOSS application distributes QAccelPlot | GPLv3 with the Universal FOSS Exception |
| A commercial business sells a qualifying open-source application containing QAccelPlot | The open-source route remains available if all its conditions are met |
| A proprietary application or device is distributed with QAccelPlot | Obtain a commercial license |
| QAccelPlot is evaluated or used privately without distribution | GPLv3 generally permits private use without a source-publication obligation |
| QAccelPlot runs only on a server and is not distributed to users | GPLv3 does not contain the network-use provision found in the AGPL |

Whether two components form a combined or derivative work can depend on the
technical and legal circumstances. If your situation is unclear, contact the
project before distributing your product.

## Qt is licensed separately

QAccelPlot depends on Qt, but QAccelPlot's licenses grant no rights to Qt.
Using either the open-source or commercial QAccelPlot option does not replace
the need to comply with the LGPL, GPL, commercial, or other terms applicable to
the Qt components you use.

## Copyright and contributions

Copyright in QAccelPlot is held by Michal Grabarczyk unless a file states
otherwise. QAccelPlot does not accept external code contributions, preserving
the copyright holder's ability to offer both the open-source and commercial
licensing options. Bug reports, feature requests, and other feedback remain
welcome through GitHub Issues.
