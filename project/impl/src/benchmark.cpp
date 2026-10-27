#include "lab_booking/admin.hpp"
#include "lab_booking/booking.hpp"
#include "lab_booking/equipment.hpp"
#include "lab_booking/student.hpp"

#include <chrono>
#include <cstddef>
#include <iostream>

int main()
{
    using Clock = std::chrono::steady_clock;
    constexpr std::size_t iterations = 10000;
    constexpr int samples = 7;
    long long total_microseconds = 0;

    for (int sample = 0; sample < samples; ++sample) {
        const auto start = Clock::now();
        for (std::size_t i = 0; i < iterations; ++i) {
            lab_booking::Student student(1, 1, "Student");
            lab_booking::Admin admin(2, 1, "Admin");
            lab_booking::Equipment equipment(3, "Microscope");
            lab_booking::Booking booking(static_cast<int>(i + 1));
            const auto begin = std::chrono::system_clock::time_point{};
            const auto end = begin + std::chrono::hours(1);
            student.requestBooking(booking, equipment, begin, end);
            admin.approveBooking(booking, student);
            equipment.startBooking(booking, begin, end);
            equipment.endBooking();
        }
        total_microseconds += std::chrono::duration_cast<std::chrono::microseconds>(
            Clock::now() - start).count();
    }

    const auto average = total_microseconds / samples;
    std::cout << "{\"name\":\"host_booking_lifecycle\","
              << "\"iterations_per_sample\":" << iterations << ","
              << "\"samples\":" << samples << ","
              << "\"average_microseconds\":" << average << ","
              << "\"microseconds_per_lifecycle\":"
              << (static_cast<double>(average) / iterations) << "}\n";
}
