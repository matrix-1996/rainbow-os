/*
    Copyright (c) 2016, Thierry Tremblay
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this
      list of conditions and the following disclaimer.

    * Redistributions in binary form must reproduce the above copyright notice,
      this list of conditions and the following disclaimer in the documentation
      and/or other materials provided with the distribution.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
    FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
    DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
    CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
    OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
    OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/******************************************************************************

    Program enty point


    Environment at boot

     The ATAG boot protocol defines a sane state for the system to be in before calling the kernel. Namely this is:

      - The CPU must be in SVC (supervisor) mode with both IRQ and FIQ interrupts disabled.
      - The MMU must be off, i.e. code running from physical RAM with no translated addressing.
      - Data cache must be off
      - Instruction cache may be either on or off
      - CPU register 0 must be 0
      - CPU register 1 must be the ARM Linux machine type
      - CPU register 2 must be the physical address of the parameter list


    The bootloader passes 3 arguments:
        r0 = 0     (Boot device ID)
        r1 = 0xC42 (ARM Linux Machine ID for Broadcom BCM2708 Video Coprocessor)
        r2 = ATAGS or Device Tree Blob (dtb)

    Preserve these registers! We want to pass them to raspi_main()

******************************************************************************/

.arch armv6

.section .boot

      org = 0x8000

.globl _start
_start:

    # Initialize the stack
    ldr sp, =__stack_end

    # Save device tree pointer
    push {r2}

    # Turn on unaligned memory access
    mrc p15, #0, r3, c1, c0, #0
    orr r3, #0x400000
    mcr p15, #0, r3, c1, c0, #0

    # Enabled CP10 and CP11
    mrc p15, #0, r3, c1, c0, #2
    orr r3, r3, #0xf00000

    mcr p15, #0, r3, c1, c0, #2

    # Flush prefetch buffer for FMXR below
    mov r3, #0
    mcr p15, #0, r3, c7, c5, #4

    # Enable VFP
    mov r3, #0x40000000
    fmxr FPEXC, r3

    # Clear BSS
    ldr r0, =__bss_start
    mov r1, #0
    ldr r2, =__bss_end
    sub r2, r0
    bl memset

    # Restore device tree pointer
    pop {r0}

    # Jump to raspi_main
    b raspi_main



/******************************************************************************

    Helper to introduce CPU delay

******************************************************************************/

.globl cpu_delay
cpu_delay:
    bx lr
