@
@ Binary backdoor for MT1939 firmware.  ONLY for version TS01.
@
@ Installs at 0xC9600, over the SCSI command AC (Get Performance Data) handler.
@
@ This replaces a relatively obscure SCSI command with a backdoor we can use to
@ peek and poke ARM memory. Unlike the Read Buffer (3C) command which is _almost_
@ already quite a good backdoor, we prefer to operate from the ARM bus point of
@ view rather than on DRAM. To accomplish this, we eschew DMA entirely and operate
@ on individual 32-bit words from the ARM side. We start the hack job with a command
@ that already uses the PIO mode instead of DMA mode for its response.
@
@ New commands:
@
@   ac 65 65 6b [address]                   peek
@   ac 6f 6b 65 [address] [data]            poke
@
@ Commands return an 8-byte packet:
@
@   [address]  [data]
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

    push    {lr}

    @ Start a FIFO transfer (without DMA) of at most 8 bytes.
    @ Writes to bits 15:8 in [40400e0]

    mov     r0, #8

    ldr     r2, =0x40400c0
    ldr     r1, [r2, #20]
    movs    r3, #0xff
    lsls    r3, r3, #8
    bics    r1, r3
    lsls    r0, r0, #8
    orrs    r1, r0
    str     r1, [r2, #20]

    @ Read command information from SCSI CDB. Normally we would do this before
    @ committing to send back a response, but this makes the patch easier to
    @ debug. If we dont find a command, instead of sending back a SCSI error
    @ just send back a signature that lets folks know the patch is working.

    ldr     r0, =0x2000cf8      @ SCSI shared metadata block in RAM
    ldr     r1, [r0, #0]        @ Pointer to SCSI CDB structure
    bl      unaligned_read32

    ldr     r2, =0x6b6565ac     @ Peek
    subs    r0, r2
    cmp     r0, #0
@    b       haxx

@    ldr     r2, =0x656b6fac     @ Poke
@    subs    r0, r2  
@    beq     cmd_poke

  @mov r0,r2
  bl fifo_write32

    ldr     r0, signature+0x0   @ No command recognized, send back signature
    bl      fifo_write32
    ldr     r0, signature+0x4
    bl      fifo_write32

haxx:
    ldr     r0, signature+0x8
    bl      fifo_write32

complete:
    ldr     r1, =0x2000ce6      @ Finish the FIFO response
    movs    r0, #1
    strb    r0, [r1]
    pop     {pc}                @ Return from patched handler


    @ Peek(address) -> (address, data)

cmd_peek:
    bl      unaligned_read32    @ Next word from CDB pointer in r1
    mov     r3, r0
    bl      fifo_write32        @ Echo address back
    ldr     r0, [r3] 
    bl      fifo_write32
    b       complete


    @ Poke(address, data) -> (address, data)

cmd_poke:
    bl      unaligned_read32
    mov     r3, r0
    bl      fifo_write32        @ Echo address
    bl      unaligned_read32
    str     r0, [r3]
    bl      fifo_write32        @ Echo data after write
    b       complete


    @ Read a 32-bit number at [r1] one byte at a time, incrementing as we go.
    @ Little endian. For data that might not be aligned.
    @ Pointer in r1, trashes r2, result in r0.

unaligned_read32:
    ldrb    r0, [r1]
    ldrb    r2, [r1, 1]
    lsls    r2, r2, #8
    orrs    r0, r2
    ldrb    r2, [r1, 2]
    lsls    r2, r2, #16
    orrs    r0, r2
    ldrb    r2, [r1, 3]
    lsls    r2, r2, #24
    orrs    r0, r2
    adds    r1, #4
    bx      lr

    @ Write a 32-bit number to the response FIFO, one byte at a time, in
    @ little endian order. This uses the byte wide PIO FIFO, instead of using
    @ the DMA engine. DMA is great if our data is in DRAM, but for this to be
    @ a good backdoor it should have an ARM eye view.
    @ Arg in r0, trashes r2.

fifo_write32:
    ldr     r2, =0x40400a8
    strb    r0, [r2]
    lsrs    r0, #8
    strb    r0, [r2]
    lsrs    r0, #8
    strb    r0, [r2]
    lsrs    r0, #8
    strb    r0, [r2]
    bx      lr

    @ Something to know us by

    .pool
    .align 4
signature:
    .ascii "~MeS`14 v.01"
    .word  -1
