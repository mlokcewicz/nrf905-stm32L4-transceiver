//------------------------------------------------------------------------------

/// @file main.c
/// @note Copyright (C) Michał Łokcewicz. All rights reserved.

//------------------------------------------------------------------------------

#include <stdio.h>

#include <stm32l4xx.h>

#include <systick.h>
#include <pwr.h>
#include <rcc.h>
#include <flash.h>
#include <wdg.h>
#include <gpio.h>
#include <usart.h>

//------------------------------------------------------------------------------

static struct usart_obj usart = {0};

//------------------------------------------------------------------------------

struct stack_frame
{
    uint32_t r0;
    uint32_t r1;
    uint32_t r2;
    uint32_t r3;
    uint32_t r12;
    uint32_t lr;
    uint32_t pc;
    uint32_t xpsr;
} __packed;

void hadfault_handler_c(struct stack_frame *frame)
{
    const uint32_t cfsr = SCB->CFSR;

    /* MemManage */
    volatile bool iaccviol __attribute__((unused)) = (cfsr & SCB_CFSR_IACCVIOL_Msk) != 0;
    volatile bool daccviol __attribute__((unused)) = (cfsr & SCB_CFSR_DACCVIOL_Msk) != 0;
    volatile bool munstkerr __attribute__((unused)) = (cfsr & SCB_CFSR_MUNSTKERR_Msk) != 0;
    volatile bool mstkerr __attribute__((unused)) = (cfsr & SCB_CFSR_MSTKERR_Msk) != 0;
    volatile bool mlsperr __attribute__((unused)) = (cfsr & SCB_CFSR_MLSPERR_Msk) != 0;
    volatile bool mmarvalid __attribute__((unused)) = (cfsr & SCB_CFSR_MMARVALID_Msk) != 0;

    /* BusFault */
    volatile bool ibuserr __attribute__((unused)) = (cfsr & SCB_CFSR_IBUSERR_Msk) != 0;
    volatile bool preciserr __attribute__((unused)) = (cfsr & SCB_CFSR_PRECISERR_Msk) != 0;
    volatile bool impreciserr __attribute__((unused)) = (cfsr & SCB_CFSR_IMPRECISERR_Msk) != 0;
    volatile bool unstkerr __attribute__((unused)) = (cfsr & SCB_CFSR_UNSTKERR_Msk) != 0;
    volatile bool stkerr __attribute__((unused)) = (cfsr & SCB_CFSR_STKERR_Msk) != 0;
    volatile bool lsperr __attribute__((unused)) = (cfsr & SCB_CFSR_LSPERR_Msk) != 0;
    volatile bool bfarvalid __attribute__((unused)) = (cfsr & SCB_CFSR_BFARVALID_Msk) != 0;

    /* UsageFault*/
    volatile bool undefinstr __attribute__((unused)) = (cfsr & SCB_CFSR_UNDEFINSTR_Msk) != 0;
    volatile bool invstate __attribute__((unused)) = (cfsr & SCB_CFSR_INVSTATE_Msk) != 0;
    volatile bool invpc __attribute__((unused)) = (cfsr & SCB_CFSR_INVPC_Msk) != 0;
    volatile bool nocp __attribute__((unused)) = (cfsr & SCB_CFSR_NOCP_Msk) != 0;
    volatile bool unaligned __attribute__((unused)) = (cfsr & SCB_CFSR_UNALIGNED_Msk) != 0;
    volatile bool divbyzero __attribute__((unused)) = (cfsr & SCB_CFSR_DIVBYZERO_Msk) != 0;

    volatile uint32_t mmfar __attribute__((unused)) = SCB->MMFAR;
    volatile uint32_t bfar __attribute__((unused)) = SCB->BFAR;
    volatile uint32_t afsr __attribute__((unused)) = SCB->AFSR;

    const uint32_t hfsr = SCB->HFSR;

    volatile bool vecttbl  __attribute__((unused)) = (hfsr & SCB_HFSR_VECTTBL_Msk) != 0;
    volatile bool forced   __attribute__((unused)) = (hfsr & SCB_HFSR_FORCED_Msk) != 0;
    volatile bool debugevt __attribute__((unused)) = (hfsr & SCB_HFSR_DEBUGEVT_Msk) != 0;

    while (1)
    {
        __NOP();
    }
}

__attribute__((naked)) void HardFault_Handler(void)
{
    asm volatile(
        "tst lr, 1 << 2\n\t"
        "ite eq\n\t"
        "mrseq r0, msp\n\t" // store sp to retrieve stack frame pointer as the first arg for C handler
        "mrsne r0, psp\n\t" // store sp to retrieve stack frame pointer as the first arg for C handler
        "ldr r4, =hadfault_handler_c\n\t" // store c handler address to r4
        "bx r4\n\t" // jump to C handler
    );
}

int __io_putchar(int ch)
{
    usart_send(&usart, (uint8_t *)&ch, 1);
    return 1;
}

static bool io_init(void)
{
    GPIO_CONFIG(A, 2, GPIO_MODE_AF, GPIO_OTYPE_PP, GPIO_OSPEED_LOW, GPIO_PUPD_NONE, 7);
    GPIO_CONFIG(A, 3, GPIO_MODE_AF, GPIO_OTYPE_PP, GPIO_OSPEED_LOW, GPIO_PUPD_NONE, 7);

    return true;
}

//------------------------------------------------------------------------------

int main()
{
    NVIC_SetPriorityGrouping(0x00000003U);
    
    /* SYSCLK = 4 MHz, source: MSI, AHB presc = /1 → HCLK = 4 MHz APB1/APB2 presc = /1 → PCLK1 = PCLK2 = 4 MHz */
    SystemInit();

    /* Set voltage scalling range */
    pwr_init(PWR_VOLTAGE_SCALING_RANGE1);

    /* Vcore range 1, HCLK <= 48 MHz */
    flash_init(FLASH_ACR_LATENCY_2WS);

    /* SYSCLK = HCLK = PCLK1 = PCLK2 = 40 MHz, HSI on, LS on */
    struct rcc_config cfg = 
    {
        .lsi_on = true,
        .hsi_on = true,
        .hse_on = false,
        .pll.main_pll_on = true,
        .pll.pllm = RCC_PLLM_DIV2,
        .pll.plln = 10,
        .pll.pllr = RCC_PLLR_DIV2,
        .pll.pllren = true,
        .pll.pllsrc = RCC_PLLCFGR_PLLSRC_HSI,
        .ahb_presc = RCC_CFGR_HPRE_DIV1,
        .apb1_presc = RCC_CFGR_PPRE1_DIV1,
        .apb2_presc = RCC_CFGR_PPRE2_DIV1,
        .sysclk_source = RCC_CFGR_SW_PLL,
    };

    rcc_init(&cfg);

    SystemCoreClockUpdate();

    systick_init();

    /* Maximum IWDG timeout: 32.768 s at nominal LSI = 32 kHz. */
    wdg_init(IWDG_PRESCALER_256, 0x0FFFu);

    struct usart_cfg usart_cfg =
    {
        .inst = USART_INSTANCE_2,
        .rx_tx = USART_TX_ENABLE | USART_RX_ENABLE,
        .data_size = USART_DATASIZE_8_BIT,
        .parity = USART_PARITY_DISABLED,
        .stop_bits = USART_STOP_BITS_1,
        .oversampling = USART_OVERSAMPLING_16,
        .baudrate = 115200,
        .io_init = io_init,
    };

    if (!usart_init(&usart, &usart_cfg))
        return 1;

    GPIO_CONFIG(A, 5, GPIO_MODE_OUTPUT, GPIO_OTYPE_PP, GPIO_OSPEED_LOW, GPIO_PUPD_NONE, 0);
    
    while (1)
    {
        GPIO_TOGGLE(A, 5);
        printf("Hello!\n");

        wdg_feed();

        systick_delay(500);
    }

    return 0;
}

//------------------------------------------------------------------------------

/*
CE = standby, (hi - on)
TXE - (hi - tx, low - rx)
PWR - (hi - on)
DR - (hi - tx finished or new data received)
SO - MISO
SI - MOSI
SCK - clock
CSN - CS


*/
