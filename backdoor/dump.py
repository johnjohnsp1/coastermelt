#!/usr/bin/env python
import remote, sys, struct, time

#
# Dumper utility using the peek command. Super slow!
#

def block_read(d, address, size):
    parts = []
    size &= ~3
    i = 0
    timestamp = time.time()

    while i < size:
        parts.append(d.peek(address + i))
        i += 4

        now = time.time()
        if now > timestamp + 0.2:
            print "%d / %d bytes read" % (i, size)
            timestamp = now

    return struct.pack('<%dI' % len(parts), *parts)

# Based on https://gist.github.com/sbz/1080258
def hexdump(src, length=16, address=0):
    FILTER = ''.join([(len(repr(chr(x))) == 3) and chr(x) or '.' for x in range(256)])
    lines = []
    for c in xrange(0, len(src), length):
        chars = src[c:c+length]
        hex = ' '.join(["%02x" % ord(x) for x in chars])
        printable = ''.join(["%s" % ((ord(x) <= 127 and FILTER[ord(x)]) or '.') for x in chars])
        lines.append("%08x  %-*s  %s\n" % (address + c, length*3, hex, printable))
    return ''.join(lines)

def command(d, address, size, log_file = 'result.log'):
    data = block_read(d, address, size)
    if log_file:
        open(log_file, 'wb').write(data)
    sys.stdout.write(hexdump(data, 16, address))


if __name__ == "__main__":
    if len(sys.argv) != 3:
        print "usage: %s address size" % sys.argv[0]
        sys.exit(1)
    command(remote.Device(),
        int(sys.argv[1].replace('_',''), 16), 
        int(sys.argv[2].replace('_',''), 16))
