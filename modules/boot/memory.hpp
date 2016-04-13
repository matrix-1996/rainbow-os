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

#ifndef _RAINBOW_BOOT_MEMORY_HPP
#define _RAINBOW_BOOT_MEMORY_HPP

#include <rainbow/types.h>


#define MEMORY_MAX_ENTRIES 1024


#if defined(__i386__) || defined(__x86_64__)
#define MEMORY_PAGE_SIZE 4096ull
#endif

#define MEMORY_ROUND_PAGE_DOWN(x) ((x) & ~(MEMORY_PAGE_SIZE - 1))
#define MEMORY_ROUND_PAGE_UP(x) (((x) + MEMORY_PAGE_SIZE - 1) & ~(MEMORY_PAGE_SIZE - 1))

// TODO: if we tracked memory using pages instead of bytes, we wouldn't need to throw away the last page!
#define MEMORY_MAX_PHYSICAL_ADDRESS (~(MEMORY_PAGE_SIZE - 1))


// The order these memory types are defined is important!
// When handling overlapping memory ranges, higher values take precedence.
enum MemoryType
{
    MemoryType_Available,       // Available memory (RAM)
    MemoryType_Unusable,        // Memory in which errors have been detected
    MemoryType_Bootloader,      // Bootloader
    MemoryType_BootModule,      // Boot module
    MemoryType_Launcher,        // Launcher
    MemoryType_AcpiReclaimable, // ACPI Tables (can be reclaimed once parsed)
    MemoryType_AcpiNvs,         // ACPI Non-Volatile Storage
    MemoryType_FirmwareRuntime, // Firmware Runtime Memory (e.g. EFI runtime services)
    MemoryType_Reserved,        // Reserved / unknown / do not use
};



struct MemoryEntry
{
    physaddr_t start;   // Start of memory range
    physaddr_t end;     // End of memory range
    MemoryType type;    // Type of memory
};


enum MemoryZone
{
    MemoryZone_Low,                 // x86: 0 - 1 MB
    MemoryZone_ISA,                 // x86: 1 MB - 16 MB
    MemoryZone_Normal,              // x86: 16 MB - 4 GB
    MemoryZone_High,                // x86: 4 GB - ...
};



class MemoryMap
{
public:

    MemoryMap();

    void AddEntry(MemoryType type, physaddr_t start, physaddr_t end);

    // Allocate memory within the specified physical address range
    // Not expected to return on error, but will return -1
    physaddr_t AllocInRange(MemoryType type, size_t size, physaddr_t minAddress, physaddr_t maxAddress, size_t alignment = 0);

    // Allocate memory within the specified memory zone
    // Not expected to return on error, but will return -1
    physaddr_t Alloc(MemoryType type, size_t size, MemoryZone zone, size_t alignment = 0);

    void Print();

    void Sanitize();


private:

    void AddEntryHelper(MemoryType type, physaddr_t start, physaddr_t end);

    MemoryEntry  m_entries[MEMORY_MAX_ENTRIES]; // Memory entries
    int          m_count;                       // Memory entry count
};



#endif
