//
//  gdl90.js
//  gdl90-wasm
//
// Copyright (c) 2024 wry
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

class GDL90 {
    constructor(gdl90Instance) {
        this.native = gdl90Instance.exports;
        this.memory = gdl90Instance.exports.memory;
    }

    // gdl90 wasm module imports

    static imports() {
        return {
            "env": {
                EXT_write_stdout(str, len) {
                    const text = new TextDecoder().decode(new Uint8Array(gdl90.memory.buffer, str, len));
                    console.log(`${text}`);
                },
                EXT_handleGDL90Message(/* GDL90Message* */_, /* void* */data) {
                    switch (new DataView(gdl90.memory.buffer, data, 1).getUint8(0))
                    {
                        case 0x14:
                            console.log(new GDL90TrafficReport(data));
                            break;
                        case 0x0b:
                            console.log(new GDL90OwnshipGeometricAltitude(data));
                            break;
                    }
                },
                EXT_handleGDL90Error(/* GDL90Message* */_, /* GDL90StreamProcessingError */ error) {
                    console.error(`Failed to process message : ${error}`);
                }
            }
        };
    }

    // gdl90 wasm module calls

    init() {
        this.native.GDL90_init();
    }

    processData(data) {
        // set data in gdl90 instance's buffer
        let gdl90StreamBuffer = new Uint8Array(this.memory.buffer, this.native.gdl90StreamBuffer, data.length);
        gdl90StreamBuffer.set(data);

        this.native.GDL90_processData(data.length);
    }

    // gdl90 tests

    testGDL90TrafficReport() {
        const data = [
            // GDL90 Flag
            0x7e,
            // GDL90Message ID
            0x14,
            // GDL90TrafficReport
            0x00, 0xAB, 0x45, 0x49, 0x1F, 0xEF, 0x15, 0xA8, 0x89, 0x78,
            0x0F, 0x09, 0xA9, 0x07, 0xB0, 0x01, 0x20, 0x01, 0x4E, 0x38,
            0x32, 0x35, 0x56, 0x20, 0x20, 0x20, 0x00,
            // GDL90CRC
            0x57, 0xd6, 
            // GDL90 Flag
            0x7e
        ];
        this.processData(data);
    }

    testGDL90OwnshipGeometricAltitude() {
        const data = [
            // GDL90 Flag
            0x7e,
            // GDL90Message ID
            0x0b,
            // GDL90OwnshipGeometricAltitude
            0xff, 0xff, 0xff, 0xff,
            // GDL90CRC (fake)
            0x01, 0x12,
            // GDL90 Flag
            0x7e
        ];
        this.processData(data);
    }
}

class GDL90TrafficReport {
    constructor(data) {
        this.id = gdl90.native[`${this.constructor.name}_id`](data);
        this.alertStatus = gdl90.native[`${this.constructor.name}_alertStatus`](data);
        this.addressType = gdl90.native[`${this.constructor.name}_addressType`](data);
        this.participantAddress = gdl90.native[`${this.constructor.name}_participantAddress`](data);
        this.latitude = gdl90.native[`${this.constructor.name}_latitude`](data);
        this.longitude = gdl90.native[`${this.constructor.name}_longitude`](data);
        this.altitude = gdl90.native[`${this.constructor.name}_altitude`](data);
        this.trackHeadingType = gdl90.native[`${this.constructor.name}_trackHeadingType`](data);
        this.reportStatus = gdl90.native[`${this.constructor.name}_reportStatus`](data);
        this.airGroundState = gdl90.native[`${this.constructor.name}_airGroundState`](data);
        this.navigationIntegrityCategory = gdl90.native[`${this.constructor.name}_navigationIntegrityCategory`](data);
        this.navigationAccuracyCategoryForPosition = gdl90.native[`${this.constructor.name}_navigationAccuracyCategoryForPosition`](data);
        this.horizontalVelocity = gdl90.native[`${this.constructor.name}_horizontalVelocity`](data);
        this.verticalVelocity = gdl90.native[`${this.constructor.name}_verticalVelocity`](data);
        this.trackHeading = gdl90.native[`${this.constructor.name}_trackHeading`](data);
        this.emitterCategory = gdl90.native[`${this.constructor.name}_emitterCategory`](data);
        const callsign = gdl90.native[`${this.constructor.name}_callsign`](data);
        this.callsign = new TextDecoder().decode(new Uint8Array(gdl90.memory.buffer, callsign, 8)).replace(/\0/g, '');
        this.emergencyPriorityCode = gdl90.native[`${this.constructor.name}_emergencyPriorityCode`](data);
        this.spare = gdl90.native[`${this.constructor.name}_spare`](data);
        this.hasValidAltitude = gdl90.native[`${this.constructor.name}_hasValidAltitude`](data);
        this.hasValidHorizontalVelocity = gdl90.native[`${this.constructor.name}_hasValidHorizontalVelocity`](data);
        this.hasValidVerticalVelocity = gdl90.native[`${this.constructor.name}_hasValidVerticalVelocity`](data);
        this.hasValidPosition = gdl90.native[`${this.constructor.name}_hasValidPosition`](data);
    }
}

class GDL90OwnshipGeometricAltitude {
    constructor(data) {
        this.id = gdl90.native[`${this.constructor.name}_id`](data)
        this.verticalWarning = gdl90.native[`${this.constructor.name}_verticalWarning`](data)
        this.hasValidVFOM = gdl90.native[`${this.constructor.name}_hasValidVFOM`](data)
        this.verticalFigureOfMerit = gdl90.native[`${this.constructor.name}_verticalFigureOfMerit`](data)
        this.geoAltitude = gdl90.native[`${this.constructor.name}_geoAltitude`](data)
    }
}

async function main() {
    WebAssembly.instantiateStreaming(
        fetch("gdl90.wasm"),
        GDL90.imports()
    ).then(result => {
        gdl90 = new GDL90(result.instance);
        gdl90.init();
        gdl90.testGDL90TrafficReport();
        gdl90.testGDL90OwnshipGeometricAltitude();
    });
}

main().catch(e => {
    console.error(`${e}\n\nStack:\n${e.stack}`);
});
