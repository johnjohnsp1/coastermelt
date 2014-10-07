/*
 * Interface to the undocumented commands for the Mediatek MT1939 Blu-Ray SoC.
 *
 * Copyright (c) 2014 Micah Elizabeth Scott
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#pragma once
#include "hexdump.h"
#include "tinyscsi.h"


class MT1939 {
public:
    static bool open(TinySCSI &scsi);

    struct FirmwareVersionInfo {
        uint8_t unknown[8];
    };

    struct ExtendedInquiryData {
        uint8_t unknown[0x60];
    };

    struct DeviceInfo {
        ExtendedInquiryData inquiry;
        FirmwareVersionInfo firmware;

        void print();
    };

    struct FirmwareImage {
        uint8_t bytes[0x200000];

        bool open(const char *filename);
        void print();
        void erase();
    };

    enum FirmwareWriteState {
        kFirmwareBegin = 0x00,
        kFirmwareContinue = 0xFF,
        kFirmwareComplete = 0x02
    };

    // Low-level commands
    static bool readFirmwareVersionInfo(TinySCSI &scsi, FirmwareVersionInfo* data);
    static bool extendedInquiry(TinySCSI &scsi, ExtendedInquiryData* data);
    static bool writeFirmware(TinySCSI &scsi, FirmwareWriteState state, uint8_t* data, unsigned dataLen);

    // Higher level operations
    static bool deviceInfo(TinySCSI &scsi, DeviceInfo* data);
    static bool writeFirmware(TinySCSI &scsi, FirmwareImage* data);
    static bool reset(TinySCSI &scsi);
};


/************************************************************************************/


inline bool MT1939::open(TinySCSI &scsi)
{
    return scsi.open(0x0e8d, 0x1956);
}

inline bool MT1939::readFirmwareVersionInfo(TinySCSI &scsi, FirmwareVersionInfo* data)
{
    uint8_t cdb[] = {0xff, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xaa};
    return scsi.in(cdb, sizeof cdb, (uint8_t*)data, sizeof *data);
}

inline bool MT1939::extendedInquiry(TinySCSI &scsi, ExtendedInquiryData* data)
{
    uint8_t cdb[] = {0x12, 0x00, 0x00, 0x00, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    return scsi.in(cdb, sizeof cdb, (uint8_t*)data, sizeof *data);
}

inline bool MT1939::deviceInfo(TinySCSI &scsi, DeviceInfo* data)
{
    return extendedInquiry(scsi, &data->inquiry) && readFirmwareVersionInfo(scsi, &data->firmware);
}

inline bool MT1939::writeFirmware(TinySCSI &scsi, FirmwareWriteState state, uint8_t* data, unsigned dataLen)
{
    uint8_t cdb[] = {0xff, 0x00, 0x01, 0x00, 0x00, state, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    return scsi.out(cdb, sizeof cdb, data, dataLen);
}

inline bool MT1939::writeFirmware(TinySCSI &scsi, FirmwareImage* data)
{
    fprintf(stderr, "[MT1939] Beginning firmware install\n");

    // First block of 0xF800 bytes
    if (!writeFirmware(scsi, kFirmwareBegin, &data->bytes[0], 0xF800)) {
        return false;
    }

    // 32 more continuation blocks. Most of these will be fast, but halfway through
    // there will be a 10 second pause as the device commits the first megabyte of data.

    for (unsigned block = 1; block <= 32; block++) {
        fprintf(stderr, "[MT1939] Writing block %d of %d\n", block, 32);
        if (!writeFirmware(scsi, kFirmwareContinue, &data->bytes[0xF800 * block], 0xF800)) {
            return false;
        }
    }

    // Final block of 2048 bytes. This will also cause a 10 second pause, and the device
    // will subsequently reboot into the new firmware. When the firmware is writing, the drive's
    // activity LED will blink with a particular cadence.

    fprintf(stderr, "[MT1939] Finishing firmware install\n");
    writeFirmware(scsi, kFirmwareComplete, &data->bytes[0xF800 * 33], 2048);

    return reset(scsi);
}

inline bool MT1939::reset(TinySCSI &scsi)
{
    fprintf(stderr, "[MT1939] USB reset and re-enumerate\n");
    scsi.reEnumerate();

    for (unsigned attempt = 0; attempt < 100; attempt++) {

        sleep(1);

        if (open(scsi)) {
            fprintf(stderr, "[MT1939] Device is back\n");

            DeviceInfo info;
            if (!deviceInfo(scsi, &info)) {
                fprintf(stderr, "[MT1939] Device info fail after reset\n");
                return false;
            }

            // Device is alive. Yay.
            info.print();
            return true;
        }

        fprintf(stderr, "[MT1939] trying again...\n");
    }

    fprintf(stderr, "[MT1939] Couldn't reopen after USB reset :(\n");
    return false;
}

inline bool MT1939::FirmwareImage::open(const char *filename)
{
    // Read a firmware image from disk, and verify it a little bit

    FILE *f = fopen(filename, "rb");
    if (!f) {
        fprintf(stderr, "Can't open firmware file: %s\n", filename);
        return false;
    }

    if (1 != fread(bytes, sizeof bytes, 1, f) ||
        0 != fseek(f, 0, SEEK_END) ||
        sizeof bytes != ftell(f)) {
        fprintf(stderr, "Firmware image must be exactly %d (0x%x) bytes\n",
            (int)sizeof bytes, (int)sizeof bytes);
        fclose(f);
        return false;
    }

    fclose(f);
    return true;
}

inline void MT1939::FirmwareImage::erase()
{
    memset(&bytes[0], 0xFF, sizeof bytes);
}

inline void MT1939::FirmwareImage::print()
{
    fprintf(stderr, "New firmware image infonub:\n");
    hexdump(&bytes[0x1fffd0], 48);
}

inline void MT1939::DeviceInfo::print()
{
    fprintf(stderr, "Inquiry:\n");
    hexdump(inquiry.unknown, sizeof inquiry.unknown);

    fprintf(stderr, "Firmware version:\n");
    hexdump(firmware.unknown, sizeof firmware.unknown);
}
