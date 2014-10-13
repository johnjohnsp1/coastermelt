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
@ Micah Elizabeth Scott, 2014
@ This file is released into the public domain.

	.text
    .syntax unified
    .thumb
_start:

    push    {r3-r7, lr}

    ldr     r4, ref_base
    add     r4, #0xc          @ r4 = ref[c]
    ldr     r6, [r4]          @ r6 = SCSI CDB
    
    ldr     r12, func_1       @ Prepare for response
    blx     r12

    mov     r0, #0
    str     r0, [r4, #4]      @ ref[4] = 0

    ldrb    r0, [r6, #3]
    lsls    r0, #16
    ldrb    r1, [r6, #4]
    lsls    r1, #8
    orrs    r0, r1
    ldrb    r1, [r6, #5]
    orrs    r0, r1
    str     r0, [r4]          @ ref[0] = cdb[3,4,5]

    ldrb    r0, [r6, #6]
    lsls    r0, #16
    ldrb    r1, [r6, #7]
    lsls    r1, #8
    orrs    r0, r1
    ldrb    r1, [r6, #8]
    orrs    r0, r1
    str     r0, [r4, #8]      @ ref[8] = cdb[6,7,8]

    mov     r2, r0            @ R2 = count = ref[8]
    ldr     r1, [r4]          @ R1 = ARM source = ref[0]
    ldr     r0, [r4, #4]      @ R0 = DRAM dest = ref[4]

    ldr     r12, func_2       @ Copy to DRAM
    blx     r12

    ldr     r0, [r4, #4]      @ DRAM address for DMA
    ldr     r1, mmio_ptr
    str     r0, [r1]

    ldr     r0, [r4, #8]      @ Count for DMA
    ldr     r1, mmio_count
    str     r0, [r1]

    mov     r0, #1            @ Done flag
    mov     r1, flag_2000ce6
    str     r1, [r0]

    pop     {r3-r7, pc}

               .hword  0
ref_base:      .word   0x2000d80
mmio_ptr:      .word   0x40300e8
mmio_count:    .word   0x4042154
flag_2000ce6:  .word   0x2000ce6
func_1:        .word     0x19654
func_2:        .word     0x1a668

