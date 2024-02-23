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

#define GDL90F(CLASS,MEMBER) \
__attribute__((used)) auto CLASS ## _ ## MEMBER(CLASS * cl) -> decltype(+cl->MEMBER) { return cl->MEMBER; }

GDL90F(GDL90TrafficReport, id)
GDL90F(GDL90TrafficReport, alertStatus)
GDL90F(GDL90TrafficReport, addressType)
GDL90F(GDL90TrafficReport, participantAddress)
GDL90F(GDL90TrafficReport, latitude)
GDL90F(GDL90TrafficReport, longitude)
GDL90F(GDL90TrafficReport, altitude)
GDL90F(GDL90TrafficReport, trackHeadingType)
GDL90F(GDL90TrafficReport, reportStatus)
GDL90F(GDL90TrafficReport, airGroundState)
GDL90F(GDL90TrafficReport, navigationIntegrityCategory)
GDL90F(GDL90TrafficReport, navigationAccuracyCategoryForPosition)
GDL90F(GDL90TrafficReport, horizontalVelocity)
GDL90F(GDL90TrafficReport, verticalVelocity)
GDL90F(GDL90TrafficReport, trackHeading)
GDL90F(GDL90TrafficReport, emitterCategory)
GDL90F(GDL90TrafficReport, callsign)
GDL90F(GDL90TrafficReport, emergencyPriorityCode)
GDL90F(GDL90TrafficReport, spare)
GDL90F(GDL90TrafficReport, hasValidAltitude)
GDL90F(GDL90TrafficReport, hasValidHorizontalVelocity)
GDL90F(GDL90TrafficReport, hasValidVerticalVelocity)
GDL90F(GDL90TrafficReport, hasValidPosition)

GDL90F(GDL90OwnshipGeometricAltitude, id)
GDL90F(GDL90OwnshipGeometricAltitude, verticalWarning)
GDL90F(GDL90OwnshipGeometricAltitude, hasValidVFOM)
GDL90F(GDL90OwnshipGeometricAltitude, verticalFigureOfMerit)
GDL90F(GDL90OwnshipGeometricAltitude, geoAltitude)

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
