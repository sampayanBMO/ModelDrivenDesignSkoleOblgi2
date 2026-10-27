# Implementation Plan: Lab Equipment Booking System

## Objective

Implement the lab equipment booking system in C++20 while preserving exact alignment
with the input UML diagrams and the ADR decisions recorded in:

- [ground-truth-research.md](../specs/ground-truth-research.md)
- [implementation-quality-specification.md](../specs/implementation-quality-specification.md)

The human remains in control through an explicit eye gate after every phase. No later
phase begins until the human approves the gate or supplies a correction. A gate approval
does not waive model alignment: every implementation change must still be checked against
the input diagrams and reported honestly.

## Non-negotiable constraints

- Never edit `project/diagrams/input/`.
- Never edit generated Mermaid designs or verifier reports.
- Write implementation code only under `project/impl/`.
- Keep process documentation under `project/process/`.
- Do not add classes, public members, relations, events, or cross-class calls that are
  not in the model or explicitly recorded as an approved ADR extension.
- Preserve the current known conflicts as tracked deviations until the human decides
  whether the diagrams are updated.
- Do not declare success while any build, test, warning, verifier, or alignment result is
  failing or unexplained.

## Phase 0 — Baseline and decision register

### Work

- Read the current specs, ADR, input diagrams, mapping rules, and relevant examples.
- Run the verifier once before implementation if the environment is available, so generated
  designs and baseline reports exist.
- Record the baseline build/verifier state and every pre-existing failure.
- Create or update a decision register containing unresolved choices from the specification:
  exact signatures, error policy, status spellings, collection representation, booking
  creation ownership, notification behavior, and scenario split.

### Exit evidence

- Baseline verifier output and reports are preserved.
- Every unresolved decision is listed with an owner and a proposed choice.
- No implementation code has been created.

### Human eye gate 0

The human confirms:

1. the source-of-truth hierarchy;
2. which ADR extensions are permitted;
3. the exact API/signature direction;
4. the error-handling policy;
5. whether the current sequence diagram will be split into approval and rejection
   scenarios before implementation.

If any answer changes the model, update the specs or input diagrams before Phase 1.

## Phase 1 — Model extraction and implementation skeleton

### Work

- Convert the class diagram into a header-level checklist:
  `User`, `Student`, `Admin`, `Equipment`, and `Booking`.
- Map inheritance, association direction, role names, multiplicities, visibility, and
  types using `UML-CPP-MAPPING.md`.
- Create the required `project/impl/` directory layout and CMake skeleton.
- Add one header per modeled class and only the declarations authorized by the approved
  decision register.
- Add minimal source files, a demo target, and finalized scenario target placeholders.
- Compile the skeleton with warnings enabled.

### Exit evidence

- CMake configures and builds without warnings.
- Header declarations can be compared line-by-line with the class model.
- Ownership and multiplicity choices are explicitly documented.
- No business behavior is implemented yet.

### Human eye gate 1

The human reviews and approves:

- all class names and namespace/file names;
- public members and signatures;
- every relationship representation;
- whether any declaration is an intentional ADR extension;
- the CMake target and include structure.

Any mismatch blocks Phase 2.

## Phase 2 — Booking state machine

### Work

- Implement `Booking::State`, `Booking::Event`, `Transition`, initial state, and the
  transition table in the exact drawing order.
- Implement `handle(Event)` as a table walk with no invented guards or actions.
- Route public booking operations through the state-machine mechanism.
- Synchronize `status` with state using the approved mapping.
- Represent terminal notation without inventing events.
- Add focused unit tests for every drawn transition and invalid event.

### Exit evidence

- All states and events correspond to the input state diagram.
- Every drawn transition is represented or explicitly reported if the verifier cannot
  encode a terminal no-event edge.
- Initial status is `pending`.
- Valid transitions pass; invalid transitions do not mutate state/status.
- Status and internal state never diverge.

### Human eye gate 2

The human reviews the transition table, status strings, terminal-state handling, and
invalid-event behavior. The human must explicitly approve any implementation convention
not visible in the state diagram.

## Phase 3 — Equipment lifecycle and booking behavior

### Gate decision

The human approved a private interval registry inside `Equipment` so the full half-open
no-overlap rule can be enforced for pending and future bookings without changing the
public UML surface. This is an intentional private implementation detail and must be
reported as an approved model-preserving implementation extension.

### Work

- Implement equipment attributes and the non-owning equipment-to-booking association.
- Implement start/end booking behavior and consistency of `inUse`, `whichBooking`,
  `startDate`, and `endDate`.
- Implement half-open overlap checking in `Student::requestBooking(...)`.
- Implement cancellation release for approved/in-use bookings through the approved
  equipment end-booking behavior.
- Add tests for valid intervals, overlapping intervals, touching boundaries, invalid
  intervals, active-booking replacement, release, and repeated release.

### Exit evidence

- The overlap predicate is tested directly at boundary cases.
- No conflicting active equipment booking can be silently overwritten.
- Cancellation releases equipment and updates Booking state.
- No ownership semantics contradict the class diagram.

### Human eye gate 3

The human reviews interval semantics, equipment ownership/reference semantics, release
behavior, and failure behavior for invalid lifecycle operations.

## Phase 4 — Student/Admin workflows

### Work

- Implement student request, cancellation, and view behavior.
- Implement admin approval, rejection, and notification behavior.
- Preserve the approved method ownership from the class diagram while documenting the
  sequence receiver discrepancy.
- Ensure approval/rejection notifications happen in the defined order and do not introduce
  unmodeled service calls.
- Add unit tests for approval, rejection, cancellation, notification, and booking views.

### Exit evidence

- Booking creation and collection updates are deterministic.
- Only valid state transitions succeed.
- Approval and rejection notify exactly as specified.
- `viewBookings()` returns non-owning references with no hidden ownership transfer.
- Tests cover all user-visible workflow rules.

### Human eye gate 4

The human reviews workflow ownership, notification semantics, collection mutation,
failure results/exceptions, and any cross-class calls that could appear in sequence
tracing.

## Phase 5 — Scenario programs and trace alignment

### Gate decision

The human approved a deterministic implementation of the current single scenario while
retaining the model conflict as a reported deviation. The scenario uses separate booking
objects for approval and rejection so no invalid state transition is forced.

### Work

- Finalize the scenario interpretation approved at Gate 0.
- If split, implement separate deterministic approval and rejection scenario programs.
- Ensure `main()` performs setup only and scenario functions perform actor calls only.
- Make exactly the modeled cross-class calls in the modeled order.
- Do not add getters, logging calls, helper service calls, clock reads, randomness, input,
  or threads that could affect traces.
- Run the verifier and inspect every sequence report.

### Exit evidence

- Each scenario builds and runs deterministically.
- Sequence traces match the approved diagrams or every mismatch is explicitly reported.
- No extra or missing cross-class call remains unexplained.

### Human eye gate 5

The human reviews each scenario's source and the generated trace/report before accepting
the scenario behavior.

## Phase 6 — Full verification and quality hardening

### Work

- Run the full CMake build with warnings enabled.
- Run all unit tests and scenario executables.
- Run `.venv/bin/python tools/verify.py project`.
- Read every report under `project/reports/`.
- Check public documentation, naming, include hygiene, const-correctness, ownership,
  deterministic behavior, and absence of speculative abstractions.
- Compare all final diffs against the input diagrams and specifications.
- Record every failure, warning, deviation, assumption, and unresolved model blocker.
- Fix implementation defects and repeat verification; do not edit reports to conceal them.

### Exit evidence

- Build succeeds without warnings.
- All tests pass.
- All scenarios pass.
- Class, state, and sequence alignment is 100%, or the final report identifies each
  unavoidable mismatch caused by an unupdated model and names the required model change.
- The implementation-quality checklist is complete.

### Human eye gate 6

The human accepts the final verification package only after reviewing:

- build/test commands and outputs;
- all verifier reports;
- the alignment percentage and every deviation;
- assumptions not captured by the ADR or diagrams;
- the final source diff;
- whether the implementation is ready for the model to be updated or released.

## Phase completion and archival

After Gate 6 approval and only after alignment is 100% (or the human explicitly records
accepted model blockers), move this plan to `project/process/plans/completed/`. Do not
move it early. If the model changes, create a new dated plan or revision and restart at
the first affected gate.

## Required final report format

The final implementation report must include:

1. files created or changed;
2. commands run;
3. build and warning result;
4. unit-test result;
5. scenario result;
6. verifier alignment result for class, state, and sequence designs;
7. every `Changed`, `Missing`, and `Extra` report entry;
8. assumptions and model deviations;
9. unresolved blockers and the human decision needed;
10. confirmation that no input diagram or generated report was edited.
