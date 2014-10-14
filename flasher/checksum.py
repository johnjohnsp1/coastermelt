#!/usr/bin/env python
#
# Read or "fix" signature and checksum in MT1939 firmware image.
# - Micah Elizabeth Scott 2014. This file is released into the public domain.
#

import struct, sys, os, random


class Firmware:
    def __init__(self, filename=None):
        if filename:
            self.open(filename)

    def open(self, filename):
        self.data = open(filename, 'rb').read()
        if len(self.data) != 0x200000:
            raise ValueError("Firmware image needs to be exactly 2 MB in size");

    def save(self, filename):
        tempname = filename + '_%04d' % random.randint(0, 9999)
        f = open(tempname, 'wb')
        f.write(self.data)
        f.close()
        os.rename(tempname, filename)

    def peek(self, fmt, addr):
        return struct.unpack(fmt, self.data[addr:addr + struct.calcsize(fmt)])

    def poke(self, fmt, addr, *arg):
        s = struct.pack(fmt, *arg)
        self.data = self.data[:addr] + s + self.data[addr+len(s):]

    def checksum_stored(self):
        return self.peek('>H', 0x1ffffe)[0]

    def checksum_calculate(self):
        return 0xffff & sum(bytearray(self.data[0x10000:0x1e2000]))
    
    def checksum_set(self, s):
        self.poke('>H', 0x1ffffe, s)

    def sigtable_entry(self, index):
        return self.peek('<IIIIIII', 0x10400 + 0x1c * index)

    def sigtable_clear(self):
        self.poke('<I', 0x10400, 0xffffffff)

    def fix(self):
        self.sigtable_clear()
        self.checksum_set(self.checksum_calculate())

    def info(self):
        print '- Key at 10ff0'
        print (' ' + 4*' %08x') % self.peek('<IIII', 0x10ff0);

        print '- Signature table at 10400'
        for i in range(16):
            flag, mem_begin, mem_end, sig0, sig1, sig2, sig3 = self.sigtable_entry(i)
            if flag == 0xFFFFFFFF:
                if i == 0:
                    print '  (empty)'
                break
            print '  %08x:%08x  %08x-%08x-%08x-%08x  (%.1f kiB)' % (
                mem_begin, mem_end, sig0, sig1, sig2, sig3, (mem_end - mem_begin + 1) / 1024.0)

        print '- 16-bit checksum at 1ffffe'
        print '  stored = %04x' % self.checksum_stored()
        print '  actual = %04x' % self.checksum_calculate()


if __name__ == '__main__':
    if len(sys.argv) == 3 and sys.argv[1] == '--fix':
        f = Firmware(sys.argv[2])
        f.info()
        f.fix()
        f.save(sys.argv[2])
        print (
            '\n'
            '--- Fixed it ---\n'
            '\n'
            'WARNING: This tool intentionally bypasses the integrity checks on the\n'
            '         supplied firmware image. It may be easy to create an image that\n'
            '         "bricks" your drive, making it impossible to install another image.\n'
        )
        f.info()

    elif len(sys.argv) == 2:
        Firmware(sys.argv[1]).info()

    else:
        print 'usage: %s [--fix] <firmware.bin>' % sys.argv[0]
