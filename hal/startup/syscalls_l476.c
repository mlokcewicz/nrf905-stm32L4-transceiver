/*
 * syscalls.c
 *
 *  Created on: 11 wrz 2018
 *      Author: mlokc
 */

#include <sys/types.h>
#include <stdint.h>
#include <errno.h>

caddr_t __attribute__((used)) _sbrk(int incr)
{
    extern char _end;               // Defined by the linker (.bss end, heap start).
    extern char _estack;            // Defined by the linker.
    extern char _Min_Stack_Size;    // Defined by the linker.

    static char* current_heap_end;
    char* current_block_address;

    if (current_heap_end == 0)
      current_heap_end = &_end;

    current_block_address = current_heap_end;

    // Need to align heap to word boundary, else will get
    // hard faults on Cortex-M0. So we assume that heap starts on
    // word boundary, hence make sure we always add a multiple of
    // 4 to it.
    incr = (incr + 3) & (~3); // align value to 4
    if (current_heap_end + incr > (&_estack - (uint32_t)&_Min_Stack_Size))
    {
      // Heap has overflowed
      errno = ENOMEM;
      return (caddr_t) - 1;
    }

    current_heap_end += incr;

    return (caddr_t) current_block_address;
}

void _close(void)
{

}

void _lseek(void)
{

}

void _read(void)
{

}

void _write(void)
{

}
