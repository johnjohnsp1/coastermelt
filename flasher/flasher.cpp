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

#include "mt1939.h"

int main(int argc, char** argv)
{
    TinySCSI scsi;
    MT1939::DeviceInfo info; 
    MT1939::FirmwareImage fw;

    if (!MT1939::open(scsi)) {
        return 1;
    }

    if (!MT1939::deviceInfo(scsi, &info)) {
        return 1;
    }
    info.print();

    // Various usage formats...

    if (argc == 1) {
        return 0;

    } else if (argc == 2 && !strcmp("--erase", argv[1])) {
        fw.erase();

    } else if (argc == 2 && fw.open(argv[1])) {
        fprintf(stderr, "Firmware image loaded from disk\n");

    } else {
        fprintf(stderr,
            "\n"
            "usage:\n"
            "    mtflash          Shows device version info, changes nothing\n"
            "    mtflash fw.bin   Program a 2MB raw firmware image file.\n"
            "                     The first 64 kiB is locked and can't be programmed,\n"
            "                     so these bytes in the image are ignored.\n"
            "    mtflash --erase  Send an image of all 0xFFs, erasing the unlocked\n"
            "                     portions of flash.\n");
        return 1;
    }

    fw.print();

    unsigned delay = 5;
    fprintf(stderr, "--- WRITING in %d seconds ---\n", delay);
    sleep(delay);

    if (!MT1939::writeFirmware(scsi, &fw)) {
        return 1;
    }

    return 0;
}
