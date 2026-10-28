# Implementation guide

This project contains the micro:bit v2 implementation of the lab booking mock-up.

- [`impl/`](impl/) contains the C++ domain model, unit tests, behavior probe, and benchmark target.
- [`verification/`](verification/) contains the standalone CTest runner, benchmark,
  ADR-based verifier, and generated JSON results.
- [`dashboard/`](dashboard/) is a browser dashboard using localStorage and WebSerial.
- [`firmware/`](firmware/) contains the micro:bit v2 firmware source, build script,
  CODAL configuration, and generated `MICROBIT.hex`.

Serve the dashboard with a local HTTP server:

```powershell
python -m http.server 8000 --directory ModelDrivenDesignSkoleOblgi2/project/dashboard
```

Open `http://localhost:8000` in Chrome or Edge. A real WebSerial connection
requires the board; the dashboard remains usable for creating bookings and
viewing generated quality results without hardware.
