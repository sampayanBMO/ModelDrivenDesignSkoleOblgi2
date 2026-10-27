const MICROBIT_USB_VENDOR = 0x0d28;

export class Board extends EventTarget {
    #port = null;
    #reader = null;
    #writer = null;
    #greeted = false;

    get connected() {
        return this.#port !== null;
    }

    static get supported() {
        return "serial" in navigator;
    }

    async connect() {
        const port = await navigator.serial.requestPort({
            filters: [{ usbVendorId: MICROBIT_USB_VENDOR }],
        });
        await port.open({ baudRate: 115200 });
        this.#port = port;
        this.#greeted = false;
        this.#writer = port.writable.getWriter();
        navigator.serial.addEventListener("disconnect", this.#onDisconnect);
        this.dispatchEvent(new Event("connected"));
        void this.#read(port);
        for (let attempt = 0; attempt < 3 && !this.#greeted && this.connected; attempt++) {
            await new Promise((resolve) => setTimeout(resolve, 700));
            if (!this.#greeted && this.connected) await this.send("HELLO");
        }
    }

    async send(command) {
        if (!this.#writer) throw new Error("The micro:bit is not connected.");
        await this.#writer.write(new TextEncoder().encode(`${command}\n`));
    }

    async disconnect() {
        const port = this.#port;
        if (!port) return;
        this.#port = null;
        navigator.serial.removeEventListener("disconnect", this.#onDisconnect);
        try {
            await this.#reader?.cancel();
        } catch {
            this.#report("Reader was already closed.");
        }
        try {
            this.#writer?.releaseLock();
        } catch {
            this.#report("Writer was already closed.");
        }
        this.#reader = null;
        this.#writer = null;
        try {
            await port.close();
        } catch (error) {
            this.#report(`Could not close the serial port: ${error.message}`);
        }
        this.dispatchEvent(new Event("disconnected"));
    }

    #onDisconnect = (event) => {
        if (event.target === this.#port) void this.disconnect();
    };

    async #read(port) {
        const decoder = new TextDecoderStream();
        const piping = port.readable.pipeTo(decoder.writable).catch((error) => {
            if (this.#port === port) this.#report(`Serial read stopped: ${error.message}`);
        });
        this.#reader = decoder.readable.getReader();
        let buffer = "";
        try {
            while (this.#port === port) {
                const { value, done } = await this.#reader.read();
                if (done) break;
                buffer += value;
                let newline;
                while ((newline = buffer.indexOf("\n")) !== -1) {
                    const line = buffer.slice(0, newline).trim();
                    buffer = buffer.slice(newline + 1);
                    if (line) this.#parse(line);
                }
            }
        } catch (error) {
            if (this.#port === port) this.#report(`Serial connection lost: ${error.message}`);
        } finally {
            this.#reader.releaseLock();
            this.#reader = null;
            await piping;
            if (this.#port === port) void this.disconnect();
        }
    }

    #parse(line) {
        try {
            const message = JSON.parse(line);
            if (message.type === "hello") this.#greeted = true;
            this.dispatchEvent(new CustomEvent("message", { detail: message }));
        } catch (error) {
            this.#report(`Invalid board message: ${error.message}`);
        }
    }

    #report(message) {
        this.dispatchEvent(new CustomEvent("error", { detail: message }));
    }
}
