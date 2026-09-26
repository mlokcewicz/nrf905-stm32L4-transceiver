/*
 * startup.c
 *
 *  Created on: 10 wrz 2018
 *      Author: mlokc
 */

extern unsigned int _sidata;
extern unsigned int _sdata;
extern unsigned int _edata;
extern unsigned int _sbss;
extern unsigned int _ebss;

extern void __libc_init_array(void);
extern void __libc_fini_array(void);
extern int main(void);
extern void _exit(int) __attribute__((noreturn));

void _exit(int a)
{
    while(1);
}

__attribute__((weak, section(".text"))) void Reset_Handler(void)
{
    // copy .data to RAM
    unsigned int *start_data = &_sdata;
    unsigned int *start_init_data = &_sidata;

    while (start_data < &_edata)
        *start_data++ = *(start_init_data++);

    // fill .bss with zeros
    unsigned int *start_bss = &_sbss;

    while (start_bss < &_ebss)
        *start_bss++ = 0;

    // __libc_init_array
    __libc_init_array();

    // call main
    int code = main();

    // libc_fini_array
    __libc_fini_array();

    // exit to system
    _exit(code);

    while (1);
}
