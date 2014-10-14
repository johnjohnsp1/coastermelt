@
@ Binary backdoor for MT1939 optical drive firmware.
@
@ Installs at 0xC9600, over the SCSI command AC (Get Performance Data) handler.
@ ONLY for firmware version TS01.
@
@ This replaces a relatively obscure SCSI command with a backdoor we can use to
@ peek and poke ARM memory. Unlike the Read Buffer (3C) command which is _almost_
@ already quite a good backdoor, we prefer to operate from the ARM bus point of
@ view rather than on DRAM. To accomplish this, we eschew DMA entirely and operate
@ on individual 32-bit words from the ARM side. We start the hack job with a command
@ that already uses the PIO mode instead of DMA mode for its response.
@
@ Commands:
@
@   Default       ac xx                                         --> [string, 12 bytes]
@   Peek          ac 65 65 6b [address/LE32]                    --> [address/LE32] [data/LE32]
@   Poke          ac 6f 6b 65 [address/LE32] [data/LE32]        --> [address/LE32] [data/LE32]
@   Peek byte     ac 65 65 42 [address/LE32]                    --> [address/LE32] [data/LE32]
@   Poke byte     ac 6f 6b 42 [address/LE32] [data/LE32]        --> [address/LE32] [data/LE32]
@   BLX           ac 42 4c 58 [address/LE32] [r0/LE32]          --> [r0/LE32] [r1/LE32]
@   Read block    ac 6c 6f 63 [address/LE32] [wordcount/LE32]   --> [data/LE32] * wordcount
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

    @ Start a FIFO transfer (without DMA), claim that the length is 8
    @ bytes. I think this length is a minimum? Not sure how it is validated
    @ if at all.  Writes to bits 15:8 in [40400e0]

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
    cmp     r0, r2
    beq.n   cmd_peek

    ldr     r2, =0x656b6fac     @ Poke
    cmp     r0, r2  
    beq.n   cmd_poke

    ldr     r2, =0x584c42ac     @ BLX
    cmp     r0, r2  
    beq.n   cmd_blx

    ldr     r2, =0x636f6cac     @ Read block
    cmp     r0, r2
    beq.n   cmd_read_block

    ldr     r2, =0x426565ac     @ Peek byte
    cmp     r0, r2
    beq.n   cmd_peek_byte

    ldr     r2, =0x426b6fac     @ Poke byte
    cmp     r0, r2  
    beq.n   cmd_poke_byte

    ldr     r0, signature+0x0   @ No command recognized, send back signature
    bl      fifo_write32
    ldr     r0, signature+0x4
    bl      fifo_write32
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
    b.n     complete


    @ Poke(address, data) -> (address, data)

cmd_poke:
    bl      unaligned_read32
    mov     r3, r0
    bl      fifo_write32        @ Echo address
    bl      unaligned_read32
    str     r0, [r3]
    bl      fifo_write32        @ Echo data after write
    b.n     complete


    @ BLX(address, arg0) -> (result0, result1)

cmd_blx:
    bl      unaligned_read32
    mov     r3, r0
    bl      unaligned_read32
    blx     r3
    bl      fifo_write32
    mov     r0, r1
    bl      fifo_write32
    b.n     complete        


    @ ReadBlock(address, wordcount) -> (words)

    @ This is redundant with Peek, but the patch is more complicated and it
    @ may not always work. So we only use it when speed is important.

    @ This seems to work with up to 0x1D words of data (116 bytes). This seems
    @ like a limitation related to this particular SCSI command. I suspect
    @ that there does not exist one fully generic SCSI or USB transport at a
    @ level we can see it so far. It appears instead that everything is split
    @ very finely between different hardware components, and there is likely a
    @ parallel state machine on the 8051 firmware that knows about these same
    @ size limits in the code we just replaced.

cmd_read_block:
    push    {r4-r5}                 @ Get some breathing room
    bl      unaligned_read32        @ Store args in r4=address and r5=wordcount
    mov     r4, r0
    bl      unaligned_read32
    mov     r5, r0
    b.n     word_test

word_loop:
    ldr     r0, [r4]
    bl      fifo_write32
    subs    r5, #1
    adds    r4, #4

word_test:
    cmp     r5, #0
    bne.n   word_loop

    pop     {r4-r5}
    b.n     complete


    @ PeekByte(address) -> (address, data)

cmd_peek_byte:
    bl      unaligned_read32    @ Next word from CDB pointer in r1
    mov     r3, r0
    bl      fifo_write32        @ Echo address back
    ldrb    r0, [r3] 
    bl      fifo_write32
    b.n     complete


    @ PokeByte(address, data) -> (address, data)

cmd_poke_byte:
    bl      unaligned_read32
    mov     r3, r0
    bl      fifo_write32        @ Echo address
    bl      unaligned_read32
    strb    r0, [r3]
    bl      fifo_write32        @ Echo data after write
    b.n     complete


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
    .ascii "~MeS`14 "
    .ascii "v.02    "
