# Security Policy

## Reporting a Vulnerability

If you discover a security vulnerability in QAccelPlot, please report it
privately by emailing [qaccelplot@gmail.com](mailto:qaccelplot@gmail.com).

Include:

- A description of the vulnerability.
- Steps to reproduce it.
- The QAccelPlot version, Qt version, OS, and compiler.

Please do not open a public GitHub issue for security vulnerabilities.

## Scope

QAccelPlot is a client-side plotting library. It does not handle network
communication, authentication, or user-supplied untrusted input in typical
deployments. Security-relevant issues are most likely to involve:

- Malformed data causing crashes or undefined behavior in buffer-handling code.
- Shader or Scene Graph issues triggered by specific graphics drivers.
