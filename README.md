
# libgdl90

A lightweight [GDL90](https://www.faa.gov/sites/faa.gov/files/air_traffic/technology/adsb/archival/GDL90_Public_ICD_RevA.PDF) library that aims to stay easy to use yet portable.

## Features

* C99
* No third party dependency
* No dynamic memory allocation
* Unit tests (though currently only for some of the message types)
* Cmake and modular project hierarchy for convenience
* Consists of a single source + header
* Examples, to get you going
* String output of all message types

## Purpose

The lib implements the GDL90 `display` role. Albeit it can process all of the message types mentioned in the official doc, including those meant to be sent by displays, it does not decode anything that isn't strictly part of the protocol (eg. Uplink Data payloads).

## Compilation

* Create the build files using `cmake` (or your IDE)
* Build the project (eg. `make` or your IDE)
* The following targets will be created :
  * `libgdl90.a`
  * `gdl90-cli`
  * `gdl90-tests`

## Use

The easiest way to see how to use it is taking a peek inside the `gdl90-cli` project contained within this project.

* Add the `gdl90.c` and `gdl90.h` files to your project
* Create a `GDL90StreamConfig` instance on the stack/heap and initialize it with the callbacks, calling `GDL90StreamConfig_init(...)`
* Create a `GDL90Stream` instance on the stack/heap and initialize it with the config you created earlier
* For each GDL90 packet (containing one or more messages) call `GDL90Stream_process(&gdl90Stream, packet, packetLength)`
* You get the GDL90 message instances in the callback you set up earlier (note: if you need to own them, copy them)
* The `0x7e` GDL90 flag bytes and CRC are intentionally only checked in `GDL90Stream`, so if you have a custom protocol you can use `GDL90Message` directly (note: `gdl90-cli` uses `GDL90Stream`, so non-conformant packets won't work with it)

## Example projects

### gdl90-cli

A simple command line decoder of hex string encoded GDL90 data to show the use of the lib in a C/C++/etc project.

Use :

```
echo "7e......7e" | gdl90-cli
```

If you have GDL90 data flowing either through a network interface or stored in a [libpcap](https://www.tcpdump.org/) packet capture, you can use [captail](https://github.com/wry/snippets/tree/master/captail) to process it, eg.:

```
tcpdump -X -v -r gdl90.pcap | captail.pl | cut -c 57- | gdl90-cli
```

### gdl90-wasm

A simple - and only partially implemented - decoder to show the use of the lib in a wasm environment. It intentionally avoids the use of emscripten to highlight the portability aspect of the lib, but that's by no means to discourage the use of it.

As I don't actually use wasm, I've only ever tested this example through the most unlikely implementation - a web project. Guess you'll tell me if you try it in a more useful environment.

Use :
* Create the build files using `emcmake cmake`
* Build the project with `make`
* Copy the `gdl90.wasm`, `/assets/gdl90.js` and `/assets/index.html` in a directory
* Serve the files using a web server, eg. `python3 -m http.server` 
* After connecting it should display the two test GDL90 messages through `console.log`

## License

The library and all of its components use the MIT license.