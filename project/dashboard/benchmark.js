const KEY = "lab-booking-benchmark-v1";

function read() {
  const stored = JSON.parse(localStorage.getItem(KEY) || "null");
  return stored ? {
    ...stored,
    totalDurationNanoseconds: Number.isFinite(stored.totalDurationNanoseconds) ? stored.totalDurationNanoseconds : 0,
    events: Array.isArray(stored.events) ? stored.events : [],
  } : {
    format: "lab-booking-dashboard-benchmark",
    version: 1,
    eventCount: 0,
    totalDurationNanoseconds: 0,
    events: [],
  };
}

export function recordBenchmarkEvent(type, action) {
  const started = performance.now();
  const result = action();
  const durationNanoseconds = Math.max(0, Math.round((performance.now() - started) * 1_000_000));
  const report = read();
  report.eventCount += 1;
  report.totalDurationNanoseconds += durationNanoseconds;
  report.events.push({
    sequence: report.eventCount,
    type,
    ok: true,
    durationNanoseconds,
  });
  report.events = report.events.slice(-200);
  localStorage.setItem(KEY, JSON.stringify(report));
  return { result, durationNanoseconds };
}

export function benchmarkReport() {
  return read();
}

export function resetBenchmark() {
  localStorage.removeItem(KEY);
}
