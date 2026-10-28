const KEY = "lab-booking-benchmark-v1";

function read() {
  return JSON.parse(localStorage.getItem(KEY) || "null") || {
    format: "lab-booking-dashboard-benchmark",
    version: 1,
    eventCount: 0,
    totalDurationMilliseconds: 0,
    events: [],
  };
}

export function recordBenchmarkEvent(type, action) {
  const started = performance.now();
  const result = action();
  const duration = performance.now() - started;
  const report = read();
  report.eventCount += 1;
  report.totalDurationMilliseconds += duration;
  report.events.push({
    sequence: report.eventCount,
    type,
    ok: true,
    durationMilliseconds: Number(duration.toFixed(3)),
  });
  report.events = report.events.slice(-200);
  localStorage.setItem(KEY, JSON.stringify(report));
  return result;
}

export function benchmarkReport() {
  return read();
}
