export class Board extends EventTarget {
  #port = null; #reader = null; #writer = null;
  get connected() { return this.#port !== null; }
  async connect() {
    if (!("serial" in navigator)) throw new Error("WebSerial is unavailable; use Chrome or Edge.");
    this.#port = await navigator.serial.requestPort({ filters: [{ usbVendorId: 0x0d28 }] });
    await this.#port.open({ baudRate: 115200 });
    this.#writer = this.#port.writable.getWriter();
    this.dispatchEvent(new Event("connected"));
    this.#read();
    await this.send("HELLO");
  }
  async send(command) {
    if (!this.#writer) throw new Error("micro:bit is not connected.");
    await this.#writer.write(new TextEncoder().encode(`${command}\n`));
  }
  async disconnect() {
    if (!this.#port) return;
    await this.#reader?.cancel();
    this.#writer?.releaseLock();
    await this.#port.close();
    this.#port = this.#reader = this.#writer = null;
    this.dispatchEvent(new Event("disconnected"));
  }
  async #read() {
    const decoder = new TextDecoderStream();
    this.#port.readable.pipeTo(decoder.writable).catch(() => {});
    this.#reader = decoder.readable.getReader();
    let buffer = "";
    try {
      while (this.connected) {
        const { value, done } = await this.#reader.read();
        if (done) break;
        buffer += value;
        let end;
        while ((end = buffer.indexOf("\n")) >= 0) {
          const line = buffer.slice(0, end).trim(); buffer = buffer.slice(end + 1);
          if (line) {
            try { this.dispatchEvent(new CustomEvent("message", { detail: JSON.parse(line) })); }
            catch { this.dispatchEvent(new CustomEvent("error", { detail: "The board sent invalid JSON." })); }
          }
        }
      }
    } finally { this.#reader = null; }
  }
}
