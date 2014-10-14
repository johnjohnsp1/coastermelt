#!/usr/bin/env python
from struct import pack, unpack
from binascii import a2b_hex, b2a_hex
import random, struct, remote, hilbert
d = remote.Device()

# Working our way up to "Hello World"!

# We can speak normal SCSI here too, since that seems useful.
print "Firmware: %r" % d.scsi_in(a2b_hex('120000006000000000000000'), 0x60)[8:48]

# Say hi to the backdoor patch
print "Backdoor: %r" % d.get_signature()

# Some free RAM! Test the peek/poke interface
pad = 0x1fffda0
d.poke(pad + 0x8, 0x1234)
d.poke(pad + 0xc, 0xf00f)
d.poke(pad + 0x10, 0xffffffff)
assert d.peek(pad + 0x8) == 0x1234
assert d.peek(pad + 0xc) == 0xf00f
assert d.peek(pad + 0x10) == 0xffffffff
d.poke_byte(pad + 0x10, 0xaa)
d.poke_byte(pad + 0x11, 0xbb)
d.poke_byte(pad + 0x12, 0xcc)
d.poke_byte(pad + 0x13, 0xdd)
assert d.peek(pad + 0x10) == 0xddccbbaa
d.poke_byte(pad + 0x12, d.peek_byte(pad + 0x12) + 1)
assert d.peek(pad + 0x10) == 0xddcdbbaa

# First step in testing BLX is to try a 'ping' against an ARM-mode "bx lr" gadget in firmware
assert d.blx(0x102b8, 0x55aabbcc)[0] == 0x55aabbcc
assert d.blx(0x102b8, 0x00112233)[0] == 0x00112233

# Now try the same test in Thumb mode
assert d.blx(0x145585, 0x55aa5283)[0] == 0x55aa5283
assert d.blx(0x145585, 0x00112233)[0] == 0x00112233

# Test the RAM block read. It seems to work with up to 0x1D words
L = 0x1d
pattern = [random.randint(0, 0xffffffff) for i in range(L)]
for i in range(L):
 	d.poke(pad + (i*4), pattern[i])
block = d.read_block(pad, L)
assert len(block) == L * 4
words = struct.unpack('<29I', block)
for i in range(L):
	assert pattern[i] == words[i]

# Hilbert self-test
hilbert.test()

print "Looks good!"
