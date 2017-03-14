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

#include <errno.h>
#include <reent.h>
#include <signal.h>
#include <sys/stat.h>
#include <unistd.h>

#include "vgaconsole.hpp"


extern VgaConsole g_console;



extern "C" void _exit(int)
{
    //todo: reboot the system or something
    for (;;)
    {
        asm("cli; hlt");
    }
}


extern "C" int close(int)
{
    errno = EBADF;
    return -1;
}


extern "C" int fstat(int, struct stat* st)
{
    st->st_mode = S_IFCHR;
    return  0;
}


extern "C" pid_t getpid(void)
{
    return 1;
}


extern "C" int isatty(int)
{
    return 1;
}


extern "C" int kill(pid_t, int)
{
    errno = EINVAL;
    return -1;
}


extern "C" off_t lseek(int, off_t, int)
{
    return 0;
}


extern "C" int read(int, void*, size_t)
{
    return 0;
}


extern "C" int write(int, const void* buffer, size_t nbBytes)
{
    return g_console.Print((const char*)buffer, nbBytes);
}



#define HEAPSIZE (1024 * 1024)

static unsigned char _heap[HEAPSIZE];

extern "C" void* sbrk(ptrdiff_t incr)
{
    static unsigned char* heap_end;
    unsigned char *prev_heap_end;

    if (heap_end == 0 )
    {
        heap_end = _heap;
    }

    prev_heap_end = heap_end;

    if (heap_end + incr - _heap > HEAPSIZE)
    {
        errno = ENOMEM;
        return (void*)-1;
    }

    heap_end += incr;

    return prev_heap_end;
}
