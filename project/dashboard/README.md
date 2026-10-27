# Lab Booking dashboard

The browser page is a small mock-up for exploring bookings and the host verification
results. It uses no build step or external JavaScript package.

## Run it

1. Generate a C++ verification report from the repository root:

   ```powershell
   python tools\adrverify\verify.py
   ```

2. Start a local static server from the repository root:

   ```powershell
   python -m http.server 8000
   ```

3. Open `http://localhost:8000/project/dashboard/`. WebSerial requires HTTPS or localhost
   in Chrome or Edge; the page and its localStorage event log can otherwise be explored
   without connecting the board.

## Behavior

- Bookings, workflow status changes, and activity events persist in browser localStorage.
- The dashboard can create, approve, reject, start, finish, and cancel mock bookings
  subject to the modeled lifecycle.
- A connected micro:bit v2 mirrors the selected booking's initial status (`P`, `A`, `U`,
  `F`, `C`, or `R`) and navigates records with buttons A (previous) and B (next).
- The page graphs verifier FoM and host benchmark values from the JSON report produced by
  `tools/adrverify/verify.py`. Timing values are not estimates of micro:bit speed.

The initial dataset is deliberately small and fixed in scope. It is not a production
booking service, calendar, authentication system, or multi-user database.
