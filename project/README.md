# ADR: Lab Equipment Booking System

## Context

The Lab Equipment Booking System allows students to request, view, and cancel bookings for laboratory equipment. Administrators are responsible for reviewing booking requests and either approving or rejecting them. The system ensures that equipment is not booked by multiple students during the same period.

## Decision

The system consists of four main entities: Student, Booking, Admin and Equipment.

- **Student:** A student can request a booking using `requestBooking()`, cancel an existing booking using `cancelBooking()`, and view their bookings using `viewBookings()`.

- **Booking:** When a student requests a booking, the system creates a Booking object with a unique `bookingId` and initially sets its status to `pending`. The booking is associated with the student and the selected equipment, as well as the intended `startDate` and `endDate`.

- **Administrator:** An administrator reviews pending bookings and can either approve or reject them. The `approveBooking()` function changes the booking status to `approved`, while `rejectBooking()` changes the booking status to `rejected`. Both actions trigger `notifyStudent()` to inform the student of the decision.

- **Equipment:** Each piece of equipment has an `inUse` Boolean variable, which indicates whether it is currently in use. When a booking is approved, the Booking object signals the selected Equipment using `startBooking()`. The equipment's `inUse` variable is then set to `true`, preventing conflicting bookings for the same period. The equipment also records the booking's `startDate` and `endDate`.

When the booking's `endDate` is reached, the Booking object signals the Equipment using `endBooking()`, resetting `inUse` to `false` and making the equipment available again.

Students can cancel their bookings at any time, including while the booking is still pending. If a booking is cancelled after approval, the system must also release the associated equipment if it has been reserved or is currently in use. The booking status is then updated to 'cancelled'.