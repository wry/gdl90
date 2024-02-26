//
//  gdl90.h
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

// Based on the GDL-90 Data Interface Specification (560-1058-00 Rev A) at
// https://www.faa.gov/sites/faa.gov/files/air_traffic/technology/adsb/archival/GDL90_Public_ICD_RevA.PDF

#ifndef __gdl90__gdl90_h__
#define __gdl90__gdl90_h__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

typedef enum GDL90Result
{
    GDL90ResultOK,
    GDL90ResultFailure
} GDL90Result;

/** 3. Message Definitions */
typedef enum GDL90MessageType
{
    /** 3.1 Heartbeat (Out) */
    GDL90MessageType_Heartbeat = 0x00,
    /** 3.2 Initialization (In) */
    GDL90MessageType_Initialization = 0x02,
    /** 3.3 Uplink Data (Out) */
    GDL90MessageType_UplinkData = 0x07,
    /** 3.7 Height Above Terrain (In) */
    GDL90MessageType_HeightAboveTerrain = 0x09,
    /** 3.4 Ownship Report (Out) */
    GDL90MessageType_OwnshipReport = 0x0a, // 10
    /** 3.8 Ownship Geometric Altitude (Out) */
    GDL90MessageType_OwnshipGeometricAltitude = 0x0b, // 11
    /** 3.5 Traffic Report (Out) */
    GDL90MessageType_TrafficReport = 0x14, // 20
    /** 3.6 Basic Report */
    GDL90MessageType_BasicReport = 0x1e, // 30
    /** 3.6 Long Report */
    GDL90MessageType_LongReport = 0x1f // 31
} GDL90MessageType;

/** 2.2. MESSAGE STRUCTURE OVERVIEW */
typedef struct GDL90Message
{
    /** Message ID (GDL90MessageType) */
    uint8_t id;
    /** Buffer to fit largest message (UplinkData) escaped */
    uint8_t data[1<<9];
    /** Size of data in payload */
    uint16_t dataLength;
} GDL90Message;

/** Initialize GDL90Message with unescaped GDL90 data (0x?? ... 0x??) */
GDL90Result GDL90Message_init(GDL90Message *, const uint8_t *data, const uint16_t dataLength);

typedef enum GDL90HeartbeatStatusByte1Bit
{
    /** Bit 0: UAT Initialized */
    GDL90HeartbeatStatusByte1BitUATInitialized = 0,
    /** Bit 2: RATCS */
    GDL90HeartbeatStatusByte1BitRATCS = 1,
    /** Bit 3: GPS Batt Low */
    GDL90HeartbeatStatusByte1BitGSPBattLow = 3,
    /** Bit 4: Addr Type */
    GDL90HeartbeatStatusByte1BitAddrType = 4,
    /** Bit 5: IDENT */
    GDL90HeartbeatStatusByte1BitIDENT = 5,
    /** Bit 6: Maint Req'd */
    GDL90HeartbeatStatusByte1BitMaintReqd = 6,
    /** Bit 7: GPS Pos Valid */
    GDL90HeartbeatStatusByte1BitGSPPosValid = 7
} GDL90HeartbeatStatusByte1Bit;

typedef enum GDL90HeartbeatStatusByte2Bit
{
    /** Bit 0: UTC OK  */
    GDL90HeartbeatStatusByte2BitUTCOK = 0,
    /** Bit 5: CSA Not Available */
    GDL90HeartbeatStatusByte2BitCSANotAvailable = 5,
    /** Bit 6: CSA Requested */
    GDL90HeartbeatStatusByte2BitCSARequested = 6,
} GDL90HeartbeatStatusByte2Bit;

/** 3.1. HEARTBEAT MESSAGE */
typedef struct GDL90Heartbeat
{
    /** Message ID (GDL90MessageType) */
    uint8_t id;
    /** Status Byte 1 (1<<GDL90HeartbeatStatusByte1Bit) */
    uint8_t status1;
    /** Status Byte 2 (1<<GDL90HeartbeatStatusByte2Bit) */
    uint8_t status2;
    /** UAT Time Stamp */
    uint32_t timestamp;
    /** Number of Uplink messages received during previous second */
    uint8_t uplinkMessageCount;
    /** Number of Basic and Long messages received during previous second */
    uint16_t basicLongMessageCount;
} GDL90Heartbeat;

GDL90Result GDL90Heartbeat_init(GDL90Heartbeat *, GDL90Message *gdl90Message);
char* GDL90Heartbeat_toString(GDL90Heartbeat *, char *out, size_t len);

typedef enum GDL90InitializationConfiguration1Bit
{
    /** Bit 0: 1 = CDTI capability is operating */
    GDL90InitializationConfiguration1BitCDTIOK = 0,
    /** Bit 1: 1 = Suppress GDL 90 audio output */
    GDL90InitializationConfiguration1BitAudioInhibit = 1,
    /** Bit 6: 1 = Initiate audio test */
    GDL90InitializationConfiguration1BitAudioTest = 6
} GDL90InitializationConfiguration1Bit;

typedef enum GDL90InitializationConfiguration2Bit
{
    /** Bit 0: 1 = Disable CSA traffic alerting */
    GDL90InitializationConfiguration2BitCSADisable = 0,
    /** Bit 1: 1 = Disable GDL 90 audible traffic alerts  */
    GDL90InitializationConfiguration2BitCSAAudioDisable = 1
} GDL90InitializationConfiguration2Bit;

/** 3.2. INITIALIZATION MESSAGE */
typedef struct GDL90Initialization
{
    /** Message ID (GDL90MessageType) */
    uint8_t id;
    /** Configuration Byte 1 (1<<GDL90InitializationConfiguration1Bit) */
    uint8_t configuration1;
    /** Configuration Byte 2 (1<<GDL90InitializationConfiguration2Bit) */
    uint8_t configuration2;
} GDL90Initialization;

GDL90Result GDL90Initialization_init(GDL90Initialization *, GDL90Message *gdl90Message);
uint8_t* GDL90Initialization_toBytes(GDL90Initialization *, uint8_t out[3]);
char* GDL90Initialization_toString(GDL90Initialization *, char *out, size_t len);

/** 3.3. UPLINK DATA MESSAGE */
typedef struct GDL90UplinkData
{
    /** Message ID (GDL90MessageType) */
    uint8_t id;
    /** Time of reception, ns (0xffffff if invalid) */
    uint32_t timeOfReception;
    /** Contents of Uplink message (see RTCA/DO-282) */
    uint8_t payload[432];

    /** 0 if 0xffffff */
    uint8_t hasValidTor;
} GDL90UplinkData;

GDL90Result GDL90UplinkData_init(GDL90UplinkData *, GDL90Message *gdl90Message);
char* GDL90UplinkData_toString(GDL90UplinkData *, char *out, size_t len);

/** 3.7. HEIGHT ABOVE TERRAIN */
typedef struct GDL90HeightAboveTerrain
{
    /** Message ID (GDL90MessageType) */
    uint8_t id;
    /** Height Above Terrain (0x8000/-32768 if invalid) */
    int16_t heightAboveTerrain;
} GDL90HeightAboveTerrain;

GDL90Result GDL90HeightAboveTerrain_init(GDL90HeightAboveTerrain *, GDL90Message *gdl90Message);
uint8_t* GDL90HeightAboveTerrain_toBytes(GDL90HeightAboveTerrain *, uint8_t out[3]);
char* GDL90HeightAboveTerrain_toString(GDL90HeightAboveTerrain *, char *out, size_t len);

/** 3.8. OWNSHIP GEOMETRIC ALTITUDE MESSAGE */
typedef struct GDL90OwnshipGeometricAltitude
{
    /** Message ID (GDL90MessageType) */
    uint8_t id;
    /** Vertical Warning Indicator */
    uint8_t verticalWarning;
    /** Set if VFOM 0x7fff */
    uint8_t hasValidVFOM;
    /** Vertical Figure of Merit (m) */
    uint16_t verticalFigureOfMerit;
    /** Ownship Geo Altitude (ft) */
    int32_t geoAltitude;
} GDL90OwnshipGeometricAltitude;

GDL90Result GDL90OwnshipGeometricAltitude_init(GDL90OwnshipGeometricAltitude *, GDL90Message *gdl90Message);
char* GDL90OwnshipGeometricAltitude_toString(GDL90OwnshipGeometricAltitude *, char *out, size_t len);

/** 3.5.1.1 TRAFFIC ALERT STATUS */
typedef enum GDL90TrafficReportAlertStatusType
{
    /** s = 0 : No alert */
    GDL90TrafficReportAlertStatusTypeNoAlert,
    /** s = 1 : Traffic Alert */
    GDL90TrafficReportAlertStatusTypeTrafficAlert,
} GDL90TrafficReportAlertStatusType;

char* GDL90TrafficReportAlertStatusType_toString(GDL90TrafficReportAlertStatusType alertStatus);

/** 3.5.1.2 TARGET IDENTITY */
typedef enum GDL90TrafficReportAddressType
{
    /** t = 0 : ADS-B with ICAO address */
    GDL90TrafficReportAddressTypeADSBWithICAO,
    /** t = 1 : ADS-B with Self-assigned address */
    GDL90TrafficReportAddressTypeADSBSelfAssigned,
    /** t = 2 : TIS-B with ICAO address */
    GDL90TrafficReportAddressTypeTISBWithICAO,
    /** t = 3 : TIS-B with track file ID */
    GDL90TrafficReportAddressTypeTISBWithTrackFileID,
    /** t = 4 : Surface Vehicle */
    GDL90TrafficReportAddressTypeSurfaceVehicle,
    /** t = 5 : Ground Station Beacon */
    GDL90TrafficReportAddressTypeGroundStationBeacon,
} GDL90TrafficReportAddressType;

char* GDL90TrafficReportAddressType_toString(GDL90TrafficReportAddressType addressType);

/** 3.5.1.5 MISCELLANEOUS INDICATORS */
typedef enum GDL90TrafficReportTrackHeadingType
{
    /** "tt" not valid  */
    GDL90TrafficReportTrackHeadingTypeInvalid,
    /** "tt" = True Track Angle */
    GDL90TrafficReportTrackHeadingTypeTrueTrackAngle,
    /** "tt" = Heading (Magnetic) */
    GDL90TrafficReportTrackHeadingTypeHeadingMagnetic,
    /** "tt" = Heading (True) */
    GDL90TrafficReportTrackHeadingTypeHeadingTrue
} GDL90TrafficReportTrackHeadingType;

char* GDL90TrafficReportTrackHeadingType_toString(GDL90TrafficReportTrackHeadingType trackHeadingType);

/** 3.5.1.6 INTEGRITY (NIC) AND ACCURACY (NACP) */
typedef enum GDL90TrafficReportNICType
{
    /** Unknown */
    GDL90TrafficReportNICTypeUnknown,
    /** < 20.0 NM */
    GDL90TrafficReportNICTypeLT_20NM,
    /** < 8.0 NM */
    GDL90TrafficReportNICTypeLT_8NM,
    /** < 4.0 NM */
    GDL90TrafficReportNICTypeLT_4NM,
    /** < 2.0 NM */
    GDL90TrafficReportNICTypeLT_2NM,
    /** < 1.0 NM */
    GDL90TrafficReportNICTypeLT_1NM,
    /** < 0.6 NM */
    GDL90TrafficReportNICTypeLT_p6NM,
    /** < 0.2 NM */
    GDL90TrafficReportNICTypeLT_p2NM,
    /** < 0.1 NM */
    GDL90TrafficReportNICTypeLT_p1NM,
    /** HPL < 75 m and VPL < 112 m */
    GDL90TrafficReportNICTypeHPL_LT75M_VPL_LT_112M,
    /** HPL < 25 m and VPL < 37.5 m */
    GDL90TrafficReportNICTypeHPL_LT25M_VPL_LT_37p5M,
    /** HPL < 7.5 m and VPL < 11 m */
    GDL90TrafficReportNICTypeHPL_LT7p5M_VPL_LT_11M
} GDL90TrafficReportNICType;

char* GDL90TrafficReportNICType_toString(GDL90TrafficReportNICType nicType);

/** 3.5.1.6 INTEGRITY (NIC) AND ACCURACY (NACP) */
typedef enum GDL90TrafficReportNACPType
{
    /** Unknown */
    GDL90TrafficReportNACPTypeUnknown,
    /** < 10.0 NM */
    GDL90TrafficReportNACPTypeLT_10NM,
    /** < 4.0 NM */
    GDL90TrafficReportNACPTypeLT_4NM,
    /** < 2.0 NM */
    GDL90TrafficReportNACPTypeLT_2NM,
    /** < 1.0 NM */
    GDL90TrafficReportNACPTypeLT_1NM,
    /** < 0.5 NM */
    GDL90TrafficReportNACPTypeLT_0p5NM,
    /** < 0.3 NM */
    GDL90TrafficReportNACPTypeLT_0p3NM,
    /** < 0.1 NM */
    GDL90TrafficReportNACPTypeLT_0p1NM,
    /** < 0.05 NM */
    GDL90TrafficReportNACPTypeLT_0p05NM,
    /** HFOM < 30 m and VFOM < 45 m */
    GDL90TrafficReportNACPTypeHFOM_LT30M_VFOM_LT_45M,
    /** HFOM < 10 m and VFOM < 15 m */
    GDL90TrafficReportNACPTypeHFOM_LT10M_VFOM_LT_15M,
    /** HFOM < 3 m and VFOM < 4 m */
    GDL90TrafficReportNACPTypeHFOM_LT3M_VFOM_LT_4M
} GDL90TrafficReportNACPType;

char* GDL90TrafficReportNACPType_toString(GDL90TrafficReportNACPType nacpType);

/** 3.5.1.10 EMITTER CATEGORY */
typedef enum GDL90TrafficReportEmitterCategory
{
    /** No aircraft type information */
    GDL90TrafficReportEmitterCategoryNoAircraftTypeInformation,
    /** Light (ICAO) < 15 500 lbs */
    GDL90TrafficReportEmitterCategoryLightICAO,
    /** Small - 15 500 to 75 000 lbs */
    GDL90TrafficReportEmitterCategorySmall,
    /** Large - 75 000 to 300 000 lbs */
    GDL90TrafficReportEmitterCategoryLarge,
    /** High Vortex Large (e.g., aircraft such as B757) */
    GDL90TrafficReportEmitterCategoryHighVortexLarge,
    /** Heavy (ICAO) - > 300 000 lbs */
    GDL90TrafficReportEmitterCategoryHeavyICAO,
    /** Highly Maneuverable > 5G acceleration and high speed */
    GDL90TrafficReportEmitterCategoryHighlyManeuverable,
    /** Rotorcraft */
    GDL90TrafficReportEmitterCategoryRotorcraft,
    /** (Unassigned) */
    GDL90TrafficReportEmitterCategoryUnassigned8,
    /** Glider/sailplane */
    GDL90TrafficReportEmitterCategoryGliderSailPlane,
    /** Lighter than air */
    GDL90TrafficReportEmitterCategoryLighterThanAir,
    /** Parachutist/sky diver */
    GDL90TrafficReportEmitterCategoryParachutistSkyDiver,
    /** Ultra light/hang glider/paraglider */
    GDL90TrafficReportEmitterCategoryUltraLightHandGliderParaGlider,
    /** (Unassigned) */
    GDL90TrafficReportEmitterCategoryUnassigned13,
    /** Unmanned aerial vehicle */
    GDL90TrafficReportEmitterCategoryUnmannedAerialVehicle,
    /** Space/transatmospheric vehicle */
    GDL90TrafficReportEmitterCategorySpaceTransAtmosphericVehicle,
    /** (Unassigned) */
    GDL90TrafficReportEmitterCategoryUnassigned16,
    /** Surface vehicle — emergency vehicle */
    GDL90TrafficReportEmitterCategorySurfaceVehicleEmergencyVehicle,
    /** Surface vehicle — service vehicle */
    GDL90TrafficReportEmitterCategorySurfaceVehicleServiceVehicle,
    /** Point Obstacle (includes tethered balloons) */
    GDL90TrafficReportEmitterCategoryPointObstacle,
    /** Cluster Obstacle */
    GDL90TrafficReportEmitterCategoryClusterObstacle,
    /** Line Obstacle */
    GDL90TrafficReportEmitterCategoryLineObstacle
} GDL90TrafficReportEmitterCategory;

char* GDL90TrafficReportEmitterCategory_toString(GDL90TrafficReportEmitterCategory category);

/** 3.5.1.12 EMERGENCY/PRIORITY CODE */
typedef enum GDL90TrafficReportEmergencyPriorityCodeType
{
    /** p = 0 : no emergency */
    GDL90TrafficReportEmergencyPriorityCodeTypeNoEmergency,
    /** p = 1 : general emergency */
    GDL90TrafficReportEmergencyPriorityCodeTypeGeneralEmergency,
    /** p = 2 : medical emergency */
    GDL90TrafficReportEmergencyPriorityCodeTypeMedicalEmergency,
    /** p = 3 : minimum fuel */
    GDL90TrafficReportEmergencyPriorityCodeTypeMinimumFuel,
    /** p = 4 : no communication */
    GDL90TrafficReportEmergencyPriorityCodeTypeNoCommunication,
    /** p = 5 : unlawful interference */
    GDL90TrafficReportEmergencyPriorityCodeTypeUnlawfulInterference,
    /** p = 6 : downed aircraft  */
    GDL90TrafficReportEmergencyPriorityCodeTypeDownedAircraft
} GDL90TrafficReportEmergencyPriorityCodeType;

char* GDL90TrafficReportEmergencyPriorityCodeType_toString(GDL90TrafficReportEmergencyPriorityCodeType epcType);

/** 3.5.1. Traffic and Ownship Report Data Format */
typedef struct GDL90TrafficReport
{
    /** Message ID (GDL90MessageType) */
    uint8_t id;
    /** Traffic Alert Status (GDL90TrafficReportAlertStatusType) */
    uint8_t alertStatus; 
    /** Address Type (GDL90TrafficReportAddressType) */
    uint8_t  addressType;
    /** Participant Address */
    uint32_t participantAddress;
    /** Latitude (degrees) */
    double latitude;
    /** Longitude (degrees) */
    double longitude;
    /** Altitude (ft) */
    int32_t altitude;
    /** Misc indicators bits 0-1 */
    GDL90TrafficReportTrackHeadingType trackHeadingType;
    /** Misc indicators bit 2 (0 = Updated, 1 = Extrapolated) */
    uint8_t reportStatus;
    /** Misc indicators bit 3 (0 = On Ground, 1 = Airborne) */
    uint8_t airGroundState;
    /** Navigation Integrity Category (GDL90TrafficReportNICType) */
    uint8_t navigationIntegrityCategory;
    /** Navigation Accuracy Category for Position (GDL90TrafficReportNACPType) */
    uint8_t navigationAccuracyCategoryForPosition;
    /** Horizontal velocity (kt) */
    uint32_t horizontalVelocity;
    /** Vertical Velocity (ft/min) */
    int32_t verticalVelocity;
    /** Track/Heading (degrees) */
    double trackHeading;
    /** Emitter Category (GDL90TrafficReportEmitterCategory) */
    uint8_t emitterCategory;
    /** Call Sign (ascii) */
    char callsign[8];
    /** Emergency/Priority Code (GDL90TrafficReportEmergencyPriorityCodeType) */
    int8_t emergencyPriorityCode;
    /** Spare (reserved for future use) */
    int8_t spare;

    /** 0 if 0xfff */
    uint8_t hasValidAltitude;
    /** 0 if 0xfff */
    uint8_t hasValidHorizontalVelocity;
    /** 0 if 0x800 */
    uint8_t hasValidVerticalVelocity;
    /** 0f if lat == 0 && lon == 0 && nic == 0 */
    uint8_t hasValidPosition;
} GDL90TrafficReport;

GDL90Result GDL90TrafficReport_init(GDL90TrafficReport *, GDL90Message *gdl90Message);
char* GDL90TrafficReport_toString(GDL90TrafficReport *, char *out, size_t len);

/** 3.6. PASS-THROUGH REPORTS */
typedef struct GDL90BasicReport
{
    /** Message ID (GDL90MessageType) */
    uint8_t id;
    /** Time of reception, ns (0xffffff if invalid) (3.3.1. Time of Reception (TOR) ) */
    uint32_t timeOfReception;
    /** See RTCA/DO-282, Section 2.2 for format */
    uint8_t payload[18];

    /** 0 if 0xffffff */
    uint8_t hasValidTor;
} GDL90BasicReport;

GDL90Result GDL90BasicReport_init(GDL90BasicReport *, GDL90Message *gdl90Message);
char* GDL90BasicReport_toString(GDL90BasicReport *, char *out, size_t len);

typedef struct GDL90LongReport
{
    /** Message ID (GDL90MessageType) */
    uint8_t id;
    /** Time of reception, ns (0xffffff if invalid) (3.3.1. Time of Reception (TOR) ) */
    uint32_t timeOfReception;
    /** See RTCA/DO-282, Section 2.2 for format */
    uint8_t payload[34];

    /** 0 if 0xffffff */
    uint8_t hasValidTor;
} GDL90LongReport;

GDL90Result GDL90LongReport_init(GDL90LongReport *, GDL90Message *gdl90Message);
char* GDL90LongReport_toString(GDL90LongReport *, char *out, size_t len);

typedef enum GDL90CRCResult
{
    GDL90CRCResultOK,
    GDL90CRCResultInvalidInput,
    GDL90CRCResultMismatch
} GDL90CRCResult;

/** 2.2.3. FCS Calculation  */
typedef struct GDL90CRC
{
    /** GDL90 CRC-CCITT */
    uint16_t crc16Table[256];
} GDL90CRC;

GDL90Result GDL90CRC_init(GDL90CRC *);
GDL90Result GDL90CRC_crc(GDL90CRC *, uint16_t *outCrc, uint8_t *data, size_t len);
GDL90CRCResult GDL90CRC_isValid(GDL90CRC *, uint8_t *data, size_t len);

typedef enum GDL90StreamProcessingError
{
    GDL90StreamProcessingErrorCRCError,
    GDL90StreamProcessingErrorInvalidMessage,
    GDL90StreamProcessingErrorUnknownMessageType,
} GDL90StreamProcessingError;

/** Called for each detected, unescaped, CRC and size validated GDL90 msg */
typedef void (GDL90StreamMessageHandler)(GDL90Message *, void *);
/** Called for each error, with the contents of the unescaped GDL90 msg */
typedef void (GDL90StreamErrorHandler)(GDL90Message *, GDL90StreamProcessingError);

typedef struct GDL90StreamConfig
{
    GDL90StreamMessageHandler *messageHandler;
    GDL90StreamErrorHandler *errorHandler;
} GDL90StreamConfig;

GDL90Result GDL90StreamConfig_init(GDL90StreamConfig *, GDL90StreamMessageHandler *messageHandler, GDL90StreamErrorHandler *errorHandler);

typedef struct GDL90Stream
{
    GDL90StreamConfig config;
    GDL90CRC crc;
} GDL90Stream;

GDL90Result GDL90Stream_init(GDL90Stream *, GDL90StreamConfig *config);
GDL90Result GDL90Stream_process(GDL90Stream *, const uint8_t *data, const uint16_t dataLength);
GDL90Result GDL90Stream_handleUnescapedMessage(GDL90Stream *, GDL90Message* gdl90Message);

#ifdef __cplusplus
}
#endif

#endif /* defined(__gdl90__gdl90_h__) */
