# Lab Equipment Booking System — Implementation and Quality Specification

## Purpose and authority

This specification is the implementation contract for the C++ mock-up. It is based on:

- `project/README.md` (ADR)
- `project/diagrams/input/class.drawio`
- `project/diagrams/input/state-booking.drawio`
- `project/diagrams/input/sequence-diagram.drawio`
- `project/process/specs/ground-truth-research.md`
- `tools/umlverify/docs/UML-CPP-MAPPING.md`

The diagrams are mechanically authoritative for the class surface, relationships, state
machine structure, and sequence-message shape. The ADR supplies behavioral context. No
implementation may silently add classes, public members, relations, state events, or
cross-class calls that are not authorized below.

The implementation must be written only under `project/impl/`. This specification is
documentation and does not authorize changes to the input diagrams, generated diagrams,
or generated reports.

## Pre-implementation conditions and known model deviations

The current input set is not fully sufficient for a verifier-aligned implementation:

1. The class diagram omits all operation parameters and return types, while the confirmed
   ADR extension requests realistic parameters.
2. The sequence diagram is one linear sequence containing both `approveBooking()` and
   `rejectBooking()` for the same apparent booking. Those transitions are mutually
   exclusive from `Pending`.
3. The sequence diagram addresses approval and rejection to `Booking`, while the class
   diagram declares those operations on `Admin`.
4. The sequence diagram shows `Admin -> Student : notifyStudent()`, while the class
   diagram declares `notifyStudent()` on `Admin`.
5. The ADR requires equipment release during cancellation, but the state diagram does not
   draw an equipment message on cancellation.

The user confirmed that realistic ADR signatures are an explicit extension and that the
diagrams must later be updated before verifier-aligned implementation is expected. Until
that update occurs, any verifier report showing these known differences is an expected
model mismatch, not a code-quality failure. It must nevertheless be reported; it must
never be hidden by weakening tests or editing generated reports.

## Required project structure

Use exactly:

```text
project/impl/
  CMakeLists.txt
  include/lab_booking/
    user.hpp
    student.hpp
    admin.hpp
    equipment.hpp
    booking.hpp
  src/
    user.cpp              # only when implementation is required
    student.cpp
    admin.cpp
    equipment.cpp
    booking.cpp
    demo.cpp
  scenarios/
    approval.cpp          # after the sequence model is split
    rejection.cpp
```

The project namespace must be short, lowercase, and consistent, for example
`lab_booking`. File names use `snake_case`. Each class has one public header. Headers
must expose the modeled API; implementation details belong in source files.

## Class and relationship contract

### `User`

Public data members exactly as modeled:

- `id: integer`
- `departmentId: integer`
- `name: string`

`Student` and `Admin` publicly inherit from `User`. Do not add user authentication,
permissions, collections, utility methods, or lifecycle state.

### `Student`

Public operations:

- `requestBooking(...)`
- `cancelBooking(...)`
- `viewBookings()`

The confirmed ADR extension gives `requestBooking` an `Equipment` and start/end time
values, gives `cancelBooking` a `Booking`, and makes `viewBookings` return
`std::vector<Booking*>`.

The `Student -> Booking` association is a non-owning association with role `requests`
and multiplicity `0..*` at `Booking`. Use a collection capable of representing multiple
bookings without transferring ownership. `Student` must not own bookings through
`std::unique_ptr` or `std::shared_ptr`.

`requestBooking` must enforce the half-open interval rule `[start, end)`: two bookings
overlap exactly when `newStart < existingEnd && existingStart < newEnd`. A booking that
starts exactly when another ends is allowed. Invalid intervals where end is not after
start must be rejected explicitly according to the selected error policy; they must not
be accepted as a successful booking.

### `Admin`

Public operations:

- `approveBooking(...)`
- `rejectBooking(...)`
- `notifyStudent(...)`

The `Admin -> Booking` association is non-owning, role `reviews`, with `0..*` bookings
and `0..1` at the Admin end as drawn. Approval changes a pending booking to approved;
rejection changes a pending booking to rejected. Both actions invoke the sender-side
`Admin::notifyStudent(...)` behavior with the decision string.

Do not add an authorization framework, persistence layer, notification service, or
separate administrator role class.

### `Equipment`

Public data members exactly as modeled:

- `equipmentId: integer`
- `name: string`
- `inUse: bool`
- `whichBooking: Booking = null`
- `startDate: time`
- `endDate: time`

`time` maps to `std::chrono::system_clock::time_point`.
`whichBooking` is a nullable, non-owning reference to the currently active booking only.
It is not a collection and does not represent pending or future bookings.

The `Equipment -> Booking` association is non-owning, role `booked in`, and represents
multiple bookings. Equipment owns no booking lifetime.

`startBooking()` sets `inUse` to `true`, records the active booking and its interval, and
must not silently overwrite a conflicting active reservation. `endBooking()` releases
the active booking and sets `inUse` to `false`. The implementation must make the
release operation idempotent or explicitly reject an invalid release according to the
selected error policy; it must not leave contradictory `inUse` and `whichBooking`
values.

### `Booking`

Public data members exactly as modeled:

- `bookingId: integer`
- `status: string`

Public operations:

- `startBooking()`
- `endBooking()`

Do not add public `student` or `equipment` members to `Booking`; the research decision
explicitly rejected adding those members based only on the ADR.

The state machine's internal `State` value and the public `status` string must remain
synchronized. The externally visible values are:

| State | Status string |
|---|---|
| `Pending` | `pending` |
| `Approved` | `approved` |
| `Rejected` | `rejected` |
| `InUse` | `in_use` |
| `Cancelled` | `cancelled` |
| `Finished` | `finished` |

The lowercase values for `InUse` and `Finished` are an implementation convention because
the ADR names only the first four strings; this is an assumption and must be reported.

## State-machine implementation

Implement the Booking state machine as a transition table inside `Booking`:

- nested `enum class State` in drawing order:
  `Pending`, `Approved`, `Rejected`, `InUse`, `Cancelled`, `Finished`;
- nested `enum class Event` containing the distinct drawn events:
  `requestBooking`, `approveBooking`, `rejectBooking`, `cancelBooking`,
  `startBooking`, `endBooking`;
- one `Transition` row for every drawn arrow, including rows to the terminal notation
  if the verifier requires them;
- initial state set to `Pending`;
- `handle(Event)` walks the table and ignores an event for which no row exists;
- no guard or action is invented where the drawing has none.

The unlabeled transitions from `Rejected`, `Cancelled`, and `Finished` to the final node
are terminal notation only. No event or public operation is added for them. If the
verifier cannot represent a no-event terminal edge, record that limitation in the report
instead of inventing an event.

The public operations must route state changes through the same transition mechanism.
No operation may mutate `status` independently of `State`.

## Behavioral rules

1. A new booking begins in `Pending`.
2. Only `Pending` may be approved or rejected.
3. Only `Approved` may start use.
4. `InUse` may finish through `endBooking()`.
5. `Pending`, `Approved`, and `InUse` may be cancelled only where the state diagram
   explicitly shows that transition.
6. Cancelling an approved or in-use booking also calls `Equipment::endBooking()` and
   releases the equipment.
7. A rejected, cancelled, or finished booking is terminal.
8. Approval and rejection notify the student through `Admin::notifyStudent(...)`.
9. Overlap validation occurs in `Student::requestBooking(...)` using `[start, end)`.
10. IDs are supplied by callers; no hidden generator or global counter is introduced.

## Error and invalid-operation policy

The current ground truth does not define exception types, return-status types, or logging.
This is an implementation assumption that must be selected before coding.

Until explicitly resolved, use a small, documented policy consistently:

- invalid state events are no-ops only when that is the state-machine contract;
- invalid input intervals and invalid ownership/association operations must produce an
  explicit failure result or exception;
- do not use broad `catch (...)`, silent fallback values, or success-shaped error returns;
- do not print diagnostics from library methods unless the project establishes a logging
  policy;
- demo and scenario programs must make deterministic, valid calls.

The chosen concrete policy, including whether failures use `std::optional`, `bool`,
`std::expected`-like behavior, or exceptions, must be recorded in the implementation
documentation and in the final verification report.

## C++ and build standards

Use C++20 with:

- `CMAKE_EXPORT_COMPILE_COMMANDS ON`;
- a static library containing class sources;
- `include` as the public include directory;
- warnings enabled at least with `-Wall -Wextra` (or the MSVC-equivalent flags);
- a small `demo` executable;
- one executable per finalized sequence scenario, linked to the library.

Headers must include what they use and compile independently. Use forward declarations
only where they preserve a clear dependency direction and are valid for the chosen
member types. Avoid circular includes through unnecessary value members.

Use RAII, const-correctness, explicit ownership semantics, and standard-library types.
Do not use raw owning pointers, global mutable state, macros for business behavior,
`using namespace`, unexplained numeric constants, or compiler-specific extensions.

## Readability and documentation quality

The code must be maintainable by another C++ developer:

- names must describe domain concepts and follow one project-wide naming convention;
- methods should be short enough to make their state changes and failure behavior clear;
- each public class and non-obvious public operation needs a concise documentation comment
  stating its responsibility and relevant preconditions/postconditions;
- comments must explain domain invariants or non-obvious state-table decisions, not narrate
  obvious syntax;
- state names, event names, transition rows, and status conversions must be easy to
  compare with the diagrams;
- avoid duplicated transition logic and duplicated overlap predicates;
- keep headers focused on declarations and contracts; keep behavior in `.cpp` files;
- avoid speculative abstractions, framework dependencies, persistence, networking, UI,
  threads, clocks used for current-time reads, randomness, and input;
- scenario programs must be deterministic and must make exactly their modeled
  cross-class calls.

## Verification specification

After each coherent implementation change:

1. Run the project verifier using the repository's documented command:
   `.venv/bin/python tools/verify.py project` (or the equivalent configured environment).
2. Read every generated report under `project/reports/`.
3. Build the CMake project with warnings enabled.
4. Run all unit tests available for the changed behavior and all scenario executables.
5. Confirm that no compiler warning, test failure, verifier `Changed`, `Missing`, or
   `Extra` entry is ignored.

Required behavioral tests include:

- initial `Pending` status;
- valid approval and rejection paths;
- invalid mutually exclusive transitions;
- cancellation from each drawn cancellable state;
- release of equipment after approved/in-use cancellation;
- start/end equipment lifecycle and consistency of `inUse`/`whichBooking`;
- half-open boundary acceptance;
- actual overlap rejection;
- caller-provided ID preservation;
- status/state synchronization;
- deterministic notification behavior;
- `viewBookings()` returning the expected non-owning booking references.

The final report must include:

- exact build and test commands;
- pass/fail result for each command;
- verifier alignment percentage and every report entry;
- known mismatches caused by the current diagrams;
- assumptions made because the ADR or diagrams omitted a detail;
- any remaining blocker that requires a model or diagram update.

No report may be edited manually. No failing test, warning, or model mismatch may be
described as passing through a workaround.

## Assumptions not captured by the current input

These assumptions are explicitly outside the current ADR/diagram theory and must be
confirmed or recorded before implementation is considered complete:

1. The exact C++ signatures and return/error types for all operations.
2. The `InUse` and `Finished` string spellings.
3. The concrete representation of the three association collections and their mutation
   API, since role names and multiplicities do not specify storage operations.
4. Whether invalid interval/state/association operations throw or return failure values.
5. Whether `Equipment::startBooking()` receives a booking reference and interval directly,
   since its diagram signature is empty.
6. Whether a booking is created externally or by `Student::requestBooking(...)`.
7. Whether booking collections are shared through a repository-like object or held directly
   by each association owner; no repository is modeled.
8. Whether `Admin::notifyStudent(...)` records a notification, prints one, or only models
   the call; no notification output contract is drawn.
9. How the current linear sequence is split into finalized approval and rejection diagrams.
10. Whether the current diagrams will be updated before implementation, or whether the
    known signature and sequence mismatches are accepted as intentional extensions.

These assumptions must not be resolved by silently adding domain entities or public API.
If they remain unresolved, stop before implementation and report them as blockers.
