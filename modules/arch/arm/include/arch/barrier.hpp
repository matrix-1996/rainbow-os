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

#ifndef _RAINBOW_ARCH_ARM_BARRIER_HPP
#define _RAINBOW_ARCH_ARM_BARRIER_HPP

/*
    From https://github.com/raspberrypi/firmware/wiki/Accessing-mailboxes:

    Cache flushing:
        mov r3, #0                      # The read register Should Be Zero before the call
        mcr p15, 0, r3, C7, C6, 0       # Invalidate Entire Data Cache
        mcr p15, 0, r3, c7, c10, 0      # Clean Entire Data Cache
        mcr p15, 0, r3, c7, c14, 0      # Clean and Invalidate Entire Data Cache
        mcr p15, 0, r3, c7, c10, 4      # Data Synchronization Barrier
        mcr p15, 0, r3, c7, c10, 5      # Data Memory Barrier

    MemoryBarrier:
        mcr p15, 0, r3, c7, c5, 0       # Invalidate instruction cache
        mcr p15, 0, r3, c7, c5, 6       # Invalidate BTB
        mcr p15, 0, r3, c7, c10, 4      # Drain write buffer
        mcr p15, 0, r3, c7, c5, 4       # Prefetch flush
        mov pc, lr                      # Return
*/


//todo: implement these properly
#define read_barrier()  __asm__ __volatile__ ("" : : : "memory")
#define write_barrier() __asm__ __volatile__ ("" : : : "memory")

#endif
