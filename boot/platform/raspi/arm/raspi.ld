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

OUTPUT_FORMAT(elf32-littlearm)
ENTRY(_start)



PHDRS
{
    text PT_LOAD;
    rodata PT_LOAD;
    data PT_LOAD;
}



SECTIONS
{
    . = 0x8000;

    bootloader_image_start = .;

    .text :
    {
        *(.boot)        /* Make sure boot code is at the beginning of the image */
        *(.text)
    } : text

    .rodata :
    {
        *(.rodata*)

        . = ALIGN(4);
        __preinit_array_start = .;
        *(.preinit_array)
        __preinit_array_end = .;

        . = ALIGN(4);
        __init_array_start = .;
        *(SORT(.init_array.*));
        *(.init_array);
        __init_array_end = .;

        . = ALIGN(4);
        __fini_array_start = .;
        *(.fini_array);
        *(SORT(.fini_array.*));
        __fini_array_end = .;

    } : rodata

    .ARM.exidx :
    {
        . = ALIGN(4);
        __exidx_start = .;
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
        __exidx_end = .;
    } : rodata


    .data :
    {
        *(.data)
        *(.data*)
    } : data

    .bss (NOLOAD):
    {
        . = ALIGN(8);
        __bss_start = .;
        *(.bss)
        *(COMMON)
        __bss_end = .;

        . = ALIGN(8);
        __stack_start = .;
        . = . + 65536;
        __stack_end = .;

        . = ALIGN(8);
        __heap_start = .;
        . = . + 1024*1024;
        __heap_end = .;

    } : data

    /DISCARD/ :
    {
        *(.comment)
        *(.debug)
        *(.debug*)
        *(.eh_frame)
    }

    bootloader_image_end = .;
}
