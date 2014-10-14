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

#include <algorithm>
#include "mt1939.h"

int main(int argc, char** argv)
{
    TinySCSI scsi;
    MT1939::DeviceInfo info; 
    MT1939::FirmwareImage fw;
    MT1939::BackdoorSignature bdsig;

    if (!MT1939::open(scsi)) {
        return 1;
    }

    if (!MT1939::deviceInfo(scsi, &info)) {
        return 1;
    }

    // Various usage formats...

    if (argc == 1) {

        info.print();

        if (MT1939::backdoorSignature(scsi, &bdsig)) {
            bdsig.print();
        }

    } else if (argc == 2 && !strcmp("--erase", argv[1])) {

        // It's important that "--erase" skip the backdoor info, in case the
        // backdoor patch is crashing.

        info.print();
        fw.erase();
        fw.print();

        if (!MT1939::writeFirmware(scsi, &fw)) {
            return 1;
        }

    } else if (argc >= 3 && !strcmp("--scsi", argv[1])) {

        uint8_t cdb[12];
        const char *dumpfile = "result.log";
        static uint8_t data[1024*1024*128];
        unsigned len = std::min<unsigned>(strtol(argv[2], 0, 16), sizeof data - 1);

        memset(cdb, 0, sizeof cdb);
        for (int i = 0; i < sizeof cdb; i++) {
            const char *arg = argv[3+i];
            if (!arg) {
                break;
            }
            cdb[i] = strtol(arg, 0, 16);
        }

        fprintf(stderr, "\nCDB:\n");
        hexdump(cdb, sizeof cdb);

        if (scsi.in(cdb, sizeof cdb, data, len) && len) {
            FILE *f = fopen(dumpfile, "wb");
            if (f && fwrite(data, len, 1, f) == 1) {
                fprintf(stderr, "Result, %d bytes (saved to %s)\n", len, dumpfile);
                fclose(f);
            } else {
                perror("Problem saving result to disk");
            }

            hexdump(data, len);
        }  

    } else if (argc == 2 && fw.open(argv[1])) {

        info.print();

        if (MT1939::backdoorSignature(scsi, &bdsig)) {
            bdsig.print();
        }

        fprintf(stderr, "Firmware image loaded from disk\n");
        fw.print();

        if (!MT1939::writeFirmware(scsi, &fw)) {
            return 1;
        }

        info.print();

        if (MT1939::backdoorSignature(scsi, &bdsig)) {
            bdsig.print();
        }

    } else {
        fprintf(stderr,
            "\n"
            "usage:\n"
            "    mtflash           Shows device version info, changes nothing\n"
            "    mtflash fw.bin    Program a 2MB raw firmware image file.\n"
            "                      The first 64 kiB is locked and can't be programmed,\n"
            "                      so these bytes in the image are ignored.\n"
            "    mtflash --erase   Send an image of all 0xFFs, erasing the unlocked\n"
            "                      portions of flash.\n"
            "    mtflash --scsi    Send a low level SCSI command.\n"
            "\n"
            "scsi examples:\n"
            "    mtflash --scsi 60 12 00 00 00 60                    Long inquiry command\n"
            "    mtflash --scsi 8 ff 00 ff                           Firmware version\n"
            "    mtflash --scsi 2 ff 00 05                           Read appselect bit0\n"
            "    mtflash --scsi 0 ff 00 04 00 00 01                  Set appselect bit0\n"
            "    mtflash --scsi 0 ff 00 04 00 00 00                  Clear appselect bit0\n"
            "    mtflash --scsi ffff 3c 6 0 0 0 0 0 ff ff            Read first 64k of DRAM\n"
            );
        return 1;
    }

    return 0;
}
