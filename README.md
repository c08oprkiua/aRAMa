# Disclaimer: This project is no longer being worked on.
If you REALLY want to use gecko codes on Aroma, check out [argecko by quarky](https://git.sr.ht/~quarky/arGecko). 

# What is this?

This is an unofficial TCPGecko rewrite and port for Aroma. It contains several improvements, including notifications, enabling and disabling specific features via the plugin config menu, and more.

### **At the time of writing this, this is a work in progress and is not usable yet**.

# How do I use this?

In the config menu, there will be several options:

* `aRAMa active`: Activates/deactivates the entire plugin.

* `Notifications`: When enabled, will send Aroma notifications when the plugin does various things, such as load a code or have an error.

* `Save sent codes`: When a code is set from a client, such as JGeckoU, automatically save it to the SD card for offline use.

* `Caffiine`: Enable Caffiine. 

* `Saviine`: Enable Saviine. 

# Compiling

To compile this, you will need:

* `wut`

* `WiiUPluginSystem`

* `libkernel`

And then simply run `make` in the root directory of this repo after cloning locally. 

# Feature re-implementation checklist:

Though I tried, I cannot guarantee that everything works, because of what was necessarily changed in porting TCPGecko from the old libraries to the new equivalents. 

* Things related to kernel access may be depreciated and/or redundant to their non-kernel counterparts in the future.
* **Old codes are expected to not work due to RAM offsets, so please do not report stock old codes not working as an issue.**

I am reliant on people testing it and getting back to me on the status of various features. A checklist of what has been reported to work is below. 

Commands:

- [ ] Write 8 bit value
- [ ] Write 16 bit value
- [ ] Write 32 bit value
- [ ] Read memory
- [ ] Read memory with kernel access
- [ ] Validate address range
- [ ] Disassemble range (currently disabled)
- [ ] Disassemble memory
- [ ] Read compressed memory
- [ ] Write with kernel access
- [ ] Read with kernel access 
- [ ] Take a screenshot
- [ ] Upload memory
- [ ] Get the size of data buffer
- [ ] Read a file
- [ ] Read a directory
- [ ] Replace a file
- [ ] "IOSU read file" (currently disabled)
- [ ] Get version hash
- [ ] Get code handler address
- [ ] Read threads
- [ ] Get account identifier
- [ ] Write screen (currently disabled)
- [ ] Follow pointer
- [ ] Get server status
- [ ] Remote procedure call
- [ ] Get symbol
- [ ] Search memory 32
- [ ] Advanced memory search
- [ ] Execute assembly
- [ ] Pause the console
- [ ] Unpause the console
- [ ] See whether the console is paused or not
- [ ] Get server version
- [ ] Get OS version 
- [ ] Set data breakpoint
- [ ] Set instruction breakpoint
- [ ] Toggle breakpoint
- [ ] Remove all breakpoints
- [ ] Get stack trace
- [ ] Poke registers
- [ ] Get entry point address
- [ ] Run kernel copy service
- [ ] Persist assembly
- [ ] Clear assembly

# Credits

## Original TCPGecko credits:

* **dimok** for Homebrew Launcher project engine/base
* **BullyWiiPlaza** for further development of advanced/efficient features of TCPGecko
* **wj44** for porting [`pyGecko`](https://github.com/wiiudev/pyGecko) to the Homebrew Launcher and some development
* **Marionumber1** for exploit development and TCP Gecko Installer contributions
* **NWPlayer123** for the `pyGecko` client library
* **Chadderz** for the original `TCP Gecko Installer`
* **Kinnay** for some `DiiBugger` code this project made use of
* **CosmoCortney** for the original café code handler
* **Mewtality** for various code handler improvements

## aRAMa credits:

* **Maschell** for help on Discord
* **Lulsec2#0064** on Discord, for coming up with the name "aRAMa"
* (The devs of) **FTPiiU**, for implementation referencing
* **Quarky**, for arGecko

# Help, inquiries, etc.

For help with or questions about JGecko U or the original TCPGecko, join [the JGecko U Discord](https://discord.com/invite/rSRM3RWDq4)
