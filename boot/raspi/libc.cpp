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

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include <stdint.h>

#if defined(__ARM_ARCH_6ZK__)
#define PERIPHERAL_BASE 0x02000000  // Peripheral Base Address (Original Raspberry Pi)
#else
#define PERIPHERAL_BASE 0x3F000000  // Peripheral Base Address (Raspberry Pi 2, 3)
#endif

#define GPIO_BASE                           (PERIPHERAL_BASE + 0x00200000)  // GPIO Base Address
#define GPIO_GPFSEL0                        (GPIO_BASE + 0x00000000)        // GPIO Function Select 0
#define GPIO_GPFSEL1                        (GPIO_BASE + 0x00000004)        // GPIO Function Select 1
#define GPIO_GPFSEL2                        (GPIO_BASE + 0x00000008)        // GPIO Function Select 2
#define GPIO_GPFSEL3                        (GPIO_BASE + 0x0000000C)        // GPIO Function Select 3
#define GPIO_GPFSEL4                        (GPIO_BASE + 0x00000010)        // GPIO Function Select 4
#define GPIO_GPFSEL5                        (GPIO_BASE + 0x00000014)        // GPIO Function Select 5
#define GPIO_GPSET0                         (GPIO_BASE + 0x0000001C)        // GPIO Pin Output Set 0
#define GPIO_GPSET1                         (GPIO_BASE + 0x00000020)        // GPIO Pin Output Set 1
#define GPIO_GPCLR0                         (GPIO_BASE + 0x00000028)        // GPIO Pin Output Clear 0
#define GPIO_GPCLR1                         (GPIO_BASE + 0x0000002C)        // GPIO Pin Output Clear 1
#define GPIO_GPLEV0                         (GPIO_BASE + 0x00000034)        // GPIO Pin Level 0
#define GPIO_GPLEV1                         (GPIO_BASE + 0x00000038)        // GPIO Pin Level 1
#define GPIO_GPEDS0                         (GPIO_BASE + 0x00000040)        // GPIO Pin Event Detect Status 0
#define GPIO_GPEDS1                         (GPIO_BASE + 0x00000044)        // GPIO Pin Event Detect Status 1
#define GPIO_GPREN0                         (GPIO_BASE + 0x0000004C)        // GPIO Pin Rising Edge Detect Enable 0
#define GPIO_GPREN1                         (GPIO_BASE + 0x00000050)        // GPIO Pin Rising Edge Detect Enable 1
#define GPIO_GPFEN0                         (GPIO_BASE + 0x00000058)        // GPIO Pin Falling Edge Detect Enable 0
#define GPIO_GPFEN1                         (GPIO_BASE + 0x0000005C)        // GPIO Pin Falling Edge Detect Enable 1
#define GPIO_GPHEN0                         (GPIO_BASE + 0x00000064)        // GPIO Pin High Detect Enable 0
#define GPIO_GPHEN1                         (GPIO_BASE + 0x00000068)        // GPIO Pin High Detect Enable 1
#define GPIO_GPLEN0                         (GPIO_BASE + 0x00000070)        // GPIO Pin Low Detect Enable 0
#define GPIO_GPLEN1                         (GPIO_BASE + 0x00000074)        // GPIO Pin Low Detect Enable 1
#define GPIO_GPAREN0                        (GPIO_BASE + 0x0000007C)        // GPIO Pin Async. Rising Edge Detect 0
#define GPIO_GPAREN1                        (GPIO_BASE + 0x00000080)        // GPIO Pin Async. Rising Edge Detect 1
#define GPIO_GPAFEN0                        (GPIO_BASE + 0x00000088)        // GPIO Pin Async. Falling Edge Detect 0
#define GPIO_GPAFEN1                        (GPIO_BASE + 0x0000008C)        // GPIO Pin Async. Falling Edge Detect 1
#define GPIO_GPPUD                          (GPIO_BASE + 0x00000094)        // GPIO Pin Pull-up/down Enable
#define GPIO_GPPUDCLK0                      (GPIO_BASE + 0x00000098)        // GPIO Pin Pull-up/down Enable Clock 0
#define GPIO_GPPUDCLK1                      (GPIO_BASE + 0x0000009C)        // GPIO Pin Pull-up/down Enable Clock 1
#define GPIO_TEST                           (GPIO_BASE + 0x000000B0)        // GPIO Test

#define UART0_BASE      (GPIO_BASE + 0x00001000)
#define UART0_DR        (UART0_BASE + 0x00)
#define UART0_RSRECR    (UART0_BASE + 0x04)
#define UART0_FR        (UART0_BASE + 0x18)
#define UART0_ILPR      (UART0_BASE + 0x20)
#define UART0_IBRD      (UART0_BASE + 0x24)
#define UART0_FBRD      (UART0_BASE + 0x28)
#define UART0_LCRH      (UART0_BASE + 0x2C)
#define UART0_CR        (UART0_BASE + 0x30)
#define UART0_IFLS      (UART0_BASE + 0x34)
#define UART0_IMSC      (UART0_BASE + 0x38)
#define UART0_RIS       (UART0_BASE + 0x3C)
#define UART0_MIS       (UART0_BASE + 0x40)
#define UART0_ICR       (UART0_BASE + 0x44)
#define UART0_DMACR     (UART0_BASE + 0x48)
#define UART0_ITCR      (UART0_BASE + 0x80)
#define UART0_ITIP      (UART0_BASE + 0x84)
#define UART0_ITOP      (UART0_BASE + 0x88)
#define UART0_TDR       (UART0_BASE + 0x8C)


static void cpu_delay()
{
    asm volatile("nop");
}

// Wait at least 150 GPU cycles (and not 150 CPU cycles)
static void gpio_delay()
{
    for (int i = 0; i != 150; ++i)
    {
        cpu_delay();
    }
}

#define read_barrier()  __asm__ __volatile__ ("" : : : "memory")
#define write_barrier() __asm__ __volatile__ ("" : : : "memory")


inline uint32_t mmio_read32(const volatile void* address)
{
    uint32_t value;
    asm volatile("ldr %1, %0" : "+Qo" (*(volatile uint32_t*)address), "=r" (value));
    read_barrier();
    return value;
}


inline void mmio_write32(volatile void* address, uint32_t value)
{
    write_barrier();
    asm volatile("str %1, %0" : "+Qo" (*(volatile uint32_t*)address) : "r" (value));
}


#define GET32(x) mmio_read32((void*)x)
#define PUT32(x,y) mmio_write32((void*)x, y)


class RaspberryUart
{
public:

    void Initialize()
    {
        unsigned int ra;

        // Disable UART 0
        PUT32(UART0_CR, 0);

        // Map UART0 (alt function 0) to GPIO pins 14 and 15
        ra = GET32(GPIO_GPFSEL1);
        ra &= ~(7 << 12);   //gpio14
        ra |= 4 << 12;      //alt0
        ra &= ~(7 << 15);   //gpio15
        ra |= 4 << 15;      //alt0
        PUT32(GPIO_GPFSEL1, ra);
        PUT32(GPIO_GPPUD, 0);
        gpio_delay();
        PUT32(GPIO_GPPUDCLK0, 3 << 14);
        gpio_delay();
        PUT32(GPIO_GPPUDCLK0, 0);

        // Clear pending interrupts
        PUT32(UART0_ICR, 0x7FF);

        // Baud rate
        // Divider = UART_CLOCK / (16 * Baud)
        // Fraction = (Fraction part * 64) + 0.5

        // Raspberry 2: UART CLOCK = 3000000 (3MHz)
        // Divider = 3000000 / (16 * 115200) = 1.627    --> 1
        // Fraction = (.627 * 64) + 0.5 = 40.6          --> 40
        //PUT32(UART0_IBRD, 1);
        //PUT32(UART0_FBRD, 40);

        // Raspberry 3: UART_CLOCK = 48000000 (48 MHz)
        // Divider = 48000000 / (16 * 115200) = 26.041766667  --> 26
        // Fraction = (.041766667 * 64) + 0.5 = 3.1666667     --> 3
        PUT32(UART0_IBRD, 26);
        PUT32(UART0_FBRD, 3);

        // Enable FIFO, 8-N-1
        PUT32(UART0_LCRH, 0x70);

        // Mask all interrupts
        PUT32(UART0_IMSC, 0x7F2);

        // Enable UART0 (receive + transmit)
        PUT32(UART0_CR, 0x301);
    }


    void putc(unsigned int c)
    {
        while(1)
        {
            if ((GET32(UART0_FR) & 0x20)==0)
                break;
        }

        PUT32(UART0_DR, c);

        if (c == '\n')
        {
            putc('\r');
        }
    }


    unsigned int getc()
    {
        while(1)
        {
            if ((GET32(UART0_FR) & 0x10)==0)
                break;
        }

        return GET32(UART0_DR);
    }
};




static bool console_initialized = false;
static RaspberryUart uart;

extern "C" int _libc_print(const char* string, size_t length)
{
    if (!console_initialized)
    {
        uart.Initialize();
        console_initialized = true;
    }

    for (size_t i = 0; i != length; ++i)
    {
        uart.putc(string[i]);
    }

    return length;
}



extern "C" int getchar()
{
    return EOF;
}



extern "C" void* malloc(size_t size)
{
    (void)size;
    assert(0 && "Out of memory");
    return NULL;
}



extern "C" void free(void* p)
{
    (void)p;
}



extern "C" void abort()
{
    for (;;)
    {
        //todo: cpu_disable_interrupts();
        asm("wfi");
    }
}