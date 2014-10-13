@
@ Binary backdoor for MT1939 firmware.  ONLY for version TS01.
@
@ Installs at 0xC9600, over the SCSI command AC (Get Performance Data) handler.
@ This area has room for up to 1936 bytes of patch code and data. So generous!
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
@ Commands return a 16-byte packet, as three little endian 32-bit words:
@
@   [command echo]  [address]  [data]  [status]
@
@ In the patch, these uniform 16-byte structures are stured in r4-r7.
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

    push    {r4-r7, lr}

    @ Load 12 bytes of args from the CDB {r4-r6}, in little endian order

    ldr     r0, =0x2000cf8      @ SCSI shared metadata block in RAM
    ldr     r0, [r0, #0]        @ Pointer to SCSI CDB structure

    ldr     r4, [r0, #0]        @ Not aligned enough to use LDM sadly
    ldr     r5, [r0, #4]
    ldr     r6, [r0, #8]
    movs    r7, #0

@    ldr     r0, =0x6b6565ac
@    cmp     r0, r4
@    beq     cmd_peek

@    ldr     r0, =0x656b6fac
@    cmp     r0, r4
@    beq     cmd_poke

   ldr     r5, [r4]
 
@cmd_unknown:
@    mov     r4, r7
@    mov     r5, r7
@    mov     r6, r7
@    b       cmd_complete
@
@cmd_peek:
@    ldr     r5, [r4]
@    b       cmd_complete
@
@cmd_poke:
@    str     r5, [r4]
@    b       cmd_complete


    @ Write back a 128-bit structure {r4-r7} as a SCSI response
cmd_complete:

    mov     r0, #16
    bl      fifo_begin_with_length

    mov     r0, r4
    bl      fifo_write32
    mov     r0, r5
    bl      fifo_write32
    mov     r0, r6
    bl      fifo_write32
    mov     r0, r7
    bl      fifo_write32

    bl      fifo_done
    pop     {r4-r7, pc}

    @ Start a FIFO transfer (without DMA) of at most r0 bytes.
    @ Writes to bits 15:8 in [40400e0]

fifo_begin_with_length:
    ldr     r2, =0x40400c0
    ldr     r1, [r2, #20]
    movs    r3, #0xff
    lsls    r3, r3, #8
    bics    r1, r3
    lsls    r0, r0, #8
    orrs    r1, r0
    str     r1, [r2, #20]
    bx      lr

    @ Write a 32-bit number to the response FIFO, one byte at a time, in
    @ little endian order. This uses the byte wide PIO FIFO, instead of using
    @ the DMA engine. DMA is great if our data is in DRAM, but for this to be
    @ a good backdoor it should have an ARM's eye view.

fifo_write32:
    ldr     r1, =0x40400a8
    strb    r0, [r1]
    lsrs    r0, #8
    strb    r0, [r1]
    lsrs    r0, #8
    strb    r0, [r1]
    lsrs    r0, #8
    strb    r0, [r1]
    bx      lr

    @ Finish with a FIFO response

fifo_done:
    ldr     r1, =0x2000ce6
    movs    r0, #1
    strb    r0, [r1]
    bx      lr

