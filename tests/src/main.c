//
//  gdl90-tests.c
//  gdl90-tests
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

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// https://www.faa.gov/sites/faa.gov/files/air_traffic/technology/adsb/archival/GDL90_Public_ICD_RevA.PDF

#define UpdateGDL90Bytes2(CLASS,INSTANCE,IDX,B0,B1) do {\
    gdl90Message.data[IDX+0] = B0; \
    gdl90Message.data[IDX+1] = B1; \
    assert(CLASS ## _init(&INSTANCE, &gdl90Message) == GDL90ResultOK); \
} while(0)

#define UpdateGDL90Bytes3(CLASS,INSTANCE,IDX,B0,B1,B2) do {\
    gdl90Message.data[IDX+0] = B0; \
    gdl90Message.data[IDX+1] = B1; \
    gdl90Message.data[IDX+2] = B2; \
    assert(CLASS ## _init(&INSTANCE, &gdl90Message) == GDL90ResultOK); \
} while(0)

static void testGDL90Heartbeat(void)
{
    // 3.1. HEARTBEAT MESSAGE

    uint8_t data[] = {
        // GDL90 Flag
        0x7e,
        // GDL90Message ID
        0x00,
        // Status Byte 1, Bit 7: GPS Pos Valid, Bit 3: GPS Batt Low 
        ((1<<7) | (1<<3)),
        // Status Byte 2, Bit 7: Time Stamp (MS bit), Bit 0: UTC OK
        ((1<<7) | (1<<0)),
        // Time Stamp
        0x0f, 0xff,
        // Message Counts
        0x01, 0xff,
        // GDL90CRC (fake)
        0x00, 0x00,
        // GDL90 Flag
        0x7e
    };

    GDL90Message gdl90Message = {0};
    assert(GDL90Message_init(&gdl90Message, data, sizeof(data)) == GDL90ResultOK);

    GDL90Heartbeat gdl90Heartbeat = {0};
    assert(GDL90Heartbeat_init(&gdl90Heartbeat, &gdl90Message) == GDL90ResultOK);
    
    // 3.1.1. Status Byte 1, Bit 7: GPS Pos Valid, Bit 3: GPS Batt Low 
    assert(gdl90Heartbeat.status1 & (1<<GDL90HeartbeatStatusByte1BitGPSBattLow));
    assert(gdl90Heartbeat.status1 & (1<<GDL90HeartbeatStatusByte1BitGPSPosValid));

    // 3.1.2. Status Byte 2, Bit 0: UTC OK 
    assert(gdl90Heartbeat.status2 & (1<<GDL90HeartbeatStatusByte2BitUTCOK));

    // 3.1.3. UAT Time Stamp 

    // 3.1.4. Received Message Counts 
}

static void testGDL90Initialization(void)
{
    // 3.2. INITIALIZATION MESSAGE

    GDL90Initialization gdl90Initialization = {0};

    gdl90Initialization.configuration1 |= 1<<GDL90InitializationConfiguration1BitCDTIOK;
    gdl90Initialization.configuration1 |= 1<<GDL90InitializationConfiguration1BitAudioTest;

    gdl90Initialization.configuration2 |= 1<<GDL90InitializationConfiguration2BitCSAAudioDisable;

    uint8_t data[3] = {0};
    (void)GDL90Initialization_toBytes(&gdl90Initialization, data);
    
    // Configuration Byte 1
    assert((data[1] & (1<<0)) != 0); // CDTI OK
    assert((data[1] & (1<<1)) == 0); // Audio Inhibit
    assert((data[1] & (1<<6)) != 0); // Audio Test
    
    // Configuration Byte 2
    assert((data[2] & (1<<0)) == 0);
    assert((data[2] & (1<<1)) != 0);
}

static void testGDL90UplinkData(void)
{
    // 3.3. UPLINK DATA MESSAGE

    uint8_t data[436+4] = {0};

    // GDL90 Flag
    data[0] = 0x7e;
    // GDL90Message ID
    data[1] = 0x07;
    // Time of Reception
    data[2] = 0x00;
    data[3] = 0x00;
    data[4] = 0x00;
    // Uplink Payload
    // ...
    // GDL90CRC (fake)
    data[437] = 0x00;
    data[438] = 0x00;
    // GDL90 Flag
    data[439] = 0x7e;

    GDL90Message gdl90Message = {0};
    assert(GDL90Message_init(&gdl90Message, data, sizeof(data)) == GDL90ResultOK);

    GDL90UplinkData gdl90UplinkData = {0};
    assert(GDL90UplinkData_init(&gdl90UplinkData, &gdl90Message) == GDL90ResultOK);

    // 3.3.1. Time of Reception (TOR)

    // A TOR of "all ONES" (0xFFFFFF, or 16,777,21510) indicates that the TOR value is not valid
    UpdateGDL90Bytes3(GDL90UplinkData, gdl90UplinkData, 1, 0xff, 0xff, 0xff);
    assert(!gdl90UplinkData.hasValidTor);

    // 3.3.2. Uplink Payload (not implemented; needs RTCA/DO-282)
}

static void testGDL90HeightAboveTerrain(void)
{
    // 3.7. HEIGHT ABOVE TERRAIN (Example)

    uint8_t data[] = {
        // GDL90 Flag
        0x7e,
        // GDL90Message ID
        0x09,
        // GDL90HeightAboveTerrain
        0x01, 0x00,
        // GDL90CRC (fake)
        0x00, 0x00,
        // GDL90 Flag
        0x7e
    };

    GDL90Message gdl90Message = {0};
    assert(GDL90Message_init(&gdl90Message, data, sizeof(data)) == GDL90ResultOK);

    GDL90HeightAboveTerrain gdl90HeightAboveTerrain = {0};
    assert(GDL90HeightAboveTerrain_init(&gdl90HeightAboveTerrain, &gdl90Message) == GDL90ResultOK);

    // If Byte 2 is 0x01, and Byte 3 is 0x00, this represents a Height Above Terrain of 256 feet.
    assert(gdl90HeightAboveTerrain.heightAboveTerrain == 256);
    
    // Special Value: The value 0x8000 indicates that the Height Above Terrain data is invalid.
    UpdateGDL90Bytes2(GDL90HeightAboveTerrain, gdl90HeightAboveTerrain, 1, 0x80, 0x00);
    assert(gdl90HeightAboveTerrain.heightAboveTerrain == -32768);
}

static void testGDL90OwnshipGeometricAltitude(void)
{
    // 3.8. OWNSHIP GEOMETRIC ALTITUDE MESSAGE

    uint8_t data[] = {
        // GDL90 Flag
        0x7e,
        // GDL90Message ID
        0x0b,
        // GDL90OwnshipGeometricAltitude
        0xff, 0xff, 0xff, 0xff,
        // GDL90CRC
        0x01, 0x12,
        // GDL90 Flag
        0x7e
    };

    GDL90Message gdl90Message = {0};
    assert(GDL90Message_init(&gdl90Message, data, sizeof(data)) == GDL90ResultOK);

    GDL90OwnshipGeometricAltitude gdl90OwnshipGeometricAltitude = {0};
    assert(GDL90OwnshipGeometricAltitude_init(&gdl90OwnshipGeometricAltitude, &gdl90Message) == GDL90ResultOK);

    // Geo Altitude: 
    // -1,000 feet 0xFF38
    UpdateGDL90Bytes2(GDL90OwnshipGeometricAltitude, gdl90OwnshipGeometricAltitude, 1, 0xff, 0x38);
    assert(gdl90OwnshipGeometricAltitude.geoAltitude == -1000);
    // 0 feet 0x0000
    UpdateGDL90Bytes2(GDL90OwnshipGeometricAltitude, gdl90OwnshipGeometricAltitude, 1, 0x00, 0x00);
    assert(gdl90OwnshipGeometricAltitude.geoAltitude == 0);
    // +1000 feet 0x00C8
    UpdateGDL90Bytes2(GDL90OwnshipGeometricAltitude, gdl90OwnshipGeometricAltitude, 1, 0x00, 0xc8);
    assert(gdl90OwnshipGeometricAltitude.geoAltitude == 1000);

    // Vertical Metrics:

    // Value 0x7FFF is reserved to indicate that VFOM is not available.
    UpdateGDL90Bytes2(GDL90OwnshipGeometricAltitude, gdl90OwnshipGeometricAltitude, 3, 0x7f, 0xff);
    assert(gdl90OwnshipGeometricAltitude.hasValidVFOM == 0);

    // Examples of Vertical Metrics values: 

    // Vertical Warning and VFOM not available: 0xFFFF
    UpdateGDL90Bytes2(GDL90OwnshipGeometricAltitude, gdl90OwnshipGeometricAltitude, 3, 0xff, 0xff);
    assert(gdl90OwnshipGeometricAltitude.verticalWarning == 1);
    assert(gdl90OwnshipGeometricAltitude.hasValidVFOM == 0);
    // No Vertical Warning, VFOM = 40,000 meters 0x7FFE (max value represantable is 32,766)
    UpdateGDL90Bytes2(GDL90OwnshipGeometricAltitude, gdl90OwnshipGeometricAltitude, 3, 0x7f, 0xfe);
    assert(gdl90OwnshipGeometricAltitude.verticalWarning == 0);
    assert(gdl90OwnshipGeometricAltitude.verticalFigureOfMerit == 32766);
    // No Vertical Warning, VFOM = 10 meters 0x000A
    UpdateGDL90Bytes2(GDL90OwnshipGeometricAltitude, gdl90OwnshipGeometricAltitude, 3, 0x00, 0x0a);
    assert(gdl90OwnshipGeometricAltitude.verticalWarning == 0);
    assert(gdl90OwnshipGeometricAltitude.verticalFigureOfMerit == 10);
    // Vertical Warning, VFOM = 50 meters 0x8032
    UpdateGDL90Bytes2(GDL90OwnshipGeometricAltitude, gdl90OwnshipGeometricAltitude, 3, 0x80, 0x32);
    assert(gdl90OwnshipGeometricAltitude.verticalWarning == 1);
    assert(gdl90OwnshipGeometricAltitude.verticalFigureOfMerit == 50);
}

static void testGDL90TrafficReport(void)
{
    // 3.5.2. Traffic Report Example 
    
    uint8_t data[] = {
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
    };

    GDL90Message gdl90Message = {0};
    assert(GDL90Message_init(&gdl90Message, data, sizeof(data)) == GDL90ResultOK);

    GDL90TrafficReport gdl90TrafficReport = {0};
    assert(GDL90TrafficReport_init(&gdl90TrafficReport, &gdl90Message) == GDL90ResultOK);
    
    // No Traffic Alert
    assert(gdl90TrafficReport.alertStatus == GDL90TrafficReportAlertStatusTypeNoAlert);
    // ICAO ADS-B Address (octal): 526425118
    assert(gdl90TrafficReport.participantAddress == 052642511);
    // // Latitude: 44.90708 (North)
    assert((int32_t)gdl90TrafficReport.latitude == 44);
    // // Longitude: -122.99488 (West)
    assert((int32_t)gdl90TrafficReport.longitude == -122);
    // // Altitude: 5,000 feet (pressure altitude)
    assert(gdl90TrafficReport.altitude == 5000);
    // // Airborne with True Track
    assert(gdl90TrafficReport.airGroundState == 1);
    assert(gdl90TrafficReport.trackHeadingType == GDL90TrafficReportTrackHeadingTypeTrueTrackAngle);
    // // HPL = 20 meters, HFOM = 25 meters (NIC = 10, NACp = 9)
    assert(gdl90TrafficReport.navigationIntegrityCategory == GDL90TrafficReportNICTypeHPL_LT25M_VPL_LT_37p5M);
    // // Horizontal velocity: 123 knots at 45 degrees (True Track)
    assert(gdl90TrafficReport.horizontalVelocity == 123);
    assert((int32_t)gdl90TrafficReport.trackHeading == 45);
    // // Vertical velocity: 64 FPM climb
    assert(gdl90TrafficReport.verticalVelocity == 64);
    // // Emergency/Priority Code: none
    assert(gdl90TrafficReport.emergencyPriorityCode == GDL90TrafficReportEmergencyPriorityCodeTypeNoEmergency);
    // // Emitter Category: Light
    assert(gdl90TrafficReport.emitterCategory == GDL90TrafficReportEmitterCategoryLightICAO);
    // // Tail Number: N825V
    assert(strncmp(gdl90TrafficReport.callsign, "N825V", 8) == 0);

    // 3.5.1.7 HORIZONTAL VELOCITY

    // 0xFFE = 4094
    UpdateGDL90Bytes2(GDL90TrafficReport, gdl90TrafficReport, 14, 0xff, 0xe0);
    assert(gdl90TrafficReport.horizontalVelocity == 4094);

    // 3.5.1.8 VERTICAL VELOCITY

    // 0 = 0 FPM
    UpdateGDL90Bytes2(GDL90TrafficReport, gdl90TrafficReport, 15, 0x00, 0x00);
    assert(gdl90TrafficReport.verticalVelocity == 0);
    // 0x001 = 64 FPM climb
    UpdateGDL90Bytes2(GDL90TrafficReport, gdl90TrafficReport, 15, 0x00, 0x01);
    assert(gdl90TrafficReport.verticalVelocity == 64);
    // 0xFFF = 64 FPM descend
    UpdateGDL90Bytes2(GDL90TrafficReport, gdl90TrafficReport, 15, 0x0f, 0xff);
    assert(gdl90TrafficReport.verticalVelocity == -64);
    // 0x1FD = 32,576 FPM climb
    UpdateGDL90Bytes2(GDL90TrafficReport, gdl90TrafficReport, 15, 0x01, 0xfd);
    assert(gdl90TrafficReport.verticalVelocity == 32576);
    // 0x1FE = > 32,576 climb
    UpdateGDL90Bytes2(GDL90TrafficReport, gdl90TrafficReport, 15, 0x01, 0xfe);
    assert(gdl90TrafficReport.verticalVelocity == 32640);
    // 0xE03 = 32,576 FPM descend
    UpdateGDL90Bytes2(GDL90TrafficReport, gdl90TrafficReport, 15, 0x0e, 0x03);
    assert(gdl90TrafficReport.verticalVelocity == -32576);
    // 0xE02 = > 32.576 FPM descend
    UpdateGDL90Bytes2(GDL90TrafficReport, gdl90TrafficReport, 15, 0x0e, 0x02);
    assert(gdl90TrafficReport.verticalVelocity == -32640);
    // 0x800 = no vertical rate available
    UpdateGDL90Bytes2(GDL90TrafficReport, gdl90TrafficReport, 15, 0x08, 0x00);
    assert(gdl90TrafficReport.hasValidVerticalVelocity == 0);

    // 3.5.1.4 ALTITUDE

    // -1,000 feet 0x000
    UpdateGDL90Bytes2(GDL90TrafficReport, gdl90TrafficReport, 11, 0x00, 0x00);
    assert(gdl90TrafficReport.altitude == -1000);
    // 0 feet 0x028
    UpdateGDL90Bytes2(GDL90TrafficReport, gdl90TrafficReport, 11, 0x02, 0x80);
    assert(gdl90TrafficReport.altitude == 0);
    // +1000 feet 0x050
    UpdateGDL90Bytes2(GDL90TrafficReport, gdl90TrafficReport, 11, 0x05, 0x00);
    assert(gdl90TrafficReport.altitude == 1000);
    // +101,350 feet 0xFFE
    UpdateGDL90Bytes2(GDL90TrafficReport, gdl90TrafficReport, 11, 0xff, 0xe0);
    assert(gdl90TrafficReport.altitude == 101350);
}

static void testGDL90BasicReport(void)
{

}

static void testGDL90LongReport(void)
{

}

int main(int argc, char *argv[])
{
    if (argc == 1)
    {
        return EXIT_FAILURE;
    }

    switch (atoi(argv[1]))
    {
        case GDL90MessageType_Heartbeat:
            /** 3.1 Heartbeat (Out) */
            testGDL90Heartbeat();
            break;
        case GDL90MessageType_Initialization:
            /** 3.2 Initialization (In) */
            testGDL90Initialization();
            break;
        case GDL90MessageType_UplinkData:
            /** 3.3 Uplink Data (Out) */
            testGDL90UplinkData();
            break;
        case GDL90MessageType_HeightAboveTerrain:
            /** 3.7 Height Above Terrain (In) */
            testGDL90HeightAboveTerrain();
            break;
        case GDL90MessageType_OwnshipGeometricAltitude:
            /** 3.8 Ownship Geometric Altitude (Out) */
            testGDL90OwnshipGeometricAltitude();
            break;
        case GDL90MessageType_OwnshipReport:
        case GDL90MessageType_TrafficReport:
            /** 3.4 Ownship Report (Out) + 3.5 Traffic Report (Out) */
            testGDL90TrafficReport();
            break;
        case GDL90MessageType_BasicReport:
            testGDL90BasicReport();
            /** 3.6 Basic Report */
            break;
        case GDL90MessageType_LongReport:
            /** 3.6 Long Report */
            testGDL90LongReport();
            break;
        default:
            return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
