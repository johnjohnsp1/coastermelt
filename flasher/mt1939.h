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

    enum FirmwareWriteState {
        kFirmwareBegin = 0x00,
        kFirmwareContinue = 0xFF,
        kFirmwareComplete = 0x02
    };

    static bool readFirmwareVersionInfo(TinySCSI &scsi, FirmwareVersionInfo* data);
    static bool extendedInquiry(TinySCSI &scsi, ExtendedInquiryData* data);
    static bool deviceInfo(TinySCSI &scsi, DeviceInfo* data);

    static bool writeFirmware(TinySCSI &scsi, FirmwareWriteState state, uint8_t* data, unsigned dataLen);
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

inline void MT1939::DeviceInfo::print()
{
    fprintf(stderr, "Inquiry:\n");
    hexdump(inquiry.unknown, sizeof inquiry.unknown);

    fprintf(stderr, "Firmware version:\n");
    hexdump(firmware.unknown, sizeof firmware.unknown);
}
