# Lab Equipment Booking System — Ground-Truth Research

## Sources inspected

- `project/README.md` (ADR)
- `project/diagrams/input/class.drawio`
- `project/diagrams/input/state-booking.drawio`
- `project/diagrams/input/sequence-diagram.drawio`

The ADR and diagrams are treated as authoritative. No implementation files or design
artifacts were changed.

### Confirmed precedence

The diagrams are mechanically authoritative. The ADR is behavioral context and does not
add members, relations, parameters, or calls that are absent from the diagrams.

### Confirmed Booking surface

`Booking` must contain only the explicitly drawn members: `bookingId`, `status`,
`startBooking()`, and `endBooking()`. No student or equipment reference is added to
`Booking` based solely on the ADR.

### Pending signature decision

The user prefers realistic operation parameters and date/time types even though those
details are omitted from the diagrams. Exact signatures and the concrete replacement for
`time` must be specified before implementation; they cannot be inferred without adding
theory beyond the current ground truth.

The selected direction is ADR-driven: booking requests carry equipment and start/end
values; cancellation, approval, and rejection carry a booking; viewing returns bookings;
notifications carry a student and decision; `time` maps to
`std::chrono::system_clock::time_point`. This is a deliberate extension beyond the
diagram notation and requires an explicit decision about whether the design artifacts may
be amended or whether the implementation is intentionally allowed to diverge from their
mechanical contract.

The selected policy is to treat the realistic signatures as an explicit ADR extension;
the diagrams must be updated later before a verifier-aligned implementation is expected.

The linear sequence is to be split into separate approval and rejection scenarios, because
approval and rejection are mutually exclusive transitions from `Pending`.

`notifyStudent(...)` remains an `Admin` operation; it is a sender-side operation that
targets a `Student`, rather than a method moved onto `Student` or duplicated there.

Cancellation of an approved or in-use booking explicitly releases equipment through
`Equipment::endBooking()`, in addition to moving the Booking to `Cancelled`.

Equipment overlap uses half-open intervals `[start, end)`, so a booking may start exactly
when another booking ends.

The unlabeled transitions from `Rejected`, `Cancelled`, and `Finished` are terminal
notation only; no additional events or methods are introduced for them.

For the ADR-driven extension, `viewBookings()` returns `std::vector<Booking*>`, and
notification decision/status is represented as `std::string`. The remaining exact
signature convention is: booking request receives an `Equipment` plus start/end
`std::chrono::system_clock::time_point` values; cancellation, approval, and rejection
receive a `Booking`; notification receives a `Student` and a string decision/status.

`Equipment::whichBooking` represents only the currently active booking. Pending and future
bookings are represented through the existing equipment-to-booking association.

Identifiers are provided by the caller when objects are created; the mock-up does not
invent an ID-generation policy.

`Student::requestBooking(...)` enforces the no-overlap rule before creating or retaining a
booking. The selected boundary rule remains half-open `[start, end)`.

The public `Booking::status` string remains externally visible and is synchronized with
the state machine's internal `State` value; the two are not independent sources of truth.

## Consolidated model

### Classes and members shown in the class diagram

- `User`
  - public attributes: `id: integer`, `departmentId: integer`, `name: string`
- `Student : User`
  - public operations: `requestBooking()`, `cancelBooking()`, `viewBookings()`
- `Admin : User`
  - public operations: `approveBooking()`, `rejectBooking()`, `notifyStudent()`
- `Equipment`
  - public attributes: `equipmentId: integer`, `name: string`, `inUse: bool`,
    `whichBooking: Booking = null`, `startDate: time`, `endDate: time`
- `Booking`
  - public attributes: `bookingId: integer`, `status: string`
  - public operations: `startBooking()`, `endBooking()`

### Class relationships

- `User` is the base class of `Student` and `Admin`.
- `Student -> Booking`, role `requests`, multiplicity `1` at `Student` and `0..*` at
  `Booking`.
- `Equipment -> Booking`, role `booked in`, multiplicity `1` at `Equipment` and `0..*`
  at `Booking`.
- `Admin -> Booking`, role `reviews`, multiplicity `0..1` at `Admin` and `0..*` at
  `Booking`.

The arrows are associations in the drawing, not ownership/composition relationships.
The ADR additionally says that a booking is associated with one student and one selected
equipment, but the class diagram does not show corresponding `student` or `equipment`
members on `Booking`.

### Booking state machine

- Initial state: `Pending`
- States: `Pending`, `Approved`, `Rejected`, `InUse`, `Cancelled`, `Finished`
- Transitions:
  - initial -> `Pending`, labelled `requestBooking()`
  - `Pending` -> `Approved`, labelled `approveBooking()`
  - `Pending` -> `Rejected`, labelled `rejectBooking()`
  - `Pending` -> `Cancelled`, labelled `cancelBooking()`
  - `Approved` -> `InUse`, labelled `startBooking()`
  - `Approved` -> `Cancelled`, labelled `cancelBooking()`
  - `InUse` -> `Cancelled`, labelled `cancelBooking()`
  - `InUse` -> `Finished`, labelled `endBooking()`
  - `Rejected` -> final state, with no visible event label
  - `Cancelled` -> final state, with no visible event label
  - `Finished` -> final state, with no visible event label

The ADR uses the lowercase status values `pending`, `approved`, `rejected`, and
`cancelled`; the state diagram uses title-case state names and also adds `InUse` and
`Finished`.

### Sequence diagram messages, in drawn order

1. `Student -> Booking : requestBooking()`
2. `Admin -> Booking : approveBooking()`
3. `Admin -> Booking : rejectBooking()`
4. `Admin -> Student : notifyStudent()`
5. `Student -> Booking : cancelBooking()`
6. `Student -> Booking : viewBookings()`
7. `Booking -> Equipment : startBooking()`
8. `Booking -> Equipment : endBooking()`

The sequence diagram is a single linear scenario and contains both approval and rejection
of the same apparent booking, even though the state machine makes those paths mutually
exclusive from `Pending`.

## Clarification checklist reviewed with the user

The questions below document the ambiguities that were checked. Their selected resolutions
are recorded in the confirmed decisions above; they are retained as an audit trail of the
ground-truth review.

1. **Which artifact wins where the ADR and diagrams conflict?**
   - Should implementation follow the diagrams mechanically, with the ADR used only as
     behavioral explanation?
   - Or should the ADR's explicitly described associations and lifecycle behavior amend
     the diagrams?

2. **Booking-to-Student and Booking-to-Equipment navigation**
   - Should `Booking` contain references to its student and equipment, despite those
     members not appearing in the class diagram?
   - If yes, should they be non-owning associations, and what exact role names should
     they have?

3. **Equipment's `whichBooking` type**
   - Is `whichBooking` intended to be a nullable reference to one `Booking`, as the
     `= null` notation suggests?
   - Is it intended to represent the currently active booking only, or all reservations
     overlapping the equipment's schedule?

4. **Date/time representation**
   - What concrete C++ type should `time` map to?
   - Are `startDate` and `endDate` dates only, date-times, or an opaque mock-up value?

5. **Operation signatures and ownership of behavior**
   - The diagrams omit all parameters and return types. Should the methods be parameterless
     as drawn, or should the ADR's operations accept/request booking, student, equipment,
     and date information?
   - The sequence diagram addresses `approveBooking()` and `rejectBooking()` to `Booking`,
     while the class diagram declares them on `Admin`. Should these calls be interpreted as
     `Admin` methods that update a `Booking`, or should the receiver in the sequence diagram
     be changed conceptually to `Admin`?

6. **Notification direction**
   - The ADR says approval/rejection triggers `notifyStudent()`, and the sequence diagram
     shows `Admin -> Student : notifyStudent()`. Should `notifyStudent()` be a method on
     `Student` despite being declared on `Admin`, or should `Admin::notifyStudent()` be
     treated as an action that targets a student without adding a `Student` method?

7. **Conflicting sequence path**
   - Should the scenario be split into separate approval and rejection scenarios, or should
     the single drawn sequence intentionally exercise both calls even though rejection after
     approval is not a valid state transition?

8. **Equipment release on cancellation**
   - The ADR requires releasing equipment when an approved booking is cancelled, but the
     state diagram has no explicit `Equipment` call on the cancellation transitions. Should
     cancellation invoke `Equipment::endBooking()` implicitly, or should the diagrams be
     considered incomplete until that interaction is represented?

9. **Overlap prevention**
   - The ADR requires preventing conflicting bookings for the same equipment and period.
     What is the authoritative rule for overlap at boundaries (for example, whether a new
     booking may start exactly when another ends)?
   - Which operation performs the conflict check, given that no such operation is drawn?

10. **Identifiers and status values**
    - Are `bookingId`, `equipmentId`, `User::id`, and `departmentId` assigned externally or
      generated by the mock-up?
    - Should status remain a public `string`, or may the state machine's `State` enum be
      the source of truth with string conversion only at an interface boundary?

11. **Meaning of multiplicities**
    - The association arrows point from `Student`, `Equipment`, and `Admin` to `Booking`.
      Should the implementation preserve exactly those navigable directions, even though
      the ADR describes a booking as associated back to its student and equipment?

12. **Terminal state semantics**
    - What event or operation causes the no-label transitions from `Rejected`,
      `Cancelled`, and `Finished` to the final state?
    - Are those transitions only diagram notation, or must they be represented as explicit
      state-machine transitions?
