@
@ Binary backdoor for MT1939 firmware.  ONLY for version TS01.
@
@ Install at 0xCABB8, must be no longer than 0xCAD74.
@
@ Here it overwrites the handler function for SCSI command 3C (Read Buffer).
@ Offers very little protection against bricking your device! Use at your own risk!
@
@ This replaces the normal implementation of the obscure 3C command with a new backdoor:
@
@    3C 69 <uint32_t address>                   Read 32-bit word from ARM address
@    3C 6A <uint32_t address> <uint32_t data>   Write 32-bit word to ARM address
@
@ Note that, for running code dynamically, command FF can be patched as it
@ already lives in SRAM for some reason. No need for a separate branch
@ command.
@
@ Copyright (c) 2014 Micah Elizabeth Scott
@ 
@   Permission is hereby granted, free of charge, to any person obtaining a copy of
@   this software and associated documentation files (the "Software"), to deal in
@   the Software without restriction, including without limitation the rights to
@   use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
@   the Software, and to permit persons to whom the Software is furnished to do so,
@   subject to the following conditions:
@   
@   The above copyright notice and this permission notice shall be included in all
@   copies or substantial portions of the Software.
@   
@   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
@   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
@   FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
@   COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
@   IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
@   CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
@

	.text
    .syntax unified
    .thumb
    .global _start

_start:

    push    {r3-r7, lr}
 
    ldr     r4, ref_base      @ r4 = ref  (SCSI metadata)
    ldr     r5, ra_base       @ r5 = ra   (More SCSI metadata)
    ldr     r6, [r4, #0xc]    @ r6 = cdb  (SCSI command)

    mov     r0, #0
    str     r0, [r5, #4]      @ ra[4] = 0
    str     r0, [r5]          @ ra[0] = 0   (source)
    mov     r7, #4
    str     r7, [r5, #8]      @ ra[8] = 4   (count)

    ldr     r1, proto_size
    str     r7, [r1]

    ldr     r1, mmio_regs
    ldr     r7, [r1, 0x34]    @ Size into byte1
    movs    r0, #0xFF
    lsls    r0, r0, #8
    bics    r7, r0
    mov     r0, #4
    lsls    r0, r0, #8
    orrs    r0, r7
    str     r7, [r1, 0x34]

    ldrb    r7, [r1, 5]
    movs    r0, #0x1f
    bics    r7, r0
    strb    r7, [r1, 5]

    ldrb    r1, [r6, #2]
    lsls    r0, r1, #24
    ldrb    r1, [r6, #3]
    lsls    r1, r1, #16
    orrs    r0, r1
    ldrb    r1, [r6, #4]
    lsls    r1, r1, #8
    orrs    r0, r1
    ldrb    r1, [r6, #5]
    orrs    r0, r1            @ r0 = cdb[2,3,4,5]

    strb    r0, [r1, 8]       @ Write bytes
    lsrs    r0, r0, #8
    strb    r0, [r1, 8]
    lsrs    r0, r0, #8
    strb    r0, [r1, 8]
    lsrs    r0, r0, #8
    strb    r0, [r1, 8]

    mov     r0, #1            @ Done flag
    ldr     r1, flag_2000ce6
    strb    r0, [r1]
 
    pop     {r3-r7, pc}

    .align 2

ref_base:      .word   0x2000cec
ra_base:       .word   0x2000d80

zr_base:       .word   0x2000d60

mmio_regs:     .word   0x40400a0
proto_size:    .word   0x4042154
flag_2000ce6:  .word   0x2000ce6

