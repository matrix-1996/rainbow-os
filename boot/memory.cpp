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

#include "memory.hpp"
#include <assert.h>
#include <inttypes.h>
#include <stdio.h>


// Sanity checks
static_assert(sizeof(MemoryDescriptor) == 24, "MemoryDescriptor should be packed to 24 bytes");



static const physaddr_t PAGE_MAX = (((physaddr_t)-1) >> MEMORY_PAGE_SHIFT) + 1;



MemoryMap::MemoryMap()
{
    m_count = 0;
}



void MemoryMap::AddBytes(MemoryType type, uint32_t flags, uint64_t address, uint64_t bytesCount)
{
    if (bytesCount == 0)
        return;

    physaddr_t pageStart;
    physaddr_t pageEnd;

    if (type == MemoryType_Available)
    {
        // Calculate start page
        pageStart = address >> MEMORY_PAGE_SHIFT; // 0..PAGE_MAX-1

        // Round start address up to the next page boundary
        physaddr_t delta = address & (MEMORY_PAGE_SIZE-1);
        if (delta > 0)
        {
            ++pageStart; // 0..PAGE_MAX

            // Check if we have enough in bytesCount to compensate for the page rounding
            delta = MEMORY_PAGE_SIZE - delta;
            if (delta >= bytesCount)
                return;

            // Fix bytes count
            bytesCount = bytesCount - delta;
        }

        // Calculate end page (rounding down)
        pageEnd = pageStart + (bytesCount >> MEMORY_PAGE_SHIFT); // 0..PAGE_MAX*2-1
    }
    else
    {
        // Calculate start page (rounded down) and end page (rounded up)
        pageStart = address >> MEMORY_PAGE_SHIFT; // 0..PAGE_MAX-1
        pageEnd = pageStart + (bytesCount >> MEMORY_PAGE_SHIFT); // 0..PAGE_MAX*2-1

        // Calculate how many bytes we missed with our roundings above
        physaddr_t missing = (address & (MEMORY_PAGE_SIZE-1)) + (bytesCount & (MEMORY_PAGE_SIZE-1));

        // Fix page end to account for missing bytes
        pageEnd = pageEnd + (missing >> MEMORY_PAGE_SHIFT); // 0..PAGE_MAX*2
        if (missing & (MEMORY_PAGE_SIZE-1))
            ++pageEnd; // 0..PAGE_MAX*2+1
    }

    if (pageEnd > PAGE_MAX)
        pageEnd = PAGE_MAX; // 0..PAGE_MAX

    AddPageRange(type, flags, pageStart, pageEnd);
}



void MemoryMap::AddPageRange(MemoryType type, uint32_t flags, uint64_t pageStart, uint64_t pageEnd)
{
    // Ignore invalid entries (including zero-sized ones)
    if (pageStart >= pageEnd)
        return;

    // Walk all existing entries looking for overlapping ranges
    for (int i = 0; i != m_count; ++i)
    {
        MemoryEntry* entry = &m_entries[i];

        // Always check for overlaps!
        if (pageStart < entry->pageEnd() && pageEnd > entry->pageStart())
        {
            // Copy the entry as we will delete it
            MemoryEntry other = *entry;

            // Delete existing entry
            --m_count;
            for (int j = i; j != m_count; ++j)
            {
                m_entries[j] = m_entries[j+1];
            }

            // Handle left piece
            if (pageStart < other.pageStart())
                AddPageRange(type, flags, pageStart, other.pageStart());
            else if (other.pageStart() < pageStart)
                AddPageRange(other.type, other.flags, other.pageStart(), pageStart);

            // Handle overlap
            MemoryType overlapType = type < other.type ? other.type : type;
            uint32_t overlapFlags = flags | other.flags;
            physaddr_t overlapStart = pageStart < other.pageStart() ? other.pageStart() : pageStart;
            physaddr_t overlapEnd = pageEnd < other.pageEnd() ? pageEnd : other.pageEnd();
            AddPageRange(overlapType, overlapFlags, overlapStart, overlapEnd);

            // Handle right piece
            if (pageEnd < other.pageEnd())
                AddPageRange(other.type, other.flags, pageEnd, other.pageEnd());
            else if (other.pageEnd() < pageEnd)
                AddPageRange(type, flags, other.pageEnd(), pageEnd);

            return;
        }
    }

    // No overlap, try to merge with an existing entry
    for (int i = 0; i != m_count; ++i)
    {
        MemoryEntry* entry = &m_entries[i];

        // Same type and flags?
        if (type != entry->type || flags != entry->flags)
            continue;

        // Check for overlaps / adjacency
        if (pageStart <= entry->pageEnd() && pageEnd >= entry->pageStart())
        {
            // Update existing entry in-place
            if (pageStart < entry->pageStart())
                entry->SetStart(pageStart);

            if (pageEnd > entry->pageEnd())
                entry->SetEnd(pageEnd);

            return;
        }
    }

    // If the table is full, we can't add more entries
    if (m_count == MEMORY_MAX_ENTRIES)
        return;

    // Insert this new entry
    MemoryEntry* entry = &m_entries[m_count];
    entry->Initialize(type, flags, pageStart, pageEnd);
    ++m_count;
}



physaddr_t MemoryMap::AllocateBytes(MemoryType type, size_t bytesCount, uint64_t maxAddress)
{
    size_t pageCount = bytesCount >> MEMORY_PAGE_SHIFT;

    if (bytesCount & (MEMORY_PAGE_SIZE-1))
        ++pageCount;

    return AllocatePages(type, pageCount, maxAddress);
}



physaddr_t MemoryMap::AllocatePages(MemoryType type, size_t pageCount, uint64_t maxAddress)
{
    // Fail early by validating pageCount range
    if (pageCount == 0)
        return MEMORY_ALLOC_FAILED;

    const physaddr_t minPage = 1;  //  Don't allocate NULL address

    physaddr_t maxPage = (maxAddress + 1) >> MEMORY_PAGE_SHIFT;

    // Allocate from highest memory as possible (low memory is precious, on PC anyways)
    // To do this, we need to look at all free entries
    physaddr_t allocStart = MEMORY_ALLOC_FAILED;
    uint32_t flags = 0;

    for (int i = 0; i != m_count; ++i)
    {
        const MemoryEntry& entry = m_entries[i];

        if (entry.type != MemoryType_Available)
            continue;

        // Calculate entry's overlap with what we need
        const physaddr_t overlapStart = entry.pageStart() < minPage ? minPage : entry.pageStart();
        const physaddr_t overlapEnd = entry.pageEnd() > maxPage ? maxPage : entry.pageEnd();

        if (overlapStart > overlapEnd || overlapEnd - overlapStart < pageCount)
            continue;

        physaddr_t candidate = overlapEnd - pageCount;

        if (allocStart == MEMORY_ALLOC_FAILED || candidate > allocStart)
        {
            allocStart = candidate;
            flags = entry.flags;
        }
    }

    if (allocStart == MEMORY_ALLOC_FAILED)
        return MEMORY_ALLOC_FAILED;

    AddPageRange(type, flags, allocStart, allocStart + pageCount);

    return allocStart << MEMORY_PAGE_SHIFT;
}



void MemoryMap::Print()
{
    printf("Memory map:\n");

    for (int i = 0; i != m_count; ++i)
    {
        const MemoryEntry& entry = m_entries[i];

        const char* type = "Unknown";

        switch (entry.type)
        {
        case MemoryType_Available:
            type = "Available";
            break;

        case MemoryType_Persistent:
            type = "Persistent";
            break;

        case MemoryType_Unusable:
            type = "Unusable";
            break;

        case MemoryType_Bootloader:
            type = "Bootloader";
            break;

        case MemoryType_Kernel:
            type = "Kernel";
            break;

        case MemoryType_AcpiReclaimable:
            type = "ACPI Reclaimable";
            break;

        case MemoryType_AcpiNvs:
            type = "ACPI Non-Volatile Storage";
            break;

        case MemoryType_Firmware:
            type = "Firmware Runtime";
            break;

        case MemoryType_Reserved:
            type = "Reserved";
            break;
        }

        printf("    %016" PRIx64 " - %016" PRIx64 " : %s\n",
            entry.pageStart() << MEMORY_PAGE_SHIFT,
            entry.pageEnd() << MEMORY_PAGE_SHIFT,
            type);
    }
}



void MemoryMap::Sanitize()
{
    // We will sanitize the memory map by doing an insert-sort of all entries
    // MemoryMap::AddPageRange() will take care of merging adjacent blocks.
    MemoryMap sorted;

    while (m_count > 0)
    {
        MemoryEntry* candidate = &m_entries[0];

        for (int i = 1; i != m_count; ++i)
        {
            MemoryEntry* entry = &m_entries[i];
            if (entry->pageStart() < candidate->pageStart())
                candidate = entry;
            else if (entry->pageStart() == candidate->pageStart() && entry->pageEnd() < candidate->pageEnd())
                candidate = entry;
        }

        sorted.AddPageRange(candidate->type, candidate->flags, candidate->pageStart(), candidate->pageEnd());

        *candidate = m_entries[--m_count];
    }

    *this = sorted;
}
