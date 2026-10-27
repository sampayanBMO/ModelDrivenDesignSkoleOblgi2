#include <cstdio>
#include <cstring>

#include "MicroBit.h"
#include "board_behavior.hpp"

MicroBit uBit;

namespace {

const std::size_t kMaxBookings = 8;

struct BoardBooking
{
    char id[16];
    char status[16];
};

BoardBooking bookings[kMaxBookings] = {
    {"B001", "pending"},
    {"B002", "approved"},
    {"B003", "in_use"},
};
std::size_t bookingCount = 3;
std::size_t selected = 0;
bool sending = false;

void sendLine(const char* line)
{
    while (sending) uBit.sleep(1);
    sending = true;
    uBit.serial.send(ManagedString(line), SYNC_SLEEP);
    sending = false;
}

void show()
{
    if (bookingCount == 0) {
        uBit.display.clear();
        return;
    }
    uBit.display.printChar(lab_booking::booking_display_code(bookings[selected].status));
}

void sendState(const char* source, const char* type = "select")
{
    if (bookingCount == 0) {
        char line[64];
        std::snprintf(line, sizeof line, "{\"type\":\"%s\",\"count\":0}\n", type);
        sendLine(line);
        return;
    }
    char line[160];
    std::snprintf(line, sizeof line,
                  "{\"type\":\"%s\",\"id\":\"%s\",\"status\":\"%s\",\"display\":\"%c\",\"source\":\"%s\"}\n",
                  type, bookings[selected].id, bookings[selected].status,
                  lab_booking::booking_display_code(bookings[selected].status), source);
    sendLine(line);
}

void sendHello()
{
    char line[512];
    int used = std::snprintf(line, sizeof line, "{\"type\":\"hello\",\"bookings\":[");
    for (std::size_t i = 0; i < bookingCount && used < static_cast<int>(sizeof line); ++i) {
        used += std::snprintf(line + used, sizeof line - static_cast<std::size_t>(used),
                              "%s{\"id\":\"%s\",\"status\":\"%s\"}",
                              i == 0 ? "" : ",", bookings[i].id, bookings[i].status);
    }
    std::snprintf(line + used, sizeof line - static_cast<std::size_t>(used),
                  "],\"selected\":\"%s\"}\n",
                  bookingCount == 0 ? "" : bookings[selected].id);
    sendLine(line);
}

void moveBooking(int direction, const char* source)
{
    selected = lab_booking::next_booking_index(selected, bookingCount, direction);
    show();
    sendState(source);
}

bool validStatus(const char* status)
{
    return std::strcmp(status, "pending") == 0 ||
           std::strcmp(status, "approved") == 0 ||
           std::strcmp(status, "in_use") == 0 ||
           std::strcmp(status, "finished") == 0 ||
           std::strcmp(status, "cancelled") == 0 ||
           std::strcmp(status, "rejected") == 0;
}

int findBooking(const char* id)
{
    for (std::size_t i = 0; i < bookingCount; ++i) {
        if (std::strcmp(bookings[i].id, id) == 0) return static_cast<int>(i);
    }
    return -1;
}

void handleCommand(const char* line)
{
    char command[64];
    std::snprintf(command, sizeof command, "%s", line);
    std::size_t length = std::strlen(command);
    while (length > 0 && (command[length - 1] == '\r' || command[length - 1] == ' ')) {
        command[--length] = '\0';
    }

    if (std::strcmp(command, "HELLO") == 0) {
        sendHello();
        return;
    }
    if (std::strncmp(command, "MOVE A", 6) == 0) {
        moveBooking(-1, "dashboard");
        return;
    }
    if (std::strncmp(command, "MOVE B", 6) == 0) {
        moveBooking(1, "dashboard");
        return;
    }
    if (std::strncmp(command, "ADD ", 4) == 0) {
        char id[16] = {};
        if (bookingCount < kMaxBookings &&
            std::sscanf(command + 4, "%15[A-Za-z0-9]", id) == 1 &&
            findBooking(id) < 0) {
            std::snprintf(bookings[bookingCount].id, sizeof bookings[bookingCount].id, "%s", id);
            std::snprintf(bookings[bookingCount].status, sizeof bookings[bookingCount].status, "pending");
            selected = bookingCount++;
            show();
            sendState("dashboard");
            return;
        }
    }
    if (std::strncmp(command, "SELECT ", 7) == 0) {
        const int index = findBooking(command + 7);
        if (index >= 0) {
            selected = static_cast<std::size_t>(index);
            show();
            sendState("dashboard");
            return;
        }
    }
    if (std::strncmp(command, "SET ", 4) == 0) {
        char id[16] = {};
        char status[16] = {};
        if (std::sscanf(command + 4, "%15[A-Za-z0-9] %15[a-z_]", id, status) == 2 &&
            validStatus(status)) {
            const int index = findBooking(id);
            if (index >= 0) {
                std::snprintf(bookings[index].status, sizeof bookings[index].status, "%s", status);
                selected = static_cast<std::size_t>(index);
                show();
                sendState("dashboard", "state");
                return;
            }
        }
    }
    sendLine("{\"type\":\"error\",\"message\":\"unknown command or booking\"}\n");
}

void onButtonA(MicroBitEvent)
{
    moveBooking(-1, "button");
}

void onButtonB(MicroBitEvent)
{
    moveBooking(1, "button");
}

void serialLoop()
{
    while (true) {
        ManagedString message = uBit.serial.readUntil(ManagedString("\n"), SYNC_SLEEP);
        handleCommand(message.toCharArray());
    }
}

}  // namespace

int main()
{
    uBit.init();
    uBit.serial.setRxBufferSize(64);
    uBit.serial.setTxBufferSize(128);
    uBit.messageBus.listen(MICROBIT_ID_BUTTON_A, MICROBIT_BUTTON_EVT_CLICK, onButtonA);
    uBit.messageBus.listen(MICROBIT_ID_BUTTON_B, MICROBIT_BUTTON_EVT_CLICK, onButtonB);
    show();
    sendHello();
    create_fiber(serialLoop);
    release_fiber();
}
