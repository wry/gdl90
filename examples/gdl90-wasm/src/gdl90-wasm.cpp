//
//  gdl90-wasm.cpp
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

#include <gdl90.h>

#include <cstdint>
#include <cstddef>
#include <cstring>

#ifdef __cplusplus
extern "C" {
#endif

static GDL90Stream gdl90Stream = {};

__attribute__((used))
uint8_t gdl90StreamBuffer[1024] = {};

extern void EXT_write_stdout(const char *str, size_t len);
extern void EXT_handleGDL90Message(GDL90Message *gdl90Message, void *message);
extern void EXT_handleGDL90Error(GDL90Message *gdl90Message, GDL90StreamProcessingError error);

void GDL90_write_stdout(const char * str)
{
    EXT_write_stdout(str, strlen(str));
}

#define GDL90_SYNTHESIZE(CLASS,MEMBER) \
__attribute__((used)) auto CLASS ## _ ## MEMBER(CLASS * cl) -> decltype(+cl->MEMBER) { return cl->MEMBER; }

GDL90_SYNTHESIZE(GDL90TrafficReport, id)
GDL90_SYNTHESIZE(GDL90TrafficReport, alertStatus)
GDL90_SYNTHESIZE(GDL90TrafficReport, addressType)
GDL90_SYNTHESIZE(GDL90TrafficReport, participantAddress)
GDL90_SYNTHESIZE(GDL90TrafficReport, latitude)
GDL90_SYNTHESIZE(GDL90TrafficReport, longitude)
GDL90_SYNTHESIZE(GDL90TrafficReport, altitude)
GDL90_SYNTHESIZE(GDL90TrafficReport, trackHeadingType)
GDL90_SYNTHESIZE(GDL90TrafficReport, reportStatus)
GDL90_SYNTHESIZE(GDL90TrafficReport, airGroundState)
GDL90_SYNTHESIZE(GDL90TrafficReport, navigationIntegrityCategory)
GDL90_SYNTHESIZE(GDL90TrafficReport, navigationAccuracyCategoryForPosition)
GDL90_SYNTHESIZE(GDL90TrafficReport, horizontalVelocity)
GDL90_SYNTHESIZE(GDL90TrafficReport, verticalVelocity)
GDL90_SYNTHESIZE(GDL90TrafficReport, trackHeading)
GDL90_SYNTHESIZE(GDL90TrafficReport, emitterCategory)
GDL90_SYNTHESIZE(GDL90TrafficReport, callsign)
GDL90_SYNTHESIZE(GDL90TrafficReport, emergencyPriorityCode)
GDL90_SYNTHESIZE(GDL90TrafficReport, spare)
GDL90_SYNTHESIZE(GDL90TrafficReport, hasValidAltitude)
GDL90_SYNTHESIZE(GDL90TrafficReport, hasValidHorizontalVelocity)
GDL90_SYNTHESIZE(GDL90TrafficReport, hasValidVerticalVelocity)
GDL90_SYNTHESIZE(GDL90TrafficReport, hasValidPosition)

GDL90_SYNTHESIZE(GDL90OwnshipGeometricAltitude, id)
GDL90_SYNTHESIZE(GDL90OwnshipGeometricAltitude, verticalWarning)
GDL90_SYNTHESIZE(GDL90OwnshipGeometricAltitude, hasValidVFOM)
GDL90_SYNTHESIZE(GDL90OwnshipGeometricAltitude, verticalFigureOfMerit)
GDL90_SYNTHESIZE(GDL90OwnshipGeometricAltitude, geoAltitude)

__attribute__((used))
void GDL90_init()
{
    GDL90StreamConfig gdl90StreamConfig = {};
    if (GDL90StreamConfig_init(&gdl90StreamConfig, EXT_handleGDL90Message, EXT_handleGDL90Error) != 0)
    {
        GDL90_write_stdout("failed to initialize GDL90Stream");
        return;
    }

    if (GDL90Stream_init(&gdl90Stream, &gdl90StreamConfig) != 0)
    {
        GDL90_write_stdout("failed to initialize GDL90Stream");
        return;
    }
}

__attribute__((used))
void GDL90_processData(uint16_t packetLength)
{
    if (GDL90Stream_process(&gdl90Stream, gdl90StreamBuffer, packetLength) != 0)
    {
        GDL90_write_stdout("GDL90Stream processing failed");
    }
}

#ifdef __cplusplus
}
#endif
