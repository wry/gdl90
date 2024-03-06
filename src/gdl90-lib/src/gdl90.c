//
//  gdl90.c
//  gdl90-lib
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

#include "gdl90.h"

#include <stdio.h>
#include <string.h>

static const uint8_t GDL90_FLAGBYTE = 0x7E;
static const uint8_t GDL90_ESCAPEBYTE = 0x7D;

static inline uint32_t msbu24u32(uint8_t b0, uint8_t b1, uint8_t b2)
{
    return ((uint32_t)b0 << 16) | ((uint32_t)b1 << 8) | (uint32_t)b2;
}

static inline int32_t msbi24i32(uint8_t b0, uint8_t b1, uint8_t b2)
{
    uint32_t ret = msbu24u32(b0, b1, b2);
    if (ret & (1<<23))
    {
        ret |= (uint32_t)0xff<<24;
    }
    return (int32_t)ret;
}

static inline uint16_t msbu12u16(uint8_t b0, uint8_t b1, uint8_t firstByteComplete)
{
    if (firstByteComplete)
    {
        return (uint16_t)((uint16_t)b0 << 4) | ((uint16_t)b1 >> 4);
    }
    else
    {
        return (uint16_t)(((uint16_t)b0 & 0x0f) << 8) | (uint16_t)b1;
    }
}

static inline int16_t msbi12i16(uint8_t b0, uint8_t b1, uint8_t firstByteComplete)
{
    uint16_t ret = msbu12u16(b0, b1, firstByteComplete);
    if (ret & (1<<11))
    {
        ret |= (uint16_t)0xf<<12;
    }
    return (int16_t)ret;
}

static inline int32_t msbi16i32(uint8_t b0, uint8_t b1)
{
    uint32_t ret = ((uint32_t)b0 << 8) | (uint32_t)b1;
    if (ret & (1<<15))
    {
        ret |= (uint32_t)0xffff<<16;
    }
    return (int32_t)ret;
}

GDL90Result GDL90Message_init(GDL90Message *self, const uint8_t *data, const uint16_t dataLength)
{
    if (!self || !data || dataLength < 3) { return GDL90ResultFailure; }

    // unescape gdl90 message
    for (uint16_t j = 1; j < dataLength-1; j++)
    {
        uint8_t b = data[j] == GDL90_ESCAPEBYTE ? data[++j] ^ 0x20 : data[j];
        self->data[self->dataLength++] = b;
    }

    self->id = self->data[0];

    return GDL90ResultOK;
}

GDL90Result GDL90Heartbeat_init(GDL90Heartbeat *self, GDL90Message *gdl90Message)
{
    if (!self || !gdl90Message || gdl90Message->dataLength < 7) { return GDL90ResultFailure; }

    uint8_t *data = &gdl90Message->data[0];

    self->id = data[0];
    self->status1 = data[1];
    self->status2 = data[2];
    self->timestamp = ((uint32_t)(data[2] >> 7) << 16) | ((uint32_t)data[4] << 8) | data[3];
    self->uplinkMessageCount = (uint8_t)(data[5] >> 3);
    self->basicLongMessageCount = (uint16_t)((uint16_t)(data[5] & 0x03) << 8) | (uint16_t)data[6];

    return GDL90ResultOK;
}

char* GDL90Heartbeat_toString(GDL90Heartbeat *self, char *out, size_t len)
{
    if (!self || !out) { return out; }

    snprintf(out, len,
        "GDL90Heartbeat (%p)\n"
        ".id : %d\n"
        ".UATInitialized : %s\n"
        ".RATCS : %s\n"
        ".GPSBatteryLow : %s\n"
        ".AddrType : %s\n"
        ".IDENT : %s\n"
        ".MaintenanceRequired : %s\n"
        ".GSPPosValid : %s\n"
        ".UTCOK : %s\n"
        ".CSANotAvailable : %s\n"
        ".CSARequested : %s\n"
        ".timestamp : %d\n"
        ".uplinkMessageCount : %d\n"
        ".basicLongMessageCount : %d\n"
        , (void*)self
        , self->id
        , (self->status1 & (1<<GDL90HeartbeatStatusByte1BitUATInitialized)) ? "yes" : "no"
        , (self->status1 & (1<<GDL90HeartbeatStatusByte1BitRATCS)) ? "enabled" : "disabled"
        , (self->status1 & (1<<GDL90HeartbeatStatusByte1BitGPSBattLow)) ? "yes" : "no"
        , (self->status1 & (1<<GDL90HeartbeatStatusByte1BitAddrType)) ? "anonymous" : "set/unknown"
        , (self->status1 & (1<<GDL90HeartbeatStatusByte1BitIDENT)) ? "yes" : "no"
        , (self->status1 & (1<<GDL90HeartbeatStatusByte1BitMaintReqd)) ? "yes" : "no"
        , (self->status1 & (1<<GDL90HeartbeatStatusByte1BitGPSPosValid)) ? "yes" : "no"
        , (self->status1 & (1<<GDL90HeartbeatStatusByte2BitUTCOK)) ? "yes" : "no"
        , (self->status1 & (1<<GDL90HeartbeatStatusByte2BitCSANotAvailable)) ? "true" : "false"
        , (self->status1 & (1<<GDL90HeartbeatStatusByte2BitCSARequested)) ? "yes" : "no"
        , self->timestamp
        , self->uplinkMessageCount
        , self->basicLongMessageCount
    );

    return out;
}

GDL90Result GDL90Initialization_init(GDL90Initialization *self, GDL90Message *gdl90Message)
{
    if (!self || !gdl90Message || gdl90Message->dataLength < 3) { return GDL90ResultFailure; }

    uint8_t *data = &gdl90Message->data[0];

    self->id = data[0];
    self->configuration1 = data[1];
    self->configuration2 = data[2];

    return GDL90ResultOK;
}

uint8_t* GDL90Initialization_toBytes(GDL90Initialization *self, uint8_t out[3])
{
    if (!self) { return out; }

    out[0] = self->id;
    out[1] = self->configuration1;
    out[2] = self->configuration2;

    return out;
}

char* GDL90Initialization_toString(GDL90Initialization *self, char *out, size_t len)
{
    if (!self || !out) { return out; }

    snprintf(out, len,
        "GDL90Initialization (%p)\n"
        ".id : %d\n"
        ".cdtiOK : %s\n"
        ".audioInhibit : %s\n"
        ".audioTest : %s\n"
        ".csaDisable : %s\n"
        ".csaAudioDisable : %s\n"
        , (void*)self
        , self->id
        , (self->configuration1 & (1<<GDL90InitializationConfiguration1BitCDTIOK)) ? "yes" : "no"
        , (self->configuration1 & (1<<GDL90InitializationConfiguration1BitAudioInhibit)) ? "supress" : "keep"
        , (self->configuration1 & (1<<GDL90InitializationConfiguration1BitAudioTest)) ? "initiate" : "keep"
        , (self->configuration2 & (1<<GDL90InitializationConfiguration2BitCSADisable)) ? "disable" : "keep"
        , (self->configuration2 & (1<<GDL90InitializationConfiguration2BitCSAAudioDisable)) ? "disable" : "keep"
    );

    return out;
}

GDL90Result GDL90UplinkData_init(GDL90UplinkData *self, GDL90Message *gdl90Message)
{
    if (!self || !gdl90Message || gdl90Message->dataLength < 436) { return GDL90ResultFailure; }

    uint8_t *data = &gdl90Message->data[0];

    self->id = data[0];

    uint32_t tor = msbu24u32(data[3], data[2], data[1]);
    if (tor != 0xffffff)
    {
        self->timeOfReception = tor * 80;
        self->hasValidTor = 1;
    }
    else
    {
        self->timeOfReception = 0;
        self->hasValidTor = 0;
    }

    memcpy(self->payload, data+4, sizeof(self->payload));

    return GDL90ResultOK;
}

char* GDL90UplinkData_toString(GDL90UplinkData *self, char *out, size_t len)
{
    if (!self || !out) { return out; }

    snprintf(out, len,
        "GDL90UplinkData (%p)\n"
        ".id : %d\n"
        ".timeOfReception : %d\n"
        ".payload : (...)\n"
        , (void*)self
        , self->id
        , self->hasValidTor == 1 ? self->timeOfReception : 0
    );

    return out;
}

GDL90Result GDL90HeightAboveTerrain_init(GDL90HeightAboveTerrain *self, GDL90Message *gdl90Message)
{
    if (!self || !gdl90Message || gdl90Message->dataLength < 3) { return GDL90ResultFailure; }

    uint8_t *data = &gdl90Message->data[0];

    self->id = data[0];
    self->heightAboveTerrain = (int16_t)((uint16_t)data[1] << 8 | (uint16_t)data[2]);

    return GDL90ResultOK;
}

uint8_t* GDL90HeightAboveTerrain_toBytes(GDL90HeightAboveTerrain *self, uint8_t out[3])
{
    if (!self) { return out; }

    out[0] = self->id;
    out[1] = (uint8_t)((self->heightAboveTerrain & 0xff00) >> 8);
    out[2] = (uint8_t)(self->heightAboveTerrain & 0xff);

    return out;
}

char* GDL90HeightAboveTerrain_toString(GDL90HeightAboveTerrain *self, char *out, size_t len)
{
    if (!self || !out) { return out; }

    snprintf(out, len,
        "GDL90HeightAboveTerrain (%p)\n"
        ".id : %d\n"
        ".heightAboveTerrain : %d\n"
        , (void*)self
        , self->id
        , self->heightAboveTerrain
    );

    return out;
}

GDL90Result GDL90OwnshipGeometricAltitude_init(GDL90OwnshipGeometricAltitude *self, GDL90Message *gdl90Message)
{
    if (!self || !gdl90Message || gdl90Message->dataLength < 5) { return GDL90ResultFailure; }

    uint8_t *data = &gdl90Message->data[0];

    self->id = data[0];
    self->geoAltitude = msbi16i32(data[1], data[2]) * 5;
    self->verticalWarning = (data[3] & (1<<7)) != 0;
    if ((data[3] & 0x7f) == 0x7f && data[4] == 0xff)
    {
        self->verticalFigureOfMerit = 0;
        self->hasValidVFOM = 0;
    }
    else
    {
        self->verticalFigureOfMerit = (uint16_t)(((uint16_t)data[3] & 0x7f) << 8) | (uint16_t)data[4];
        self->hasValidVFOM = 1;
    }

    return GDL90ResultOK;
}

char* GDL90OwnshipGeometricAltitude_toString(GDL90OwnshipGeometricAltitude *self, char *out, size_t len)
{
    if (!self || !out) { return out; }

    snprintf(out, len,
        "GDL90OwnshipGeometricAltitude (%p)\n"
        ".id : %d\n"
        ".geoAltitude : %d\n"
        ".verticalWarning : %s\n"
        ".verticalFigureOfMerit : %d\n"
        ".hasValidVFOM : %s\n"
        , (void*)self
        , self->id
        , self->geoAltitude
        , self->verticalWarning ? "yes" : "no"
        , self->verticalFigureOfMerit
        , self->hasValidVFOM ? "yes" : "no"
    );

    return out;
}

char* GDL90TrafficReportAlertStatusType_toString(GDL90TrafficReportAlertStatusType alertStatus)
{
    switch (alertStatus)
    {
        case GDL90TrafficReportAlertStatusTypeNoAlert:
            return "No Alert";
        case GDL90TrafficReportAlertStatusTypeTrafficAlert:
            return "Traffic Alert";
        default:
            return "Reserved";
    }
}

char* GDL90TrafficReportAddressType_toString(GDL90TrafficReportAddressType addressType)
{
    switch (addressType)
    {
        case GDL90TrafficReportAddressTypeADSBWithICAO:
            return "ADS-B with ICAO address";
        case GDL90TrafficReportAddressTypeADSBSelfAssigned:
            return "ADS-B with Self-assigned address";
        case GDL90TrafficReportAddressTypeTISBWithICAO:
            return "TIS-B with ICAO address";
        case GDL90TrafficReportAddressTypeTISBWithTrackFileID:
            return "TIS-B with track file ID";
        case GDL90TrafficReportAddressTypeSurfaceVehicle:
            return "Surface Vehicle";
        case GDL90TrafficReportAddressTypeGroundStationBeacon:
            return "Ground Station Beacon";
        default:
            return "Reserved";
    }
}

char* GDL90TrafficReportTrackHeadingType_toString(GDL90TrafficReportTrackHeadingType trackHeadingType)
{
    switch (trackHeadingType)
    {
        case GDL90TrafficReportTrackHeadingTypeInvalid:
            return "Invalid";
        case GDL90TrafficReportTrackHeadingTypeTrueTrackAngle:
            return "True Track Angle";
        case GDL90TrafficReportTrackHeadingTypeHeadingMagnetic:
            return "Heading (Magnetic)";
        case GDL90TrafficReportTrackHeadingTypeHeadingTrue:
            return "Heading (True)";
        default:
            return "Reserved";
    }
}

char* GDL90TrafficReportNICType_toString(GDL90TrafficReportNICType nicType)
{
    switch (nicType)
    {
        case GDL90TrafficReportNICTypeUnknown:
            return "Unknown";
        case GDL90TrafficReportNICTypeLT_20NM:
            return "< 20.0 NM";
        case GDL90TrafficReportNICTypeLT_8NM:
            return "< 8.0 NM";
        case GDL90TrafficReportNICTypeLT_4NM:
            return "< 4.0 NM";
        case GDL90TrafficReportNICTypeLT_2NM:
            return "< 2.0 NM";
        case GDL90TrafficReportNICTypeLT_1NM:
            return "< 1.0 NM";
        case GDL90TrafficReportNICTypeLT_p6NM:
            return "< 0.6 NM";
        case GDL90TrafficReportNICTypeLT_p2NM:
            return "< 0.2 NM";
        case GDL90TrafficReportNICTypeLT_p1NM:
            return "< 0.1 NM";
        case GDL90TrafficReportNICTypeHPL_LT75M_VPL_LT_112M:
            return "HPL < 75 m and VPL < 112 m";
        case GDL90TrafficReportNICTypeHPL_LT25M_VPL_LT_37p5M:
            return "HPL < 25 m and VPL < 37.5 m";
        case GDL90TrafficReportNICTypeHPL_LT7p5M_VPL_LT_11M:
            return "HPL < 7.5 m and VPL < 11 m";
        default:
            return "Unused";
    }
}

char* GDL90TrafficReportNACPType_toString(GDL90TrafficReportNACPType nacpType)
{
    switch (nacpType)
    {
        case GDL90TrafficReportNACPTypeUnknown:
            return "Unknown";
        case GDL90TrafficReportNACPTypeLT_10NM:
            return "< 10.0 NM";
        case GDL90TrafficReportNACPTypeLT_4NM:
            return "< 4.0 NM";
        case GDL90TrafficReportNACPTypeLT_2NM:
            return "< 2.0 NM";
        case GDL90TrafficReportNACPTypeLT_1NM:
            return "< 1.0 NM";
        case GDL90TrafficReportNACPTypeLT_0p5NM:
            return "< 0.5 NM";
        case GDL90TrafficReportNACPTypeLT_0p3NM:
            return "< 0.3 NM";
        case GDL90TrafficReportNACPTypeLT_0p1NM:
            return "< 0.1 NM";
        case GDL90TrafficReportNACPTypeLT_0p05NM:
            return "< 0.05 NM";
        case GDL90TrafficReportNACPTypeHFOM_LT30M_VFOM_LT_45M:
            return "HFOM < 30 m and VFOM < 45 m";
        case GDL90TrafficReportNACPTypeHFOM_LT10M_VFOM_LT_15M:
            return "HFOM < 10 m and VFOM < 15 m";
        case GDL90TrafficReportNACPTypeHFOM_LT3M_VFOM_LT_4M:
            return "HFOM < 3 m and VFOM < 4 m";
        default:
            return "Unused";
    }
}

char* GDL90TrafficReportEmitterCategory_toString(GDL90TrafficReportEmitterCategory category)
{
    switch (category)
    {
        case GDL90TrafficReportEmitterCategoryNoAircraftTypeInformation:
            return "No aircraft type information";
        case GDL90TrafficReportEmitterCategoryLightICAO:
            return "Light (ICAO) < 15 500 lbs";
        case GDL90TrafficReportEmitterCategorySmall:
            return "Small - 15 500 to 75 000 lbs";
        case GDL90TrafficReportEmitterCategoryLarge:
            return "Large - 75 000 to 300 000 lbs";
        case GDL90TrafficReportEmitterCategoryHighVortexLarge:
            return "High Vortex Large (e.g., aircraft such as B757)";
        case GDL90TrafficReportEmitterCategoryHeavyICAO:
            return "Heavy (ICAO) - > 300 000 lbs";
        case GDL90TrafficReportEmitterCategoryHighlyManeuverable:
            return "Highly Maneuverable > 5G acceleration and high speed";
        case GDL90TrafficReportEmitterCategoryRotorcraft:
            return "Rotorcraft";
        case GDL90TrafficReportEmitterCategoryUnassigned8:
            return "Unassigned";
        case GDL90TrafficReportEmitterCategoryGliderSailPlane:
            return "Glider/sailplane";
        case GDL90TrafficReportEmitterCategoryLighterThanAir:
            return "Lighter than air";
        case GDL90TrafficReportEmitterCategoryParachutistSkyDiver:
            return "Parachutist/sky diver";
        case GDL90TrafficReportEmitterCategoryUltraLightHandGliderParaGlider:
            return "Ultra light/hang glider/paraglider";
        case GDL90TrafficReportEmitterCategoryUnassigned13:
            return "Unassigned";
        case GDL90TrafficReportEmitterCategoryUnmannedAerialVehicle:
            return "Unmanned aerial vehicle";
        case GDL90TrafficReportEmitterCategorySpaceTransAtmosphericVehicle:
            return "Space/transatmospheric vehicle";
        case GDL90TrafficReportEmitterCategoryUnassigned16:
            return "Unassigned";
        case GDL90TrafficReportEmitterCategorySurfaceVehicleEmergencyVehicle:
            return "Surface vehicle — emergency vehicle";
        case GDL90TrafficReportEmitterCategorySurfaceVehicleServiceVehicle:
            return "Surface vehicle — service vehicle";
        case GDL90TrafficReportEmitterCategoryPointObstacle:
            return "Point Obstacle (includes tethered balloons)";
        case GDL90TrafficReportEmitterCategoryClusterObstacle:
            return "Cluster Obstacle";
        case GDL90TrafficReportEmitterCategoryLineObstacle:
            return "Line Obstacle";
        default:
            return "Reserved";
    }
}

char* GDL90TrafficReportEmergencyPriorityCodeType_toString(GDL90TrafficReportEmergencyPriorityCodeType epcType)
{
    switch (epcType)
    {
        case GDL90TrafficReportEmergencyPriorityCodeTypeNoEmergency:
            return "No emergency";
        case GDL90TrafficReportEmergencyPriorityCodeTypeGeneralEmergency:
            return "General emergency";
        case GDL90TrafficReportEmergencyPriorityCodeTypeMedicalEmergency:
            return "Medical emergency";
        case GDL90TrafficReportEmergencyPriorityCodeTypeMinimumFuel:
            return "Minimum fuel";
        case GDL90TrafficReportEmergencyPriorityCodeTypeNoCommunication:
            return "No communication";
        case GDL90TrafficReportEmergencyPriorityCodeTypeUnlawfulInterference:
            return "Unlawful interference";
        case GDL90TrafficReportEmergencyPriorityCodeTypeDownedAircraft:
            return "Downed aircraft";
        default:
            return "Reserved";
    }
}

GDL90Result GDL90TrafficReport_init(GDL90TrafficReport *self, GDL90Message *gdl90Message)
{
    if (!self || !gdl90Message || gdl90Message->dataLength < 28) { return GDL90ResultFailure; }

    static const double latlonRes = 180.0 / (double)(1<<23);

    uint8_t *data = &gdl90Message->data[0];

    self->id = data[0];
    self->alertStatus = (data[1] >> 4);
    self->addressType = (data[1] & 0x0f);
    self->participantAddress = msbu24u32(data[2], data[3], data[4]);
    self->latitude = msbi24i32(data[5], data[6], data[7]) * latlonRes;
    self->longitude = msbi24i32(data[8], data[9], data[10]) * latlonRes;
    if (data[11] == 0xff && (data[12] & 0xf0) == 0xf0)
    {
        self->altitude = 0;
        self->hasValidAltitude = 0;
    }
    else
    {
        self->altitude = (int32_t)msbu12u16(data[11], data[12], 1) * 25 - 1000;
        self->hasValidAltitude = 1;
    }

    // 560-1058-00 Rev A - 3.5.1.5 MISCELLANEOUS INDICATORS
    uint8_t miBits = data[12] & 0x0f;

    if (((miBits & (1<<0)) == 0) && ((miBits & (1<<1)) == 0))
    {
        self->trackHeadingType = GDL90TrafficReportTrackHeadingTypeInvalid;
    }
    else if (((miBits & (1<<0)) != 0) && ((miBits & (1<<1)) == 0))
    {
        self->trackHeadingType = GDL90TrafficReportTrackHeadingTypeTrueTrackAngle;
    }
    else if (((miBits & (1<<0)) == 0) && ((miBits & (1<<1)) != 0))
    {
        self->trackHeadingType = GDL90TrafficReportTrackHeadingTypeHeadingMagnetic;
    }
    else if (((miBits & (1<<0)) != 0) && ((miBits & (1<<1)) != 0))
    {
        self->trackHeadingType = GDL90TrafficReportTrackHeadingTypeHeadingTrue;
    }
    
    self->reportStatus = ((miBits & (1<<2)) != 0);
    self->airGroundState = ((miBits & (1<<3)) != 0);

    self->navigationIntegrityCategory = data[13] >> 4;
    self->navigationAccuracyCategoryForPosition = data[13] & 0x0f;
    if (data[14] == 0xff && (data[15] & 0xf0) == 0x00)
    {
        self->horizontalVelocity = 0;
        self->hasValidAltitude = 0;
    }
    else
    {
        self->horizontalVelocity = (uint32_t)msbu12u16(data[14], data[15], 1);
        self->hasValidAltitude = 1;
    }
    if ((data[15] & 0x0f) == 0x08 && data[16] == 0x00)
    {
        self->verticalVelocity = 0;
        self->hasValidVerticalVelocity = 0;
    }
    else
    {
        self->verticalVelocity = (int32_t)msbi12i16(data[15], data[16], 0) * 64;
        self->hasValidVerticalVelocity = 1;
    }
    self->trackHeading = (double)(data[17]) * (360.0/256.0);
    self->emitterCategory = data[18];
    memset(self->callsign, 0, 8);
    for (size_t i = 0; i < sizeof(self->callsign); i++)
    {
        if (data[i+19] == 0 || data[i+19] == 0x20)
        {
            break;
        }
        self->callsign[i] = (char)data[i+19];
    }
    self->emergencyPriorityCode = data[27] >> 4;
    self->spare = data[27] & 0x0f;

    if (
        msbi24i32(data[5], data[6], data[7]) != 0
        || msbi24i32(data[8], data[9], data[10]) == 0
        || self->navigationIntegrityCategory == GDL90TrafficReportEmitterCategoryNoAircraftTypeInformation
    )
    {
        self->hasValidPosition = 1;
    }

    return GDL90ResultOK;
}

char* GDL90TrafficReport_toString(GDL90TrafficReport *self, char *out, size_t len)
{
    if (!self || !out) { return out; }

    snprintf(out, len,
        "%s (%p)\n"
        ".id : %d\n"
        ".alertStatus : %s\n"
        ".addressType : %s\n"
        ".participantAddress : %o\n"
        ".latitude : %f\n"
        ".longitude : %f\n"
        ".altitude : %d\n"
        ".track/heading : %s\n"
        ".report : %s\n"
        ".aircraft is : %s\n"
        ".navigationIntegrityCategory : %s\n"
        ".navigationAccuracyCategoryForPosition : %s\n"
        ".horizontalVelocity : %d\n"
        ".verticalVelocity : %d\n"
        ".trackHeading : %f\n"
        ".emitterCategory : %s\n"
        ".callsign : '%s'\n"
        ".priorityCode : %s\n"
        ".spare : %d\n"
        , self->id == GDL90MessageType_TrafficReport ? "GDL90TrafficReport" : "GDL90OwnshipReport"
        , (void*)self
        , self->id
        , GDL90TrafficReportAlertStatusType_toString(self->alertStatus)
        , GDL90TrafficReportAddressType_toString(self->addressType)
        , self->participantAddress
        , self->latitude
        , self->longitude
        , self->altitude
        , GDL90TrafficReportTrackHeadingType_toString(self->trackHeadingType)
        , self->reportStatus ? "Extrapolated" : "Updated"
        , self->airGroundState ? "Airborne" : "On Ground"
        , GDL90TrafficReportNICType_toString(self->navigationIntegrityCategory)
        , GDL90TrafficReportNACPType_toString(self->navigationAccuracyCategoryForPosition)
        , self->horizontalVelocity
        , self->verticalVelocity
        , self->trackHeading
        , GDL90TrafficReportEmitterCategory_toString(self->emitterCategory)
        , self->callsign[0] == 0 ? "(unknown)" : self->callsign
        , GDL90TrafficReportEmergencyPriorityCodeType_toString((GDL90TrafficReportEmergencyPriorityCodeType)self->emergencyPriorityCode)
        , self->spare
    );

    return out;
}

GDL90Result GDL90BasicReport_init(GDL90BasicReport *self, GDL90Message *gdl90Message)
{
    if (!self || !gdl90Message || gdl90Message->dataLength < 22) { return GDL90ResultFailure; }

    uint8_t *data = &gdl90Message->data[0];

    self->id = data[0];

    uint32_t tor = msbu24u32(data[3], data[2], data[1]);
    if (tor != 0xffffff)
    {
        self->timeOfReception = tor * 80;
        self->hasValidTor = 1;
    }
    else
    {
        self->timeOfReception = 0;
        self->hasValidTor = 0;
    }

    memcpy(self->payload, data+4, sizeof(self->payload));

    return GDL90ResultOK;
}

char* GDL90BasicReport_toString(GDL90BasicReport *self, char *out, size_t len)
{
    if (!self || !out) { return out; }

    snprintf(out, len,
        "GDL90BasicReport (%p)\n"
        ".id : %d\n"
        ".timeOfReception : %d\n"
        ".hasValidTor : %s\n"
        , (void*)self
        , self->id
        , self->timeOfReception
        , self->hasValidTor ? "yes" : "no"
    );

    return out;
}

GDL90Result GDL90LongReport_init(GDL90LongReport *self, GDL90Message *gdl90Message)
{
    if (!self || !gdl90Message || gdl90Message->dataLength < 38) { return GDL90ResultFailure; }

    uint8_t *data = &gdl90Message->data[0];

    self->id = data[0];

    uint32_t tor = msbu24u32(data[3], data[2], data[1]);
    if (tor != 0xffffff)
    {
        self->timeOfReception = tor * 80;
        self->hasValidTor = 1;
    }
    else
    {
        self->timeOfReception = 0;
        self->hasValidTor = 0;
    }

    memcpy(self->payload, data+4, sizeof(self->payload));

    return GDL90ResultOK;
}

char* GDL90LongReport_toString(GDL90LongReport *self, char *out, size_t len)
{
    if (!self || !out) { return out; }

    snprintf(out, len,
        "GDL90LongReport (%p)\n"
        ".id : %d\n"
        ".timeOfReception : %d\n"
        ".hasValidTor : %s\n"
        , (void*)self
        , self->id
        , self->timeOfReception
        , self->hasValidTor ? "yes" : "no"
    );

    return out;
}

GDL90Result GDL90CRC_init(GDL90CRC *self)
{
    if (!self) { return GDL90ResultFailure; }

    uint16_t i, bitctr, crc;
    for (i = 0; i < 256; i++)
    {
        crc = (uint16_t)(i << 8);
        for (bitctr = 0; bitctr < 8; bitctr++)
        {
            crc = (uint16_t)(crc << 1) ^ ((crc & 0x8000) ? 0x1021 : 0);
        }
        self->crc16Table[i] = crc;
    }

    return GDL90ResultOK;
}

GDL90Result GDL90CRC_crc(GDL90CRC *self, uint16_t *outCrc, uint8_t *data, size_t len)
{
    if (!self || !data) { return GDL90ResultFailure; }

    uint32_t i;
    uint16_t crc = 0;
    for (i = 0; i < len; i++)
    {
        crc = self->crc16Table[crc >> 8] ^ (uint16_t)(crc << 8) ^ data[i];
    }

    *outCrc = crc;

    return GDL90ResultOK;
}

GDL90CRCResult GDL90CRC_isValid(GDL90CRC *self, uint8_t *data, size_t len)
{
    if (!self || !data || len < 3) { return GDL90CRCResultInvalidInput; }

    uint16_t crcInData = (uint16_t)(((uint16_t)data[len-1]) << 8) | (uint16_t)data[len-2];
    uint16_t crcCalculated = 0;
    
    if (GDL90CRC_crc(self, &crcCalculated, data, len-2) != GDL90ResultOK)
    {
        return GDL90CRCResultInvalidInput;
    }
    else if (crcCalculated != crcInData)
    {
        return GDL90CRCResultMismatch;
    }

    return GDL90CRCResultOK;
}

GDL90Result GDL90StreamConfig_init(GDL90StreamConfig *self, GDL90StreamMessageHandler *messageHandler, GDL90StreamErrorHandler *errorHandler)
{
    if (!self || !messageHandler || !errorHandler) { return GDL90ResultFailure; }

    self->messageHandler = messageHandler;
    self->errorHandler = errorHandler;

    return GDL90ResultOK;
}

GDL90Result GDL90Stream_init(GDL90Stream *self, GDL90StreamConfig *config)
{
    if (!self || !config) { return GDL90ResultFailure; }

    self->config = *config;

    GDL90CRC_init(&self->crc);

    return GDL90ResultOK;
}

GDL90Result GDL90Stream_process(GDL90Stream *self, const uint8_t *data, const uint16_t dataLength)
{
    if (!self || !data || !self->config.errorHandler || !self->config.messageHandler) { return GDL90ResultFailure; }

    size_t curMessageOffset = 0;
    uint16_t curMessageLength = 1;
    for (size_t i = 0; i < dataLength; i++)
    {
        if (data[i] == GDL90_FLAGBYTE && i > 0 && curMessageLength > 1)
        {
            GDL90Message gdl90Message = {0};
            if (GDL90Message_init(&gdl90Message, &data[curMessageOffset], curMessageLength) != GDL90ResultOK)
            {
                self->config.errorHandler(&gdl90Message, GDL90StreamProcessingErrorInvalidMessage);
            }
            else
            {
                GDL90CRCResult crcResult = GDL90CRC_isValid(&self->crc, gdl90Message.data, gdl90Message.dataLength);
                if (crcResult != GDL90CRCResultOK)
                {
                    self->config.errorHandler(&gdl90Message, GDL90StreamProcessingErrorCRCError);
                }
                else if (GDL90Stream_handleUnescapedMessage(self, &gdl90Message) != GDL90ResultOK)
                {
                    self->config.errorHandler(&gdl90Message, GDL90StreamProcessingErrorInvalidMessage);
                }
            }
            
            curMessageOffset = i+1;
            curMessageLength = 0;
        }
        curMessageLength++;
    }

    return GDL90ResultOK;
}

GDL90Result GDL90Stream_handleUnescapedMessage(GDL90Stream *self, GDL90Message* gdl90Message)
{
    if (!self || !gdl90Message || !self->config.errorHandler || !self->config.messageHandler) { return GDL90ResultFailure; }

    switch (gdl90Message->id)
    {
        case GDL90MessageType_Heartbeat:
        {
            GDL90Heartbeat gdl90Heartbeat = {0};
            if (GDL90Heartbeat_init(&gdl90Heartbeat, gdl90Message) != GDL90ResultOK)
            {
                self->config.errorHandler(gdl90Message, GDL90StreamProcessingErrorInvalidMessage);
            }
            else
            {
                self->config.messageHandler(gdl90Message, &gdl90Heartbeat);
            }
            break;
        }
        case GDL90MessageType_Initialization:
        {
            GDL90Initialization gdl90Initialization = {0};
            if (GDL90Initialization_init(&gdl90Initialization, gdl90Message) != GDL90ResultOK)
            {
                self->config.errorHandler(gdl90Message, GDL90StreamProcessingErrorInvalidMessage);
            }
            else
            {
                self->config.messageHandler(gdl90Message, &gdl90Initialization);
            }
            break;
        }
        case GDL90MessageType_UplinkData:
        {
            GDL90UplinkData gdl90UplinkData = {0};
            if (GDL90UplinkData_init(&gdl90UplinkData, gdl90Message) != GDL90ResultOK)
            {
                self->config.errorHandler(gdl90Message, GDL90StreamProcessingErrorInvalidMessage);
            }
            else
            {
                self->config.messageHandler(gdl90Message, &gdl90UplinkData);
            }
            break;
        }
        case GDL90MessageType_HeightAboveTerrain:
        {
            GDL90HeightAboveTerrain gdl0HeightAboveTerrain = {0};
            if (GDL90HeightAboveTerrain_init(&gdl0HeightAboveTerrain, gdl90Message) != GDL90ResultOK)
            {
                self->config.errorHandler(gdl90Message, GDL90StreamProcessingErrorInvalidMessage);
            }
            else
            {
                self->config.messageHandler(gdl90Message, &gdl0HeightAboveTerrain);
            }
            break;
        }
        case GDL90MessageType_OwnshipGeometricAltitude:
        {
            GDL90OwnshipGeometricAltitude gdl90OwnshipGeometricAltitude = {0};
            if (GDL90OwnshipGeometricAltitude_init(&gdl90OwnshipGeometricAltitude, gdl90Message) != GDL90ResultOK)
            {
                self->config.errorHandler(gdl90Message, GDL90StreamProcessingErrorInvalidMessage);
            }
            else
            {
                self->config.messageHandler(gdl90Message, &gdl90OwnshipGeometricAltitude);
            }
            break;
        }
        case GDL90MessageType_OwnshipReport:
        case GDL90MessageType_TrafficReport:
        {
            GDL90TrafficReport gdl90TrafficReport = {0};
            if (GDL90TrafficReport_init(&gdl90TrafficReport, gdl90Message) != GDL90ResultOK)
            {
                self->config.errorHandler(gdl90Message, GDL90StreamProcessingErrorInvalidMessage);
            }
            else
            {
                self->config.messageHandler(gdl90Message, &gdl90TrafficReport);
            }
            break;
        }
        case GDL90MessageType_BasicReport:
        {
            GDL90BasicReport gdl90BasicReport = {0};
            if (GDL90BasicReport_init(&gdl90BasicReport, gdl90Message) != GDL90ResultOK)
            {
                self->config.errorHandler(gdl90Message, GDL90StreamProcessingErrorInvalidMessage);
            }
            else
            {
                self->config.messageHandler(gdl90Message, &gdl90BasicReport);
            }
            break;
        }
        case GDL90MessageType_LongReport:
        {
            GDL90LongReport gdl90LongReport = {0};
            if (GDL90LongReport_init(&gdl90LongReport, gdl90Message) != GDL90ResultOK)
            {
                self->config.errorHandler(gdl90Message, GDL90StreamProcessingErrorInvalidMessage);
            }
            else
            {
                self->config.messageHandler(gdl90Message, &gdl90LongReport);
            }
            break;
        }
        default:
        {
            self->config.errorHandler(gdl90Message, GDL90StreamProcessingErrorUnknownMessageType);
            break;
        }
    }

    return GDL90ResultOK;
}
