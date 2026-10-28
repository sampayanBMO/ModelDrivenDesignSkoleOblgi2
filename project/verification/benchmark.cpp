#include "lab_booking/admin.hpp"
#include "lab_booking/booking.hpp"
#include "lab_booking/equipment.hpp"
#include "lab_booking/student.hpp"

#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

namespace {

using Clock = std::chrono::system_clock;
using Measurer = std::chrono::steady_clock;

struct EventResult
{
    int sequence;
    std::string type;
    bool ok;
    long long durationNanoseconds;
};

Clock::time_point at(int hour)
{
    return Clock::time_point(Clock::duration(hour * 3600LL));
}

template <typename Action>
EventResult measure(int sequence, const std::string& type, Action action)
{
    const auto started = Measurer::now();
    bool ok = true;
    try {
        action();
    } catch (const std::exception&) {
        ok = false;
    }
    const auto finished = Measurer::now();
    return {sequence, type, ok,
            std::chrono::duration_cast<std::chrono::nanoseconds>(finished - started).count()};
}

void writeJson(const std::string& path, const std::vector<EventResult>& events)
{
    std::ofstream output(path);
    if (!output) {
        throw std::runtime_error("could not open benchmark output");
    }
    long long total = 0;
    for (const EventResult& event : events) {
        total += event.durationNanoseconds;
    }
    output << "{\n  \"format\": \"lab-booking-benchmark\",\n  \"version\": 1,\n";
    output << "  \"eventCount\": " << events.size() << ",\n";
    output << "  \"totalDurationNanoseconds\": " << total << ",\n";
    output << "  \"events\": [\n";
    for (std::size_t index = 0; index < events.size(); ++index) {
        const auto& event = events[index];
        output << "    {\"sequence\": " << event.sequence << ", \"type\": \""
               << event.type << "\", \"ok\": " << (event.ok ? "true" : "false")
               << ", \"durationNanoseconds\": " << event.durationNanoseconds << "}";
        if (index + 1 != events.size()) {
            output << ",";
        }
        output << "\n";
    }
    output << "  ]\n}\n";
}

}  // namespace

int main(int argc, char** argv)
{
    const std::string outputPath = argc > 1 ? argv[1] : "benchmark-results.json";
    lab_booking::Student student(1, 10, "Student");
    lab_booking::Admin admin(2, 10, "Admin");
    lab_booking::Equipment equipment(3, "Oscilloscope");
    lab_booking::Booking booking(4);
    std::vector<EventResult> events;
    int sequence = 1;
    events.push_back(measure(sequence++, "dashboard.select_booking", [] {}));
    events.push_back(measure(sequence++, "booking.request", [&] {
        student.requestBooking(booking, equipment, at(0), at(1));
    }));
    events.push_back(measure(sequence++, "admin.approve", [&] {
        admin.approveBooking(booking, student);
    }));
    events.push_back(measure(sequence++, "equipment.start", [&] {
        equipment.startBooking(booking, at(0), at(1));
    }));
    events.push_back(measure(sequence++, "equipment.end", [&] {
        equipment.endBooking();
    }));
    events.push_back(measure(sequence++, "dashboard.status_refresh", [&] {
        (void)student.viewBookings();
    }));
    writeJson(outputPath, events);
    std::cout << "Recorded " << events.size() << " events in " << outputPath << "\n";
}
