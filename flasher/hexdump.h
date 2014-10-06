/*
 * Dumb C++ hex dump to stderr
 *
 * Micah Elizabeth Scott, 2014. This file is released into the public domain.
 */

#pragma once
#include <stdint.h>
#include <stdio.h>

inline void hexdump(const uint8_t *data, unsigned length,
    unsigned bytesPerLine = 16, const char* prefix = "", FILE* dest = 0)
{
    dest = dest ? dest : stderr;

    for (unsigned addr = 0; addr < length; addr += bytesPerLine) {

        // Address label
        fprintf(dest, "%s%8x:", prefix, addr);

        // Hex bytes
        for (unsigned i = 0; i < bytesPerLine; i++) {
            unsigned o = addr + i;

            if (o < length) {
                fprintf(dest, " %02x", data[o]);
            } else {
                fprintf(dest, "   ");
            }
        }

        // Spacer
        fprintf(dest, "  ");

        // ASCII bytes
        for (unsigned i = 0; i < bytesPerLine; i++) {
            unsigned o = addr + i;

            if (o < length) {
                uint8_t chr = data[o];
                if (chr < ' ' || chr > '~') {
                    chr = '.';
                }
                fprintf(dest, "%c", chr);
            } else {
                break;
            }
        }

        fprintf(dest, "\n");
    }
}
