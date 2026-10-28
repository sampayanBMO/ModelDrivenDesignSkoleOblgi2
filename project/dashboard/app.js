import { Board } from "./board.js";
import { benchmarkReport, recordBenchmarkEvent } from "./benchmark.js";

const KEY = "lab-booking-dashboard-v1";
const statuses = { pending: ["P", "Pending"], approved: ["A", "Approved"], in_use: ["U", "In use"], finished: ["F", "Finished"], cancelled: ["C", "Cancelled"], rejected: ["R", "Rejected"] };
const transitions = { pending: ["approved", "rejected", "cancelled"], approved: ["in_use", "cancelled"], in_use: ["finished", "cancelled"], finished: [], cancelled: [], rejected: [] };
const state = JSON.parse(localStorage.getItem(KEY) || "null") || { bookings: [], events: [], selected: 0, results: {}, history: [] };
state.bookings ??= []; state.events ??= []; state.results ??= {}; state.history ??= []; state.selected ??= 0;
const board = new Board();
const $ = (id) => document.getElementById(id);
const save = () => localStorage.setItem(KEY, JSON.stringify(state));
const event = (message) => { recordBenchmarkEvent("dashboard.event", () => {}); state.results.benchmark = benchmarkReport(); state.events.unshift({ at: new Date().toISOString(), message }); state.events = state.events.slice(0, 100); save(); renderEvents(); renderQuality(); };
const selected = () => state.bookings[state.selected];

function render() {
  const booking = selected();
  $("selected-name").textContent = booking ? `${booking.id} · ${booking.title}` : "No bookings yet";
  $("selected-details").textContent = booking ? `${statuses[booking.status][1]} · Equipment: ${booking.equipment} · Requested by ${booking.student}` : "Create a booking to begin.";
  $("board-code").textContent = booking ? statuses[booking.status][0] : "—";
  $("previous").disabled = state.bookings.length < 2; $("next").disabled = state.bookings.length < 2;
  document.querySelectorAll("[data-status]").forEach((button) => { button.disabled = !booking || !transitions[booking.status].includes(button.dataset.status); });
  $("bookings").replaceChildren(...state.bookings.map((item, index) => {
    const node = $("booking-template").content.cloneNode(true); const button = node.querySelector("button");
    button.classList.toggle("selected", index === state.selected); button.querySelector("strong").textContent = `${item.id} · ${item.title}`; button.querySelector("span").textContent = item.equipment; button.querySelector("b").textContent = statuses[item.status][0] + " · " + statuses[item.status][1]; button.querySelector("b").dataset.status = item.status;
    button.onclick = () => { state.selected = index; save(); event(`Selected booking ${item.id}`); render(); board.connected && board.send(`SELECT ${index}`); }; return node;
  }));
}
function renderEvents() { $("events").innerHTML = state.events.length ? state.events.map((item) => `<li><span class="event-time">${new Date(item.at).toLocaleTimeString()}</span>${item.message}</li>`).join("") : "<li>No events yet.</li>"; }
function renderQuality() {
  const v = state.results.verification; const t = state.results.tests; const b = state.results.benchmark;
  $("quality").innerHTML = `<div class="quality"><div class="metric"><span>Verification FoM</span><strong>${v ? `${v.fomPercent}% (${v.passed}/${v.total})` : "Load verification JSON"}</strong></div><div class="metric"><span>Unit tests</span><strong>${t?.summary ? `${t.summary.passed}/${t.summary.total} passed` : "Load test JSON"}</strong></div><div class="metric"><span>Benchmark</span><strong>${b ? `${b.eventCount} events recorded` : "Load benchmark JSON"}</strong></div></div>`;
  const points = state.history; const svg = $("fom-chart"); if (!points.length) { svg.innerHTML = ""; return; } const x = (i) => 35 + i * (530 / Math.max(1, points.length - 1)); const y = (value) => 195 - value * 1.65; svg.innerHTML = `<line x1="30" y1="30" x2="30" y2="195" stroke="#dce1e6"/><line x1="30" y1="195" x2="580" y2="195" stroke="#dce1e6"/><text x="35" y="18">100%</text><text x="35" y="212">0%</text><polyline points="${points.map((point, i) => `${x(i)},${y(point.fomPercent)}`).join(" ")}"/>${points.map((point, i) => `<circle cx="${x(i)}" cy="${y(point.fomPercent)}" r="5"><title>${point.label}: ${point.fomPercent}%</title></circle><text x="${x(i) - 12}" y="210">${point.label}</text>`).join("")}`; $("fom-label").textContent = v ? `${v.fomPercent}% current FoM` : "No verification run";
}
function loadResult(result) { if (result.format.includes("verification")) { state.results.verification = result; state.history = result.history || state.history; } else if (result.format.includes("unit-tests")) state.results.tests = result; else if (result.format.includes("benchmark")) state.results.benchmark = result; save(); renderQuality(); }
function syncSelectedStatus() { const booking = selected(); if (booking && board.connected) board.send(`STATUS ${state.selected} ${booking.status}`); }

$("new-booking").onclick = () => { const number = state.bookings.length + 1; state.bookings.push({ id: `B${number}`, title: `Booking ${number}`, equipment: "Equipment 1", student: "Dashboard user", status: "pending" }); state.selected = state.bookings.length - 1; event(`Created pending booking B${number}`); render(); };
$("previous").onclick = () => { state.selected = (state.selected + state.bookings.length - 1) % state.bookings.length; save(); event(`Selected booking ${selected().id} with A`); render(); board.connected && board.send(`SELECT ${state.selected}`); };
$("next").onclick = () => { state.selected = (state.selected + 1) % state.bookings.length; save(); event(`Selected booking ${selected().id} with B`); render(); board.connected && board.send(`SELECT ${state.selected}`); };
document.querySelectorAll("[data-status]").forEach((button) => button.onclick = () => { const booking = selected(); const next = button.dataset.status; if (!booking || !transitions[booking.status].includes(next)) return; booking.status = next; event(`Booking ${booking.id} changed to ${statuses[next][1]}`); render(); board.connected && board.send(`STATUS ${state.selected} ${next}`); });
$("connect").onclick = async () => { try { if (board.connected) await board.disconnect(); else await board.connect(); } catch (error) { event(`Connection error: ${error.message}`); } };
board.addEventListener("connected", () => { $("connection").textContent = "Connected"; $("connection").classList.add("on"); event("Connected to micro:bit"); });
board.addEventListener("disconnected", () => { $("connection").textContent = "Not connected"; $("connection").classList.remove("on"); event("Disconnected from micro:bit"); });
board.addEventListener("message", ({ detail }) => { if (detail.type === "hello") syncSelectedStatus(); if (detail.type === "state" && state.bookings[detail.selected]) { state.selected = detail.selected; render(); } if (detail.type === "event") event(`micro:bit: ${detail.message}`); if (detail.type === "error") event(`micro:bit error: ${detail.message}`); });
$("result-file").onchange = async ({ target }) => { if (target.files[0]) { try { loadResult(JSON.parse(await target.files[0].text())); event(`Loaded ${target.files[0].name}`); } catch { event("Could not read the selected JSON result."); } } };
async function loadGenerated(name) { try { const response = await fetch(`../verification/${name}`); if (response.ok) loadResult(await response.json()); } catch {} }
async function loadHistory() { try { const response = await fetch("../verification/verification-history.json"); if (response.ok) { state.history = await response.json(); save(); renderQuality(); } } catch {} }
state.results.benchmark = benchmarkReport(); loadGenerated("verification-dashboard.json"); loadGenerated("unit-test-results-dashboard.json"); loadGenerated("benchmark-dashboard.json"); loadHistory(); render(); renderEvents(); renderQuality();
