import { Board } from "./board.js";
import * as store from "./db.js";

const STATUS_LABELS = {
    pending: "Pending",
    approved: "Approved",
    in_use: "In use",
    finished: "Finished",
    cancelled: "Cancelled",
    rejected: "Rejected",
};
const DISPLAY_CODES = {
    pending: "P",
    approved: "A",
    in_use: "U",
    finished: "F",
    cancelled: "C",
    rejected: "R",
};
const TRANSITIONS = {
    pending: ["approved", "rejected", "cancelled"],
    approved: ["in_use", "cancelled"],
    in_use: ["finished", "cancelled"],
    finished: [],
    cancelled: [],
    rejected: [],
};
const board = new Board();
const state = { bookings: store.bookings(), selected: "B001", results: null };
const $ = (id) => document.getElementById(id);

function notice(text, isError = false) {
    const element = $("notice");
    element.textContent = text;
    element.hidden = false;
    element.dataset.error = String(isError);
}

function escapeText(value) {
    return document.createTextNode(String(value));
}

function renderBookings() {
    const active = state.bookings.find((booking) => booking.id === state.selected) ?? state.bookings[0];
    if (active) {
        state.selected = active.id;
        $("display-letter").textContent = DISPLAY_CODES[active.status] ?? "?";
        $("display-id").textContent = active.id;
        $("board-display").setAttribute("aria-label", `${active.id}: ${STATUS_LABELS[active.status] ?? active.status}`);
    } else {
        $("display-letter").textContent = "—";
        $("display-id").textContent = "No bookings";
    }

    const list = $("booking-list");
    list.replaceChildren();
    for (const booking of state.bookings) {
        const item = document.createElement("li");
        item.className = "booking";
        item.dataset.selected = String(booking.id === state.selected);
        const info = document.createElement("div");
        info.className = "booking-info";
        const title = document.createElement("strong");
        title.append(escapeText(booking.id));
        const equipment = document.createElement("span");
        equipment.append(escapeText(booking.equipment ?? "Lab equipment"));
        const status = document.createElement("span");
        status.className = `status status-${booking.status}`;
        status.append(escapeText(STATUS_LABELS[booking.status] ?? booking.status));
        info.append(title, equipment, status);
        const actions = document.createElement("div");
        actions.className = "booking-actions";
        for (const next of TRANSITIONS[booking.status] ?? []) {
            const button = document.createElement("button");
            button.type = "button";
            button.textContent = STATUS_LABELS[next];
            button.addEventListener("click", () => void changeStatus(booking, next));
            actions.append(button);
        }
        const select = document.createElement("button");
        select.type = "button";
        select.className = "secondary";
        select.textContent = booking.id === state.selected ? "Selected" : "Show";
        select.disabled = booking.id === state.selected;
        select.addEventListener("click", () => void selectBooking(booking.id));
        actions.append(select);
        item.append(info, actions);
        list.append(item);
    }
}

function renderEvents() {
    const list = $("event-list");
    list.replaceChildren();
    const events = store.events().slice(-60).reverse();
    for (const event of events) {
        const item = document.createElement("li");
        const time = document.createElement("time");
        time.dateTime = new Date(event.at).toISOString();
        time.textContent = new Date(event.at).toLocaleTimeString();
        const text = document.createElement("span");
        text.append(escapeText(`${event.bookingId ?? "System"} · ${event.detail} · ${event.source}`));
        item.append(time, text);
        list.append(item);
    }
}

function renderConnection() {
    $("connection").textContent = board.connected ? "Board connected" : "Board disconnected";
    $("connection").dataset.state = board.connected ? "on" : "off";
    $("connect").textContent = board.connected ? "Disconnect" : "Connect micro:bit";
}

function renderChart(svg, values, minValue, maxValue) {
    svg.replaceChildren();
    if (values.length === 0) return;
    const ns = "http://www.w3.org/2000/svg";
    const line = document.createElementNS(ns, "line");
    line.setAttribute("x1", "32");
    line.setAttribute("x2", "624");
    line.setAttribute("y1", "126");
    line.setAttribute("y2", "126");
    line.setAttribute("class", "axis");
    svg.append(line);
    const usable = values.slice(-20);
    const range = Math.max(1, maxValue - minValue);
    const points = usable.map((value, index) => {
        const x = usable.length === 1 ? 320 : 36 + index * 584 / (usable.length - 1);
        const y = 116 - (value - minValue) * 92 / range;
        return `${x},${y}`;
    }).join(" ");
    const polyline = document.createElementNS(ns, "polyline");
    polyline.setAttribute("points", points);
    polyline.setAttribute("class", "trend");
    svg.append(polyline);
    usable.forEach((value, index) => {
        const x = usable.length === 1 ? 320 : 36 + index * 584 / (usable.length - 1);
        const y = 116 - (value - minValue) * 92 / range;
        const circle = document.createElementNS(ns, "circle");
        circle.setAttribute("cx", String(x));
        circle.setAttribute("cy", String(y));
        circle.setAttribute("r", "4");
        circle.setAttribute("class", "point");
        circle.setAttribute("aria-label", `Run ${index + 1}: ${value}`);
        svg.append(circle);
    });
}

function renderResults(latest, history) {
    state.results = latest;
    $("run-time").textContent = `Run ${latest.iteration} · ${new Date(latest.timestamp).toLocaleString()}`;
    $("fom").textContent = `${latest.verification.fom_percent}%`;
    $("check-count").textContent = `${latest.verification.passed}/${latest.verification.total} fixed checks passed`;
    $("tests").textContent = `${latest.unit_tests.passed_count}/${latest.unit_tests.total_count}`;
    $("test-detail").textContent = latest.unit_tests.passed ? "All host tests passed" : "One or more host tests failed";
    $("benchmark").textContent = `${Number(latest.benchmark.microseconds_per_lifecycle).toFixed(2)} μs`;
    const checks = $("checks-list");
    checks.replaceChildren();
    for (const check of latest.verification.checks) {
        const item = document.createElement("li");
        item.dataset.pass = String(check.passed);
        item.textContent = `${check.passed ? "PASS" : "FAIL"} · ${check.description}`;
        checks.append(item);
    }
    renderChart(
        $("fom-chart"),
        history.map((run) => run.verification.fom_percent),
        0,
        100,
    );
    const measurements = history.map((run) => run.benchmark.microseconds_per_lifecycle);
    renderChart($("benchmark-chart"), measurements, Math.min(...measurements), Math.max(...measurements));
}

async function loadResults() {
    try {
        const [latestResponse, historyResponse] = await Promise.all([
            fetch("./data/latest-results.json", { cache: "no-store" }),
            fetch("./data/verification-history.json", { cache: "no-store" }),
        ]);
        if (!latestResponse.ok || !historyResponse.ok) {
            throw new Error("No generated verifier report yet. Run tools\\adrverify\\verify.py.");
        }
        const latest = await latestResponse.json();
        const history = await historyResponse.json();
        if (!Array.isArray(history.runs) || history.runs.length === 0) {
            throw new Error("Verifier history is empty. Run tools\\adrverify\\verify.py.");
        }
        renderResults(latest, history.runs);
    } catch (error) {
        $("run-time").textContent = error.message;
    }
}

async function send(command) {
    if (!board.connected) return;
    try {
        await board.send(command);
    } catch (error) {
        notice(error.message, true);
    }
}

async function selectBooking(id) {
    state.selected = id;
    renderBookings();
    store.addEvent("selected", id, "Booking selected", "dashboard");
    renderEvents();
    await send(`SELECT ${id}`);
}

async function changeStatus(booking, status) {
    const current = state.bookings.find((item) => item.id === booking.id);
    if (!current || !(TRANSITIONS[current.status] ?? []).includes(status)) return;
    current.status = status;
    state.selected = current.id;
    store.saveBookings(state.bookings);
    store.addEvent("status", current.id, `Status changed to ${STATUS_LABELS[status]}`, "dashboard");
    renderBookings();
    renderEvents();
    await send(`SET ${current.id} ${status}`);
}

async function navigate(direction) {
    if (state.bookings.length === 0) return;
    const current = state.bookings.findIndex((booking) => booking.id === state.selected);
    const next = (current + direction + state.bookings.length) % state.bookings.length;
    state.selected = state.bookings[next].id;
    renderBookings();
    store.addEvent("selected", state.selected, "Booking selected", "dashboard");
    renderEvents();
    if (board.connected) {
        await send(`MOVE ${direction < 0 ? "A" : "B"}`);
    }
}

async function addBooking() {
    if (state.bookings.length >= 8) {
        notice("The mock-up supports at most eight bookings on the board.", true);
        return;
    }
    const booking = {
        id: store.nextBookingId(state.bookings),
        status: "pending",
        equipment: "Lab equipment",
    };
    state.bookings.push(booking);
    state.selected = booking.id;
    store.saveBookings(state.bookings);
    store.addEvent("created", booking.id, "Pending booking created", "dashboard");
    renderBookings();
    renderEvents();
    await send(`ADD ${booking.id}`);
}

async function syncBoard(remoteBookings) {
    const remoteIds = new Set(remoteBookings.map((booking) => booking.id));
    for (const remote of remoteBookings) {
        if (!state.bookings.some((booking) => booking.id === remote.id)) {
            state.bookings.push({ ...remote, equipment: "Lab equipment" });
        }
    }
    store.saveBookings(state.bookings);
    for (const booking of state.bookings) {
        if (!remoteIds.has(booking.id)) await board.send(`ADD ${booking.id}`);
        await board.send(`SET ${booking.id} ${booking.status}`);
    }
    if (state.bookings.some((booking) => booking.id === state.selected)) {
        await board.send(`SELECT ${state.selected}`);
    }
    renderBookings();
}

board.addEventListener("connected", () => {
    renderConnection();
    notice("Connected. Waiting for the board's booking list.");
});
board.addEventListener("disconnected", () => {
    renderConnection();
    notice("Board disconnected.");
});
board.addEventListener("error", ({ detail }) => notice(detail, true));
board.addEventListener("message", async ({ detail: message }) => {
    try {
        if (message.type === "hello") {
            await syncBoard(message.bookings ?? []);
            notice(`Board ready with ${message.bookings?.length ?? 0} bookings.`);
        } else if (message.type === "select") {
            state.selected = message.id;
            if (message.source === "button") {
                store.addEvent("selected", message.id, "Board navigation button pressed", "micro:bit");
                renderEvents();
            }
            renderBookings();
        } else if (message.type === "state") {
            const booking = state.bookings.find((item) => item.id === message.id);
            if (booking && STATUS_LABELS[message.status]) {
                booking.status = message.status;
                state.selected = message.id;
                store.saveBookings(state.bookings);
                renderBookings();
            }
        } else if (message.type === "error") {
            notice(`Board error: ${message.message}`, true);
        }
    } catch (error) {
        notice(`Could not process board message: ${error.message}`, true);
    }
});

$("connect").addEventListener("click", async () => {
    try {
        if (board.connected) await board.disconnect();
        else if (!Board.supported) notice("WebSerial is unavailable. Use Chrome or Edge on localhost or HTTPS.", true);
        else await board.connect();
    } catch (error) {
        notice(`Could not connect: ${error.message}`, true);
    }
});
$("previous").addEventListener("click", () => void navigate(-1));
$("next").addEventListener("click", () => void navigate(1));
$("add-booking").addEventListener("click", () => void addBooking());
$("clear-events").addEventListener("click", () => {
    store.clearEvents();
    renderEvents();
});

renderBookings();
renderEvents();
renderConnection();
void loadResults();
