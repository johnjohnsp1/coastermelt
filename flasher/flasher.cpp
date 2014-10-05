/*
 * Firmware flashing tool for Blu-Ray drives using the Mediatek MT1939 chip.
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

#include "tinyscsi.h"

int main(int argc, char** argv)
{
    TinySCSI scsi;

    if (!scsi.open(0x0e8d, 0x1956)) {
        return 1;
    }

    // Read firmware version info

    uint8_t cdbReadVersion[] = {0xff, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xaa};
    uint8_t version[8];

    if (!scsi.in(cdbReadVersion, sizeof cdbReadVersion, version, sizeof version)) {
        return 1;
    }

    printf("Version info:");
    for (unsigned i = 0; i < sizeof version; i++) {
        printf(" %02x", version[i]);
    }
    printf("\n");

    return 0;
}
