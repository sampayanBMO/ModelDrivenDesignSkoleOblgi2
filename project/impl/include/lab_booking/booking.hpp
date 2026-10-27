#pragma once

#include <string>

namespace lab_booking {

/// A booking whose lifecycle is controlled by the modeled state machine.
class Booking
{
public:
    enum class State { Pending, Approved, Rejected, InUse, Cancelled, Finished };
    enum class Event { requestBooking, approveBooking, rejectBooking, cancelBooking,
                       startBooking, endBooking };

    int bookingId;
    std::string status;

    explicit Booking(int bookingId);
    Booking(int bookingId, std::string status);

    /// Moves an approved booking into the InUse state.
    void startBooking();
    /// Moves an in-use booking into the Finished state.
    void endBooking();
    /// Applies one modeled event; events without a valid transition are ignored.
    void handle(Event event);

private:
    State state_ = State::Pending;

    struct Transition
    {
        State from;
        Event event;
        State to;
        bool (Booking::*guard)() const;
        void (Booking::*action)();
    };

    static constexpr Transition transitions[] = {
        {State::Pending, Event::approveBooking, State::Approved, nullptr, nullptr},
        {State::Pending, Event::rejectBooking, State::Rejected, nullptr, nullptr},
        {State::Pending, Event::cancelBooking, State::Cancelled, nullptr, nullptr},
        {State::Approved, Event::startBooking, State::InUse, nullptr, nullptr},
        {State::InUse, Event::cancelBooking, State::Cancelled, nullptr, nullptr},
        {State::InUse, Event::endBooking, State::Finished, nullptr, nullptr},
    };

};

}  // namespace lab_booking
