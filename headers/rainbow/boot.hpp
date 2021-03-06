/*
    Copyright (c) 2017, Thierry Tremblay
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

#ifndef RAINBOW_BOOT_HPP
#define RAINBOW_BOOT_HPP

#include <stdint.h>



// The order these memory types are defined is important!
// When the firmware returns overlapping memory ranges, higher values take precedence.
enum MemoryType
{
    MemoryType_Available,           // Conventional memory (RAM)
    MemoryType_Persistent,          // Works like conventional memory, but is persistent
    MemoryType_Unusable,            // Memory in which errors have been detected
    MemoryType_Bootloader,          // Bootloader
    MemoryType_Kernel,              // Kernel
    MemoryType_AcpiReclaimable,     // ACPI Tables (can be reclaimed once parsed)
    MemoryType_AcpiNvs,             // ACPI Non-Volatile Storage
    MemoryType_Firmware,            // Firmware (e.g. EFI runtime services, ARM Device Tree, ...)
    MemoryType_Reserved,            // Reserved / unknown / do not use
};


enum MemoryFlag
{
    MemoryFlag_Code         = 1,    // Memory is code
    MemoryFlag_ReadOnly     = 2,    // Memory is read-only
};


struct MemoryDescriptor
{
    MemoryType  type;               // Memory type
    uint32_t    flags;              // Memory flags
    uint64_t    address;            // Start of memory range
    uint64_t    size;               // Size of memory range in bytes
};

static_assert(sizeof(MemoryDescriptor) == 24, "MemoryDescriptor should be packed to 24 bytes");



static const uint32_t RAINBOW_BOOT_VERSION = 1;

struct BootInfo
{
    uint32_t    version;                // Version (RAINBOW_BOOT_VERSION)

    uint32_t    memoryDescriptorCount;  // Number of available memory descriptors
    uint64_t    memoryDescriptors;      // Memory descriptors address

    uint64_t    initrdAddress;          // initrd physical address in memory
    uint64_t    initrdSize;             // Size of initrd

    uint64_t    pageTables;             // Root of page tables
};



#endif
