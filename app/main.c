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
#include <spi.h>

#include <nrf905.h>

//------------------------------------------------------------------------------

static struct usart_obj usart = {0};
static struct spi_obj spi2 = {0};
static struct spi_obj spi3 = {0};

static struct nrf905_obj nrf905_1_obj;
static struct nrf905_obj nrf905_2_obj;

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

static bool usart_io_init(void)
{
    GPIO_CONFIG(A, 2, GPIO_MODE_AF, GPIO_OTYPE_PP, GPIO_OSPEED_LOW, GPIO_PUPD_NONE, 7);
    GPIO_CONFIG(A, 3, GPIO_MODE_AF, GPIO_OTYPE_PP, GPIO_OSPEED_LOW, GPIO_PUPD_NONE, 7);

    return true;
}

static void spi2_cs_write(bool state)
{
    GPIO_SET(B, 12, state);
}

static bool spi2_io_init(void)
{
    GPIO_CONFIG(B, 13, GPIO_MODE_AF, GPIO_OTYPE_PP, GPIO_OSPEED_HI, GPIO_PUPD_NONE, 5); /* SPI2 SCK */
    GPIO_CONFIG(B, 14, GPIO_MODE_AF, GPIO_OTYPE_PP, GPIO_OSPEED_HI, GPIO_PUPD_NONE, 5); /* SPI2 MISO */
    GPIO_CONFIG(B, 15, GPIO_MODE_AF, GPIO_OTYPE_PP, GPIO_OSPEED_HI, GPIO_PUPD_NONE, 5); /* SPI2 MOSI */

    /* GPIOB clock is active; preload CS high before enabling its output. */
    spi2_cs_write(true);
    GPIO_CONFIG(B, 12, GPIO_MODE_OUTPUT, GPIO_OTYPE_PP, GPIO_OSPEED_LOW, GPIO_PUPD_NONE, 0);

    return true;
}

static bool nrf905_io_init_cb1(void)
{
    GPIO_CONFIG(C, 5, GPIO_MODE_OUTPUT, GPIO_OTYPE_PP, GPIO_OSPEED_LOW, GPIO_PUPD_NONE, 0);
    GPIO_CONFIG(C, 6, GPIO_MODE_OUTPUT, GPIO_OTYPE_PP, GPIO_OSPEED_LOW, GPIO_PUPD_NONE, 0);
    GPIO_CONFIG(C, 8, GPIO_MODE_OUTPUT, GPIO_OTYPE_PP, GPIO_OSPEED_LOW, GPIO_PUPD_NONE, 0);
    GPIO_CONFIG(C, 9, GPIO_MODE_INPUT, GPIO_OTYPE_OD, GPIO_OSPEED_LOW, GPIO_PUPD_NONE, 0);

    return true;
}

static bool nrf905_spi_transmit_cb1(uint8_t *data_tx, uint8_t *data_rx, uint32_t len)
{
    spi_transmit(&spi2, data_rx, data_tx, len);
    return true;
}

static bool nrf905_pin_set_cb1(enum nrf905_pin pin, bool state)
{
    switch (pin)
    {
        case NRF905_PIN_CS: spi_cs_set(&spi2, state); break;
        case NRF905_PIN_PWR: GPIO_SET(C, 6, state); break;
        case NRF905_PIN_CE: GPIO_SET(C, 8, state); break;
        case NRF905_PIN_TXE: GPIO_SET(C, 5, state); break;
        default: return false;
    }

    return true;
}

static bool nrf905_pin_get_cb1(enum nrf905_pin pin)
{
    switch (pin)
    {
        case NRF905_PIN_PWR: return GPIO_GET(C, 6);
        case NRF905_PIN_CE: return GPIO_GET(C, 8);
        case NRF905_PIN_TXE: return GPIO_GET(C, 5);
        case NRF905_PIN_DRDY: return GPIO_GET(C, 9);
        default: return false;
    }

    return true;
}

static bool nrf905_drdy_cb1(void)
{
    return true;
}

static void spi3_cs_write(bool state)
{
    GPIO_SET(C, 12, state);
}

static bool spi3_io_init(void)
{
    GPIO_CONFIG(C, 10, GPIO_MODE_AF, GPIO_OTYPE_PP, GPIO_OSPEED_HI, GPIO_PUPD_NONE, 6); /* SPI3 SCK */
    GPIO_CONFIG(C, 11, GPIO_MODE_AF, GPIO_OTYPE_PP, GPIO_OSPEED_HI, GPIO_PUPD_NONE, 6); /* SPI3 MISO */
    GPIO_CONFIG(C, 12, GPIO_MODE_AF, GPIO_OTYPE_PP, GPIO_OSPEED_HI, GPIO_PUPD_NONE, 6); /* SPI3 MOSI */

    /* GPIOB clock is active; preload CS high before enabling its output. */
    spi3_cs_write(true);
    GPIO_CONFIG(C, 13, GPIO_MODE_OUTPUT, GPIO_OTYPE_PP, GPIO_OSPEED_LOW, GPIO_PUPD_NONE, 0);

    return true;
}

// CE = PC0 - standby, (hi - on)
// TXE - PC1 - (hi - tx, low - rx)
// PWR - PC2 - (hi - on)
// DR PC3- (hi - tx finished or new data received)

static bool nrf905_io_init_cb2(void)
{
    GPIO_CONFIG(C, 0, GPIO_MODE_OUTPUT, GPIO_OTYPE_PP, GPIO_OSPEED_LOW, GPIO_PUPD_NONE, 0);
    GPIO_CONFIG(C, 1, GPIO_MODE_OUTPUT, GPIO_OTYPE_PP, GPIO_OSPEED_LOW, GPIO_PUPD_NONE, 0);
    GPIO_CONFIG(C, 2, GPIO_MODE_OUTPUT, GPIO_OTYPE_PP, GPIO_OSPEED_LOW, GPIO_PUPD_NONE, 0);
    GPIO_CONFIG(C, 3, GPIO_MODE_INPUT, GPIO_OTYPE_OD, GPIO_OSPEED_LOW, GPIO_PUPD_NONE, 0);

    return true;
}

static bool nrf905_spi_transmit_cb2(uint8_t *data_tx, uint8_t *data_rx, uint32_t len)
{
    spi_transmit(&spi3, data_rx, data_tx, len);
    return true;
}

static bool nrf905_pin_set_cb2(enum nrf905_pin pin, bool state)
{
    switch (pin)
    {
        case NRF905_PIN_CS: spi_cs_set(&spi3, state); break;
        case NRF905_PIN_PWR: GPIO_SET(C, 2, state); break;
        case NRF905_PIN_CE: GPIO_SET(C, 0, state); break;
        case NRF905_PIN_TXE: GPIO_SET(C, 1, state); break;
        default: return false;
    }

    return true;
}

static bool nrf905_pin_get_cb2(enum nrf905_pin pin)
{
    switch (pin)
    {
        case NRF905_PIN_PWR: return GPIO_GET(C, 2);
        case NRF905_PIN_CE: return GPIO_GET(C, 0);
        case NRF905_PIN_TXE: return GPIO_GET(C, 1);
        case NRF905_PIN_DRDY: return GPIO_GET(C, 3);
        default: return false;
    }

    return true;
}

static bool nrf905_drdy_cb2(void)
{
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

    /* LED */
    GPIO_CONFIG(A, 5, GPIO_MODE_OUTPUT, GPIO_OTYPE_PP, GPIO_OSPEED_LOW, GPIO_PUPD_NONE, 0);

    struct usart_cfg usart_cfg =
    {
        .inst = USART_INSTANCE_2,
        .rx_tx = USART_TX_ENABLE | USART_RX_ENABLE,
        .data_size = USART_DATASIZE_8_BIT,
        .parity = USART_PARITY_DISABLED,
        .stop_bits = USART_STOP_BITS_1,
        .oversampling = USART_OVERSAMPLING_16,
        .baudrate = 115200,
        .io_init = usart_io_init,
    };

    if (!usart_init(&usart, &usart_cfg))
        return 1;

    /* NRF 1 */    
    struct spi_cfg spi_cfg2 =
    {
        .inst = SPI_INSTANCE_2,
        .clock_presc = SPI_CLOCK_PRESC_16,
        .data_size = SPI_DATASIZE_8_BIT,
        .io_init = spi2_io_init,
        .cs_set = spi2_cs_write,
        .cpol = false,
        .cpha = false,
        .lsb_first = false,
    };

    if (!spi_init(&spi2, &spi_cfg2))
        return 1;

    struct nrf905_cfg nrf905_1_cfg = 
    {
        .io_init_cb = nrf905_io_init_cb1,
        .spi_transmit_cb = nrf905_spi_transmit_cb1,
        .pin_set_cb = nrf905_pin_set_cb1,
        .pin_get_cb = nrf905_pin_get_cb1,
        .drdy_cb = nrf905_drdy_cb1
    };

    if (!nrf905_init(&nrf905_1_obj, &nrf905_1_cfg))
        return 1;

    /* Allow the nRF905 to enter standby after PWR_UP rises. */
    systick_delay(3);

    /* R_CONFIG(0): the command returns status; the dummy byte clocks out register 0. */
    uint8_t data_tx[] = {0b00010000, 0x00};
    uint8_t data_rx[sizeof(data_tx)] = {0};

    /* spi_transmit keeps CS low for the entire command and data transfer. */
    spi_transmit(&spi2, data_rx, data_tx, sizeof(data_tx));

    printf("nRF905 1 status: 0x%02X, CONFIG[0]: 0x%02X\n", (unsigned int)data_rx[0], (unsigned int)data_rx[1]);

    /* NRF 2 */
    struct spi_cfg spi_cfg3 =
    {
        .inst = SPI_INSTANCE_3,
        .clock_presc = SPI_CLOCK_PRESC_16,
        .data_size = SPI_DATASIZE_8_BIT,
        .io_init = spi3_io_init,
        .cs_set = spi3_cs_write,
        .cpol = false,
        .cpha = false,
        .lsb_first = false,
    };

    if (!spi_init(&spi3, &spi_cfg3))
        return 1;

    struct nrf905_cfg nrf905_2_cfg = 
    {
        .io_init_cb = nrf905_io_init_cb2,
        .spi_transmit_cb = nrf905_spi_transmit_cb2,
        .pin_set_cb = nrf905_pin_set_cb2,
        .pin_get_cb = nrf905_pin_get_cb2,
        .drdy_cb = nrf905_drdy_cb2
    };

    if (!nrf905_init(&nrf905_2_obj, &nrf905_2_cfg))
        return 1;

    /* Allow the nRF905 to enter standby after PWR_UP rises. */
    systick_delay(3);

    /* R_CONFIG(0): the command returns status; the dummy byte clocks out register 0. */
    uint8_t data_tx3[] = {0b00010000, 0x00};
    uint8_t data_rx3[sizeof(data_tx3)] = {0};

    /* spi_transmit keeps CS low for the entire command and data transfer. */
    spi_transmit(&spi3, data_rx3, data_tx3, sizeof(data_tx3));

    printf("nRF905 2 status: 0x%02X, CONFIG[0]: 0x%02X\n", (unsigned int)data_rx3[0], (unsigned int)data_rx3[1]);

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
nRF 1:
CE = PC8 - standby, (hi - on)
TXE - PC5 - (hi - tx, low - rx)
PWR - PC6 - (hi - on)
DR - PC9 (hi - tx finished or new data received)
SO - MISO
SI - MOSI
SCK - clock
CSN - CS

nRF 2:

CE = PC0 - standby, (hi - on)
TXE - PC1 - (hi - tx, low - rx)
PWR - PC2 - (hi - on)
DR PC3- (hi - tx finished or new data received)



SCK PC10
MISO PC11
MOSI PC12
CS PC13

*/
