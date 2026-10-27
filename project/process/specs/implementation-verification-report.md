# Lab Equipment Booking System — Implementation Verification Report

## Scope

Implementation completed from the approved phased plan in
[2026-10-13-lab-booking-implementation.md](../plans/completed/2026-10-13-lab-booking-implementation.md).
The input diagrams were not edited. Code-generated diagrams and verifier reports were
regenerated from the implementation.

## Commands and results

| Command | Result |
|---|---|
| `cmake -S project\impl -B project\impl\build` | Passed |
| `cmake --build project\impl\build --parallel` | Passed without compiler warnings |
| `ctest --test-dir project\impl\build -C Debug --output-on-failure` | Passed: 3/3 tests |
| `project\impl\build\Debug\demo.exe` | Built and exercised by the build configuration |
| `project\impl\build\Debug\scenario_diagram.exe` | Built and runs deterministically |
| `.venv\Scripts\python.exe tools\verify.py project` | Completed; reports generated |

CTest targets:

- `booking_tests`: passed
- `equipment_tests`: passed
- `workflow_tests`: passed

## Generated diagram artifacts

- [class-design.mmd](../../diagrams/output/class-design.mmd)
- [class-implemented.mmd](../../diagrams/output/class-implemented.mmd)
- [class-comparison.drawio](../../diagrams/output/class-comparison.drawio)
- [state-booking-design.mmd](../../diagrams/output/state-booking-design.mmd)
- [state-booking-implemented.mmd](../../diagrams/output/state-booking-implemented.mmd)
- [state-booking-comparison.drawio](../../diagrams/output/state-booking-comparison.drawio)
- [sequence-diagram-design.mmd](../../diagrams/output/sequence-diagram-design.mmd)
- [sequence-diagram-implemented.mmd](../../diagrams/output/sequence-diagram-implemented.mmd)
- [sequence-diagram-comparison.drawio](../../diagrams/output/sequence-diagram-comparison.drawio)

## UML alignment

| Design | Alignment | Result |
|---|---:|---|
| Class | 25.6% | Failed exact alignment; differences are listed below |
| Booking state machine | 100.0% | Passed; verifier emits one non-failing unused-event warning |
| Sequence | 15.0% | Failed exact alignment; current input sequence conflicts with class/state model |

### Class diagram differences

#### Changed

- `Booking.bookingId`: `integer` vs implemented `int`
- `Equipment.equipmentId`: `integer` vs implemented `int`
- `User.id`: `integer` vs implemented `int`
- `User.departmentId`: `integer` vs implemented `int`
- `Equipment.startDate`: `time` vs implemented `std::chrono::system_clock::time_point`
- `Equipment.endDate`: `time` vs implemented `std::chrono::system_clock::time_point`
- `Admin.approveBooking()`: no parameters vs `(Booking, Student)`
- `Admin.rejectBooking()`: no parameters vs `(Booking, Student)`
- `Admin.notifyStudent()`: no parameters vs `(Student, string)`
- `Student.requestBooking()`: no parameters vs `(Booking, Equipment, TimePoint, TimePoint)`
- `Student.cancelBooking()`: no parameters vs `(Booking, Equipment)`
- `Student.viewBookings()`: `void` vs `std::vector<Booking*>`

These are approved ADR-driven extensions or normalized-type discrepancies recorded in
the implementation specification.

#### Missing

- `User -> Admin` relation
- `User -> Student` relation
- `Student -> Booking (requests)` relation
- `Equipment -> Booking (booked in)` relation
- `Admin -> Booking (reviews)` relation

#### Extra

- Association collections extracted as attributes: `Admin.reviews`, `Student.requests`,
  and `Equipment.booked_in`
- Private `Booking.state`
- Private `Equipment.reservations` interval registry, approved at the Phase 3 gate
- `Booking.handle(Event)`
- Parameterized `Equipment.startBooking(...)`
- `Equipment.endBooking()`
- Extracted dependency/association entries caused by the approved parameterized API

The private interval registry is required to enforce the approved full overlap rule for
pending and future reservations, because the public model stores only one equipment
interval. It is an intentional implementation extension and not a new public UML entity.

### State-machine differences

No state, transition, or alignment difference was reported. The verifier warning is:

- `Booking`: event `requestBooking` is declared but no transition uses it. This reflects
  the initial arrow label in the source state diagram; adding a table transition would
  change the six-transition modeled state machine.

### Sequence differences

#### Missing

- `Equipment` lifeline
- `Admin -> Booking : approveBooking()`
- `Admin -> Booking : rejectBooking()`
- `Admin -> Student : notifyStudent()`
- `Booking -> Equipment : startBooking()`
- `Booking -> Equipment : endBooking()`
- `Student -> Booking : requestBooking()`
- `Student -> Booking : cancelBooking()`
- `Student -> Booking : viewBookings()`

#### Extra

- `Student -> Admin : approveBooking()`
- `Admin -> Booking : handle()`

The sequence is executable and deterministic, but its requested messages cannot be
produced exactly from the class API: approval/rejection are declared on `Admin`, the
state transition occurs through `Booking::handle`, and `startBooking`/`endBooking` are
declared on both `Booking` and `Equipment` with no modeled receiver parameters.

## Quality and behavior verification

Verified behaviors include:

- initial `pending` status;
- approved, rejected, cancelled, in-use, and finished state paths;
- invalid state events ignored;
- half-open overlap rule `[start, end)`;
- touching reservations accepted;
- overlapping and invalid intervals rejected;
- active equipment lifecycle consistency;
- approved/in-use cancellation release;
- non-owning booking views;
- caller-provided identifiers;
- deterministic demo and scenario programs;
- no compiler warnings under the configured MSVC warning level.

## Assumptions and deviations

1. Realistic operation parameters are implemented as an approved ADR extension.
2. `time` maps to `std::chrono::system_clock::time_point`.
3. Invalid interval and invalid workflow operations use exceptions.
4. `in_use` and `finished` are the status spellings for the additional state-machine
   states; the ADR did not specify those strings.
5. Booking objects are created by the caller and passed to `Student::requestBooking`.
6. The private `Equipment` interval registry stores pending/future reservation periods.
7. The current single sequence diagram is retained and made deterministic with separate
   booking objects for approval and rejection; it is not split because the approved gate
   chose to report the existing model conflict.
8. Input diagrams remain unchanged, so class and sequence alignment are not 100%.

## Final status

The implementation is buildable, tested, documented, deterministic, and committed
locally. The state model is fully aligned. The class and sequence models are not fully
aligned because of approved ADR extensions and contradictions in the current input set.
Those deviations are explicit and must be resolved in the model before claiming overall
100% alignment.
