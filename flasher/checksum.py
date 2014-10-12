#!/usr/bin/env python

import struct, sys, binascii
from Crypto.Cipher import AES



def byteswap(b):
    w = len(b)/4
    return struct.pack('>%dI' % w, *struct.unpack('<%dI' % w, b))

def cbchash(msg, key):
    iv = chr(0)*16
    cipher = AES.new(key, AES.MODE_CBC, iv)
    msg = cipher.encrypt(msg)
    return ((' %08x')*4) % struct.unpack('<IIII', msg[-16:])

def cbchashswap(msg, key):
    return '\n'.join([
        cbchash(msg, key),
        cbchash(byteswap(msg), key),
        cbchash(msg, byteswap(key)), 
        cbchash(byteswap(msg), byteswap(key)),
    ])



class Firmware:
    def __init__(self, filename=None):
        if filename:
            self.open(filename)

    def open(self, filename):
        self.data = open(filename, 'rb').read()

    def peek(self, fmt, addr):
        return struct.unpack(fmt, self.data[addr:addr + struct.calcsize(fmt)])

    def checksum_stored(self):
        return self.peek('>H', 0x1ffffe)[0]

    def checksum_calculate(self):
        s = 0
        for i in range(0x10000, 0x1e2000):
            s += ord(self.data[i])
        return s & 0xffff

    def sigtable_entry(self, index):
        return self.peek('<IIIIIII', 0x10400 + 0x1c * index)

    def info(self):
        print '*** 16-bit checksum at 1ffffe'
        print 'Stored = %04x' % self.checksum_stored()
        print 'Calculated = %04x' % self.checksum_calculate()
        print

        print '*** Key at 10ff0'
        print '  %08x %08x %08x %08x' % self.peek('<IIII', 0x10ff0);
        print

        print '*** Signature table at 10400'
        for i in range(16):
            flag, mem_begin, mem_end, sig0, sig1, sig2, sig3 = self.sigtable_entry(i)
            if flag == 0xFFFFFFFF:
                break
            print
            print '  %08x:%08x  signature = %08x %08x %08x %08x' % (mem_begin, mem_end, sig0, sig1, sig2, sig3)

            segment = self.data[mem_begin:mem_end+1]
            print '                     length = %.f kB' % (len(segment) / 1024.0)
            print

            for key in [
                '9b684323e9d561b824a9224b42a09065',
                '00000000000000000000000000000000',
                'd7a3e98e5a3a2573357d324b0ccb1f53',
                '72ddcbacf6d5e164d765c8e5193284ab',
                '86a87e6c15d8881d2396ff005ffc8d29',
            ]:
                print cbchashswap(segment, binascii.a2b_hex(key))
        print


if __name__ == '__main__':
    if len(sys.argv) == 2:
        Firmware(sys.argv[1]).info()
    else:
        sys.stderr.write("usage: %s <firmware.bin>\n" % sys.argv[0])
