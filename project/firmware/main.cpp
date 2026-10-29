#include "MicroBit.h"

#include <cstdio>
#include <cstring>

MicroBit uBit;

namespace {

// adapted from https://stackoverflow.com/a/17323608
template<typename T>
T modulo(T n, T m) {
    return ((n % m) + m) % m;
}

constexpr int kBookingCount = 5;
const char* const kStatuses[] = {"pending", "approved", "in_use", "finished", "cancelled", "rejected"};
const char kCodes[] = {'P', 'A', 'U', 'F', 'C', 'R'};
int status[kBookingCount] = {};
int selected = 0;
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
    uBit.display.printChar(kCodes[status[selected]]);
}

void sendState()
{
    char line[120];
    snprintf(line, sizeof line, "{\"type\":\"state\",\"selected\":%d,\"status\":\"%s\",\"code\":\"%c\"}\n",
             selected, kStatuses[status[selected]], kCodes[status[selected]]);
    sendLine(line);
}

void selectBooking(int index, const char* source)
{
    selected = modulo(index + (kBookingCount - 1)), kBookingCount - 1);
    show();
    char line[120];
    snprintf(line, sizeof line, "{\"type\":\"select\",\"selected\":%d,\"source\":\"%s\"}\n",
             selected, source);
    sendLine(line);
}

void setStatus(int booking, int next, const char* source)
{
    if (booking < 0 || booking >= kBookingCount || next < 0 || next >= 6) {
        sendLine("{\"type\":\"error\",\"message\":\"invalid booking or status\"}\n");
        return;
    }
    selected = booking;
    status[booking] = next;
    show();
    char line[160];
    snprintf(line, sizeof line, "{\"type\":\"event\",\"message\":\"booking %d changed to %s\",\"booking\":%d,\"status\":\"%s\",\"source\":\"%s\"}\n",
             booking + 1, kStatuses[next], booking, kStatuses[next], source);
    sendLine(line);
    sendState();
}

int statusIndex(const char* value)
{
    for (int index = 0; index < 6; ++index) {
        if (strcmp(value, kStatuses[index]) == 0) return index;
    }
    return -1;
}

void handleCommand(const char* line)
{
    int booking = 0;
    char value[20] = {};
    if (strcmp(line, "HELLO") == 0) {
        sendLine("{\"type\":\"hello\",\"firmware\":\"lab-booking-microbit-v1\",\"bookings\":5}\n");
        sendState();
    } else if (sscanf(line, "SELECT %d", &booking) == 1) {
        selectBooking(booking, "web");
    } else if (sscanf(line, "STATUS %d %19s", &booking, value) == 2) {
        const int next = statusIndex(value);
        setStatus(booking, next, "web");
    } else {
        sendLine("{\"type\":\"error\",\"message\":\"unknown command\"}\n");
    }
}

void onButtonA(MicroBitEvent) { selectBooking(selected - 1, "button-A"); }
void onButtonB(MicroBitEvent) { selectBooking(selected + 1, "button-B"); }

void serialLoop()
{
    while (true) {
        ManagedString line = uBit.serial.readUntil(ManagedString("\n"), SYNC_SLEEP);
        handleCommand(line.toCharArray());
    }
}

}  // namespace

int main()
{
    uBit.init();
    uBit.serial.setRxBufferSize(64);
    uBit.serial.setTxBufferSize(192);
    uBit.messageBus.listen(MICROBIT_ID_BUTTON_A, MICROBIT_BUTTON_EVT_CLICK, onButtonA);
    uBit.messageBus.listen(MICROBIT_ID_BUTTON_B, MICROBIT_BUTTON_EVT_CLICK, onButtonB);
    show();
    sendLine("{\"type\":\"hello\",\"firmware\":\"lab-booking-microbit-v1\",\"bookings\":5}\n");
    sendState();
    create_fiber(serialLoop);
    release_fiber();
}
