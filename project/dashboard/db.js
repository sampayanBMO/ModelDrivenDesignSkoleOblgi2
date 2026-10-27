const BOOKINGS_KEY = "lab-booking.bookings.v1";
const EVENTS_KEY = "lab-booking.events.v1";
const INITIAL_BOOKINGS = [
    { id: "B001", status: "pending", equipment: "Microscope" },
    { id: "B002", status: "approved", equipment: "Oscilloscope" },
    { id: "B003", status: "in_use", equipment: "Camera" },
];

function read(key, fallback, validate) {
    const stored = localStorage.getItem(key);
    if (stored === null) {
        localStorage.setItem(key, JSON.stringify(fallback));
        return structuredClone(fallback);
    }
    const value = JSON.parse(stored);
    if (!validate(value)) throw new Error(`Stored data for ${key} is invalid; clear this browser's booking data to reset it.`);
    return value;
}

export function bookings() {
    return read(BOOKINGS_KEY, INITIAL_BOOKINGS, (value) =>
        Array.isArray(value) && value.every((booking) =>
            typeof booking.id === "string" && typeof booking.status === "string"));
}

export function saveBookings(value) {
    localStorage.setItem(BOOKINGS_KEY, JSON.stringify(value));
}

export function events() {
    return read(EVENTS_KEY, [], (value) =>
        Array.isArray(value) && value.every((event) =>
            typeof event.id === "string" && typeof event.at === "number" && typeof event.type === "string"));
}

export function addEvent(type, bookingId, detail, source) {
    const all = events();
    all.push({
        id: `${Date.now()}-${all.length}`,
        at: Date.now(),
        type,
        bookingId,
        detail,
        source,
    });
    saveEvents(all);
}

function saveEvents(value) {
    localStorage.setItem(EVENTS_KEY, JSON.stringify(value));
}

export function clearEvents() {
    saveEvents([]);
}

export function nextBookingId(all) {
    let number = 1;
    for (const booking of all) {
        const match = /^B(\d+)$/.exec(booking.id);
        if (match) number = Math.max(number, Number(match[1]) + 1);
    }
    return `B${String(number).padStart(3, "0")}`;
}
