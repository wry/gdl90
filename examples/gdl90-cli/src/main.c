//
//  main.cpp
//  gdl90-cli
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

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define MAX_PACKET_SIZE 1024

static char textbuf[1024] = {0};

uint16_t getPacketFromHexStr(char *buf, uint16_t buflen, uint8_t packet[MAX_PACKET_SIZE]);

static void handleGDL90Message(GDL90Message *gdl90Message, void *message)
{
    switch (gdl90Message->id)
    {
        case GDL90MessageType_Heartbeat:
            printf("%s", GDL90Heartbeat_toString((GDL90Heartbeat*)message, textbuf, sizeof(textbuf)));
            break;
        case GDL90MessageType_Initialization:
            printf("%s", GDL90Initialization_toString((GDL90Initialization*)message, textbuf, sizeof(textbuf)));
            break;
        case GDL90MessageType_UplinkData:
            printf("%s", GDL90UplinkData_toString((GDL90UplinkData*)message, textbuf, sizeof(textbuf)));
            break;
        case GDL90MessageType_HeightAboveTerrain:
            printf("%s", GDL90HeightAboveTerrain_toString((GDL90HeightAboveTerrain*)message, textbuf, sizeof(textbuf)));
            break;
        case GDL90MessageType_OwnshipGeometricAltitude:
            printf("%s", GDL90OwnshipGeometricAltitude_toString((GDL90OwnshipGeometricAltitude*)message, textbuf, sizeof(textbuf)));
            break;
        case GDL90MessageType_OwnshipReport:
        case GDL90MessageType_TrafficReport:
            printf("%s", GDL90TrafficReport_toString((GDL90TrafficReport*)message, textbuf, sizeof(textbuf)));
            break;
        case GDL90MessageType_BasicReport:
            printf("%s", GDL90BasicReport_toString((GDL90BasicReport*)message, textbuf, sizeof(textbuf)));
            break;
        case GDL90MessageType_LongReport:
            printf("%s", GDL90LongReport_toString((GDL90LongReport*)message, textbuf, sizeof(textbuf)));
            break;

    }
}

static void handleGDL90Error(GDL90Message *gdl90Message, GDL90StreamProcessingError error)
{
    switch (error)
    {
        case GDL90StreamProcessingErrorCRCError:
            printf("CRC error processing message with id %d\n", gdl90Message->id);
            break;
        case GDL90StreamProcessingErrorInvalidMessage:
            printf("Invalid message with id %d\n", gdl90Message->id);
            break;
        case GDL90StreamProcessingErrorUnknownMessageType:
            printf("Unknown message id : %d\n", gdl90Message->id);
            break;
    }
}

uint16_t getPacketFromHexStr(char *buf, uint16_t buflen, uint8_t packet[MAX_PACKET_SIZE])
{
    uint16_t packetLength = 0;
    for (uint16_t i=0; i < buflen; i++)
    {
        uint8_t c = (uint8_t)buf[i+0];

        if (c >= '0' && c <= '9')
        {
            c -= '0';
        }
        else if (c >= 'a' && c <= 'f')
        {
            c -= 'a'-0xa;
        }
        else
        {
            continue;
        }

        if (i % 2 == 0)
        {
            packet[packetLength] = (uint8_t)(c << 4);
        }
        else
        {
            packet[packetLength++] |= c;
        }

        if (packetLength == MAX_PACKET_SIZE)
        {
            break;
        }
    }
    return packetLength;
}

int main(int argc, char *argv[])
{
    GDL90StreamConfig gdl90StreamConfig = {0};
    GDL90Stream gdl90Stream = {0};

    uint16_t packetLength = 0;
    uint8_t packet[MAX_PACKET_SIZE] = {0};

    GDL90StreamConfig_init(&gdl90StreamConfig, handleGDL90Message, handleGDL90Error);
    GDL90Stream_init(&gdl90Stream, &gdl90StreamConfig);

    if (argc > 1)
    {
        packetLength = getPacketFromHexStr(argv[1], (uint16_t)strlen(argv[1]), packet);
        if (packetLength && packet[0] == 0x7e && packet[packetLength-1] == 0x7e)
        {
            GDL90Stream_process(&gdl90Stream, packet, packetLength);
        }
    }
#ifndef _WIN32
    else
    {
        char* buf = NULL;
        size_t buflen = 0;
        ssize_t nread = 0;
        while ((nread = getline(&buf, (size_t*)&buflen, stdin)) > 3)
        {
            packetLength = getPacketFromHexStr(buf, (uint16_t)nread, packet);
            if (packetLength && packet[0] == 0x7e && packet[packetLength-1] == 0x7e)
            {
                GDL90Stream_process(&gdl90Stream, packet, packetLength);
            }
        }
        free(buf);
        buf = NULL;
    }
#endif

    return EXIT_SUCCESS;
}
