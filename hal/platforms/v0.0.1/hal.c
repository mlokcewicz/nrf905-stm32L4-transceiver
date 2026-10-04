//------------------------------------------------------------------------------

/// @file hal.c
/// @note Copyright (C) Michał Łokcewicz. All rights reserved.

//------------------------------------------------------------------------------

#include "hal.h"

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <stm32l4xx.h>

#include <systick.h>
#include <pwr.h>
#include <rcc.h>
#include <flash.h>
#include <wdg.h>
#include <dma.h>
#include <gpio.h>
#include <usart.h>
#include <spi.h>

#include <nrf905.h>

//------------------------------------------------------------------------------
/* Board pin assignments */
#define HAL_UART_TX_PORT                   A
#define HAL_UART_TX_PIN                    2
#define HAL_UART_RX_PORT                   A
#define HAL_UART_RX_PIN                    3
#define HAL_NRF1_SCK_PORT                  B
#define HAL_NRF1_SCK_PIN                   13
#define HAL_NRF1_MISO_PORT                 B
#define HAL_NRF1_MISO_PIN                  14
#define HAL_NRF1_MOSI_PORT                 B
#define HAL_NRF1_MOSI_PIN                  15
#define HAL_NRF1_CS_PORT                   B
#define HAL_NRF1_CS_PIN                    12
#define HAL_NRF2_SCK_PORT                  C
#define HAL_NRF2_SCK_PIN                   10
#define HAL_NRF2_MISO_PORT                 C
#define HAL_NRF2_MISO_PIN                  11
#define HAL_NRF2_MOSI_PORT                 C
#define HAL_NRF2_MOSI_PIN                  12
#define HAL_NRF2_CS_PORT                   A
#define HAL_NRF2_CS_PIN                    15
#define HAL_NRF1_TXE_PORT                  C
#define HAL_NRF1_TXE_PIN                   5
#define HAL_NRF1_PWR_PORT                  C
#define HAL_NRF1_PWR_PIN                   6
#define HAL_NRF1_CE_PORT                   C
#define HAL_NRF1_CE_PIN                    8
#define HAL_NRF1_DR_PORT                   C
#define HAL_NRF1_DR_PIN                    9
#define HAL_NRF2_CE_PORT                   C
#define HAL_NRF2_CE_PIN                    0
#define HAL_NRF2_TXE_PORT                  C
#define HAL_NRF2_TXE_PIN                   1
#define HAL_NRF2_PWR_PORT                  C
#define HAL_NRF2_PWR_PIN                   2
#define HAL_NRF2_DR_PORT                   B
#define HAL_NRF2_DR_PIN                    4
#define HAL_LED_PORT                       A
#define HAL_LED_PIN                        5

/* GPIO alternate functions */
#define HAL_UART_AF                        7
#define HAL_NRF1_AF                        5
#define HAL_NRF2_AF                        6
#define HAL_GPIO_AF_UNUSED                 0

/* Clock tree and watchdog */
#define HAL_IRQ_PRIORITY_GROUPING          0x00000003U
#define HAL_VOLTAGE_SCALING                PWR_VOLTAGE_SCALING_RANGE1
#define HAL_FLASH_LATENCY                  FLASH_ACR_LATENCY_2WS
#define HAL_WDG_PRESCALER                  IWDG_PRESCALER_256
#define HAL_WDG_RELOAD                     0x0FFFu
#define HAL_LSI_ON                         true
#define HAL_HSI_ON                         true
#define HAL_HSE_ON                         false
#define HAL_PLL_ON                         true
#define HAL_PLL_M                          RCC_PLLM_DIV2
#define HAL_PLL_N                          10
#define HAL_PLL_R                          RCC_PLLR_DIV2
#define HAL_PLL_R_ENABLE                   true
#define HAL_PLL_SOURCE                     RCC_PLLCFGR_PLLSRC_HSI
#define HAL_AHB_PRESCALER                  RCC_CFGR_HPRE_DIV1
#define HAL_APB1_PRESCALER                 RCC_CFGR_PPRE1_DIV1
#define HAL_APB2_PRESCALER                 RCC_CFGR_PPRE2_DIV1
#define HAL_SYSCLK_SOURCE                  RCC_CFGR_SW_PLL

/* USART configuration */
#define HAL_UART_INSTANCE                  USART_INSTANCE_2
#define HAL_UART_RX_TX                     (USART_TX_ENABLE | USART_RX_ENABLE)
#define HAL_UART_DATA_SIZE                 USART_DATASIZE_8_BIT
#define HAL_UART_PARITY                    USART_PARITY_DISABLED
#define HAL_UART_STOP_BITS                 USART_STOP_BITS_1
#define HAL_UART_OVERSAMPLING              USART_OVERSAMPLING_16
#define HAL_UART_BAUDRATE                  115200

/* Radio SPI configuration */
#define HAL_NRF1_SPI_INSTANCE              SPI_INSTANCE_2
#define HAL_NRF2_SPI_INSTANCE              SPI_INSTANCE_3
#define HAL_SPI_PRESCALER                  SPI_CLOCK_PRESC_16
#define HAL_SPI_DATA_SIZE                  SPI_DATASIZE_8_BIT
#define HAL_SPI_CPOL                       false
#define HAL_SPI_CPHA                       false
#define HAL_SPI_LSB_FIRST                  false

/* SPI DMA mapping */
#define HAL_SPI2_DMA_INSTANCE              DMA_INSTANCE_1
#define HAL_SPI2_DMA_RX_CHANNEL            DMA_CHANNEL_4
#define HAL_SPI2_DMA_TX_CHANNEL            DMA_CHANNEL_5
#define HAL_SPI2_DMA_REQUEST               DMA_REQUEST_1
#define HAL_SPI3_DMA_INSTANCE              DMA_INSTANCE_2
#define HAL_SPI3_DMA_RX_CHANNEL            DMA_CHANNEL_1
#define HAL_SPI3_DMA_TX_CHANNEL            DMA_CHANNEL_2
#define HAL_SPI3_DMA_REQUEST               DMA_REQUEST_3

/* Shared radio configuration */
#define HAL_RADIO_CHANNEL                  108
#define HAL_RADIO_BAND                     NRF905_PLL_FREQ_433_MHZ
#define HAL_RADIO_TX_POWER                 NRF905_PA_PWR_10_DBM
#define HAL_RADIO_RX_REDUCED_POWER         false
#define HAL_RADIO_AUTO_RETRANSMIT          false
#define HAL_RADIO_RX_ADDRESS_WIDTH         NRF905_ADDR_WIDTH_4_BYTE
#define HAL_RADIO_TX_ADDRESS_WIDTH         NRF905_ADDR_WIDTH_4_BYTE
#define HAL_RADIO_RX_PAYLOAD_SIZE          32
#define HAL_RADIO_TX_PAYLOAD_SIZE          32
#define HAL_RADIO_RX_ADDRESS               0xDDCCBBAA
#define HAL_RADIO_OUTPUT_CLOCK_FREQ        NRF905_UP_CLK_FREQ_4_MHZ
#define HAL_RADIO_OUTPUT_CLOCK_ENABLE      false
#define HAL_RADIO_CRYSTAL_FREQ             NRF905_XOF_16_MHZ
#define HAL_RADIO_CRC_ENABLE               true
#define HAL_RADIO_CRC_16_BIT               true

/* Radio diagnostics */
#define HAL_RADIO_STARTUP_DELAY_MS         3
#define HAL_RADIO_TEST_DELAY_MS            1000
#define HAL_RADIO_READ_CONFIG_0            0b00010000
#define HAL_SPI_DUMMY_BYTE                 0x00
#define HAL_RADIO_TEST_PAYLOAD             "nRF905 RADIO TRX TEST MSG!"
#define HAL_RADIO_TEST_TX_ADDRESS          HAL_RADIO_RX_ADDRESS

//------------------------------------------------------------------------------
/* Hardware definitions */

// tu

//------------------------------------------------------------------------------
/* Static objects */

static struct usart_obj usart2_obj = {0};
static struct spi_obj spi2_obj = {0};
static struct spi_obj spi3_obj = {0};

static struct nrf905_obj nrf905_1_obj;
static struct nrf905_obj nrf905_2_obj;

static volatile bool spi2_dma_rx_done;
static volatile bool spi2_dma_tx_done;
static volatile bool spi3_dma_rx_done;
static volatile bool spi3_dma_tx_done;

//------------------------------------------------------------------------------
/* Hardfault handling */

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

//------------------------------------------------------------------------------
/* printf() redirerct */

int __io_putchar(int ch)
{
    usart_send(&usart2_obj, (uint8_t *)&ch, 1);
    return 1;
}

//------------------------------------------------------------------------------
/* Peripheral drivers configuraition */

static bool usart_io_init(void)
{
    GPIO_CONFIG(HAL_UART_TX_PORT, HAL_UART_TX_PIN, GPIO_MODE_AF, GPIO_OTYPE_PP, GPIO_OSPEED_LOW, GPIO_PUPD_NONE, HAL_UART_AF);
    GPIO_CONFIG(HAL_UART_RX_PORT, HAL_UART_RX_PIN, GPIO_MODE_AF, GPIO_OTYPE_PP, GPIO_OSPEED_LOW, GPIO_PUPD_NONE, HAL_UART_AF);

    return true;
}

static void spi2_dma_rx_tc(void)
{
    spi_transmit_finish(&spi2_obj);
    spi2_dma_rx_done = true;
}

static void spi2_dma_tx_tc(void)
{
    spi2_dma_tx_done = true;
}

static void spi3_dma_rx_tc(void)
{
    spi_transmit_finish(&spi3_obj);
    spi3_dma_rx_done = true;
}

static void spi3_dma_tx_tc(void)
{
    spi3_dma_tx_done = true;
}

static void spi2_dma_config(uintptr_t periph_address, uint8_t *data_rx, uint8_t *data_tx, uint32_t len)
{
    static uint8_t dummy_rx;
    static uint8_t dummy_tx = HAL_SPI_DUMMY_BYTE;

    struct dma_channel_cfg rx_cfg =
    {
        .inst = HAL_SPI2_DMA_INSTANCE,
        .channel = HAL_SPI2_DMA_RX_CHANNEL,
        .tc = spi2_dma_rx_tc,
        .peripheral_address = periph_address,
        .memory_address = data_rx ? (uintptr_t)data_rx : (uintptr_t)&dummy_rx,
        .count = len,
        .peripheral_size = DMA_DATA_SIZE_8_BIT,
        .memory_size = DMA_DATA_SIZE_8_BIT,
        .direction = DMA_PERIPHERAL_TO_MEMORY,
        .priority = DMA_PRIORITY_LOW,
        .memory_increment = data_rx != NULL,
        .peripheral_increment = false,
        .circular = false,
        .memory_to_memory = false,
    };

    struct dma_channel_cfg tx_cfg =
    {
        .inst = HAL_SPI2_DMA_INSTANCE,
        .channel = HAL_SPI2_DMA_TX_CHANNEL,
        .tc = spi2_dma_tx_tc,
        .peripheral_address = periph_address,
        .memory_address = data_tx ? (uintptr_t)data_tx : (uintptr_t)&dummy_tx,
        .count = len,
        .peripheral_size = DMA_DATA_SIZE_8_BIT,
        .memory_size = DMA_DATA_SIZE_8_BIT,
        .direction = DMA_MEMORY_TO_PERIPHERAL,
        .priority = DMA_PRIORITY_LOW,
        .memory_increment = data_tx != NULL,
        .peripheral_increment = false,
        .circular = false,
        .memory_to_memory = false,
    };

    dma_channel_config(&rx_cfg);
    dma_channel_config(&tx_cfg);
}

static void spi3_dma_config_internal(uintptr_t periph_address, uint8_t *data_rx, uint8_t *data_tx, uint32_t len, bool circular_rx, bool circular_tx, void (*rx_tc)(void), void (*tx_tc)(void))
{
    static uint8_t dummy_rx;
    static uint8_t dummy_tx = HAL_SPI_DUMMY_BYTE;

    struct dma_channel_cfg rx_cfg =
    {
        .inst = HAL_SPI3_DMA_INSTANCE,
        .channel = HAL_SPI3_DMA_RX_CHANNEL,
        .tc = rx_tc,
        .peripheral_address = periph_address,
        .memory_address = data_rx ? (uintptr_t)data_rx : (uintptr_t)&dummy_rx,
        .count = len,
        .peripheral_size = DMA_DATA_SIZE_8_BIT,
        .memory_size = DMA_DATA_SIZE_8_BIT,
        .direction = DMA_PERIPHERAL_TO_MEMORY,
        .priority = DMA_PRIORITY_LOW,
        .memory_increment = data_rx != NULL,
        .peripheral_increment = false,
        .circular = circular_rx,
        .memory_to_memory = false,
    };

    struct dma_channel_cfg tx_cfg =
    {
        .inst = HAL_SPI3_DMA_INSTANCE,
        .channel = HAL_SPI3_DMA_TX_CHANNEL,
        .tc = tx_tc,
        .peripheral_address = periph_address,
        .memory_address = data_tx ? (uintptr_t)data_tx : (uintptr_t)&dummy_tx,
        .count = len,
        .peripheral_size = DMA_DATA_SIZE_8_BIT,
        .memory_size = DMA_DATA_SIZE_8_BIT,
        .direction = DMA_MEMORY_TO_PERIPHERAL,
        .priority = DMA_PRIORITY_LOW,
        .memory_increment = data_tx != NULL,
        .peripheral_increment = false,
        .circular = circular_tx,
        .memory_to_memory = false,
    };

    dma_channel_config(&rx_cfg);
    dma_channel_config(&tx_cfg);
}

static void spi3_dma_config(uintptr_t periph_address, uint8_t *data_rx, uint8_t *data_tx, uint32_t len)
{
    spi3_dma_config_internal(periph_address, data_rx, data_tx, len, false, false, spi3_dma_rx_tc, spi3_dma_tx_tc);
}

static void spi2_cs_write(bool state)
{
    GPIO_SET(HAL_NRF1_CS_PORT, HAL_NRF1_CS_PIN, state);
}

static bool spi2_io_init(void)
{
    GPIO_CONFIG(HAL_NRF1_SCK_PORT, HAL_NRF1_SCK_PIN, GPIO_MODE_AF, GPIO_OTYPE_PP, GPIO_OSPEED_HI, GPIO_PUPD_NONE, HAL_NRF1_AF); /* SPI2 SCK */
    GPIO_CONFIG(HAL_NRF1_MISO_PORT, HAL_NRF1_MISO_PIN, GPIO_MODE_AF, GPIO_OTYPE_PP, GPIO_OSPEED_HI, GPIO_PUPD_NONE, HAL_NRF1_AF); /* SPI2 MISO */
    GPIO_CONFIG(HAL_NRF1_MOSI_PORT, HAL_NRF1_MOSI_PIN, GPIO_MODE_AF, GPIO_OTYPE_PP, GPIO_OSPEED_HI, GPIO_PUPD_NONE, HAL_NRF1_AF); /* SPI2 MOSI */
    GPIO_CONFIG(HAL_NRF1_CS_PORT, HAL_NRF1_CS_PIN, GPIO_MODE_AF, GPIO_OTYPE_PP, GPIO_OSPEED_LOW, GPIO_PUPD_PU, HAL_NRF1_AF);

    return true;
}

static void spi3_cs_write(bool state)
{
    GPIO_SET(HAL_NRF2_CS_PORT, HAL_NRF2_CS_PIN, state);
}

static bool spi3_io_init(void)
{
    GPIO_CONFIG(HAL_NRF2_SCK_PORT, HAL_NRF2_SCK_PIN, GPIO_MODE_AF, GPIO_OTYPE_PP, GPIO_OSPEED_HI, GPIO_PUPD_NONE, HAL_NRF2_AF); /* SPI3 SCK */
    GPIO_CONFIG(HAL_NRF2_MISO_PORT, HAL_NRF2_MISO_PIN, GPIO_MODE_AF, GPIO_OTYPE_PP, GPIO_OSPEED_HI, GPIO_PUPD_NONE, HAL_NRF2_AF); /* SPI3 MISO */
    GPIO_CONFIG(HAL_NRF2_MOSI_PORT, HAL_NRF2_MOSI_PIN, GPIO_MODE_AF, GPIO_OTYPE_PP, GPIO_OSPEED_HI, GPIO_PUPD_NONE, HAL_NRF2_AF); /* SPI3 MOSI */
    GPIO_CONFIG(HAL_NRF2_CS_PORT, HAL_NRF2_CS_PIN, GPIO_MODE_AF, GPIO_OTYPE_PP, GPIO_OSPEED_LOW, GPIO_PUPD_PU, HAL_NRF2_AF);

    return true;
}

//------------------------------------------------------------------------------
/* External drivers initialization */

static bool nrf905_io_init_cb1(void)
{
    GPIO_CONFIG(HAL_NRF1_TXE_PORT, HAL_NRF1_TXE_PIN, GPIO_MODE_OUTPUT, GPIO_OTYPE_PP, GPIO_OSPEED_LOW, GPIO_PUPD_NONE, HAL_GPIO_AF_UNUSED);
    GPIO_CONFIG(HAL_NRF1_PWR_PORT, HAL_NRF1_PWR_PIN, GPIO_MODE_OUTPUT, GPIO_OTYPE_PP, GPIO_OSPEED_LOW, GPIO_PUPD_NONE, HAL_GPIO_AF_UNUSED);
    GPIO_CONFIG(HAL_NRF1_CE_PORT, HAL_NRF1_CE_PIN, GPIO_MODE_OUTPUT, GPIO_OTYPE_PP, GPIO_OSPEED_LOW, GPIO_PUPD_NONE, HAL_GPIO_AF_UNUSED);
    GPIO_CONFIG(HAL_NRF1_DR_PORT, HAL_NRF1_DR_PIN, GPIO_MODE_INPUT, GPIO_OTYPE_OD, GPIO_OSPEED_LOW, GPIO_PUPD_NONE, HAL_GPIO_AF_UNUSED);

    return true;
}

static bool nrf905_spi_transmit_cb1(uint8_t *data_tx, uint8_t *data_rx, uint32_t len)
{
    spi2_dma_rx_done = false;
    spi2_dma_tx_done = false;

    spi_transmit(&spi2_obj, data_rx, data_tx, len);

    while (!spi2_dma_rx_done || !spi2_dma_tx_done) {}

    return true;
}

static bool nrf905_pin_set_cb1(enum nrf905_pin pin, bool state)
{
    switch (pin)
    {
        case NRF905_PIN_CS: spi_cs_set(&spi2_obj, state); break;
        case NRF905_PIN_PWR: GPIO_SET(HAL_NRF1_PWR_PORT, HAL_NRF1_PWR_PIN, state); break;
        case NRF905_PIN_CE: GPIO_SET(HAL_NRF1_CE_PORT, HAL_NRF1_CE_PIN, state); break;
        case NRF905_PIN_TXE: GPIO_SET(HAL_NRF1_TXE_PORT, HAL_NRF1_TXE_PIN, state); break;
        default: return false;
    }

    return true;
}

static bool nrf905_pin_get_cb1(enum nrf905_pin pin)
{
    switch (pin)
    {
        case NRF905_PIN_PWR: return GPIO_GET(HAL_NRF1_PWR_PORT, HAL_NRF1_PWR_PIN);
        case NRF905_PIN_CE: return GPIO_GET(HAL_NRF1_CE_PORT, HAL_NRF1_CE_PIN);
        case NRF905_PIN_TXE: return GPIO_GET(HAL_NRF1_TXE_PORT, HAL_NRF1_TXE_PIN);
        case NRF905_PIN_DRDY: return GPIO_GET(HAL_NRF1_DR_PORT, HAL_NRF1_DR_PIN);
        default: return false;
    }

    return true;
}

static bool nrf905_io_init_cb2(void)
{
    GPIO_CONFIG(HAL_NRF2_CE_PORT, HAL_NRF2_CE_PIN, GPIO_MODE_OUTPUT, GPIO_OTYPE_PP, GPIO_OSPEED_LOW, GPIO_PUPD_NONE, HAL_GPIO_AF_UNUSED);
    GPIO_CONFIG(HAL_NRF2_TXE_PORT, HAL_NRF2_TXE_PIN, GPIO_MODE_OUTPUT, GPIO_OTYPE_PP, GPIO_OSPEED_LOW, GPIO_PUPD_NONE, HAL_GPIO_AF_UNUSED);
    GPIO_CONFIG(HAL_NRF2_PWR_PORT, HAL_NRF2_PWR_PIN, GPIO_MODE_OUTPUT, GPIO_OTYPE_PP, GPIO_OSPEED_LOW, GPIO_PUPD_NONE, HAL_GPIO_AF_UNUSED);
    GPIO_CONFIG(HAL_NRF2_DR_PORT, HAL_NRF2_DR_PIN, GPIO_MODE_AF, GPIO_OTYPE_OD, GPIO_OSPEED_LOW, GPIO_PUPD_NONE, 2);

    return true;
}

static bool nrf905_spi_transmit_cb2(uint8_t *data_tx, uint8_t *data_rx, uint32_t len)
{
    spi3_dma_rx_done = false;
    spi3_dma_tx_done = false;

    spi_transmit(&spi3_obj, data_rx, data_tx, len);

    while (!spi3_dma_rx_done || !spi3_dma_tx_done) {}

    return true;
}

static bool nrf905_pin_set_cb2(enum nrf905_pin pin, bool state)
{
    switch (pin)
    {
        case NRF905_PIN_CS: spi_cs_set(&spi3_obj, state); break;
        case NRF905_PIN_PWR: GPIO_SET(HAL_NRF2_PWR_PORT, HAL_NRF2_PWR_PIN, state); break;
        case NRF905_PIN_CE: GPIO_SET(HAL_NRF2_CE_PORT, HAL_NRF2_CE_PIN, state); break;
        case NRF905_PIN_TXE: GPIO_SET(HAL_NRF2_TXE_PORT, HAL_NRF2_TXE_PIN, state); break;
        default: return false;
    }

    return true;
}

static bool nrf905_pin_get_cb2(enum nrf905_pin pin)
{
    switch (pin)
    {
        case NRF905_PIN_PWR: return GPIO_GET(HAL_NRF2_PWR_PORT, HAL_NRF2_PWR_PIN);
        case NRF905_PIN_CE: return GPIO_GET(HAL_NRF2_CE_PORT, HAL_NRF2_CE_PIN);
        case NRF905_PIN_TXE: return GPIO_GET(HAL_NRF2_TXE_PORT, HAL_NRF2_TXE_PIN);
        case NRF905_PIN_DRDY: return GPIO_GET(HAL_NRF2_DR_PORT, HAL_NRF2_DR_PIN);
        default: return false;
    }

    return true;
}

/*
nRF 1:
CE  - PC8 - standby, (hi - on)
TXE - PC5 - (hi - tx, low - rx)
PWR - PC6 - (hi - on)
DR  - PC9 (hi - tx finished or new data received)
SO  - MISO
SI  - MOSI
SCK - clock
CSN - CS

nRF 2:
CE  - PC0 - standby, (hi - on)
TXE - PC1 - (hi - tx, low - rx)
PWR - PC2 - (hi - on)
DR  - PC3 - (hi - tx finished or new data received)
SCK - PC10
SO - PC11
SI - PC12
CS - PC13
*/

//------------------------------------------------------------------------------

bool hal_init(void)
{
    NVIC_SetPriorityGrouping(HAL_IRQ_PRIORITY_GROUPING);
    
    SystemInit();

    /* Set voltage scalling range */
    pwr_init(HAL_VOLTAGE_SCALING);

    /* Vcore range 1, HCLK <= 48 MHz */
    flash_init(HAL_FLASH_LATENCY);

    /* SYSCLK = HCLK = PCLK1 = PCLK2 = 40 MHz, HSI on, LS on */
    struct rcc_config cfg = 
    {
        .lsi_on = HAL_LSI_ON,
        .hsi_on = HAL_HSI_ON,
        .hse_on = HAL_HSE_ON,
        .pll.main_pll_on = HAL_PLL_ON,
        .pll.pllm = HAL_PLL_M,
        .pll.plln = HAL_PLL_N,
        .pll.pllr = HAL_PLL_R,
        .pll.pllren = HAL_PLL_R_ENABLE,
        .pll.pllsrc = HAL_PLL_SOURCE,
        .ahb_presc = HAL_AHB_PRESCALER,
        .apb1_presc = HAL_APB1_PRESCALER,
        .apb2_presc = HAL_APB2_PRESCALER,
        .sysclk_source = HAL_SYSCLK_SOURCE,
    };

    rcc_init(&cfg);

    SystemCoreClockUpdate();

    systick_init();

    /* Maximum IWDG timeout: 32.768 s at nominal LSI = 32 kHz. */
    wdg_init(HAL_WDG_PRESCALER, HAL_WDG_RELOAD);

    /* LED */
    GPIO_CONFIG(HAL_LED_PORT, HAL_LED_PIN, GPIO_MODE_OUTPUT, GPIO_OTYPE_PP, GPIO_OSPEED_LOW, GPIO_PUPD_NONE, HAL_GPIO_AF_UNUSED);

    struct usart_cfg usart_cfg =
    {
        .inst = HAL_UART_INSTANCE,
        .rx_tx = HAL_UART_RX_TX,
        .data_size = HAL_UART_DATA_SIZE,
        .parity = HAL_UART_PARITY,
        .stop_bits = HAL_UART_STOP_BITS,
        .oversampling = HAL_UART_OVERSAMPLING,
        .baudrate = HAL_UART_BAUDRATE,
        .io_init = usart_io_init,
    };

    if (!usart_init(&usart2_obj, &usart_cfg))
        return false;

    /* DMA for SPI2 and SPI3 */
    dma_init(HAL_SPI2_DMA_INSTANCE);
    dma_init(HAL_SPI3_DMA_INSTANCE);

    if (!dma_channel_link(HAL_SPI2_DMA_INSTANCE, HAL_SPI2_DMA_RX_CHANNEL, HAL_SPI2_DMA_REQUEST))
        return false;

    if (!dma_channel_link(HAL_SPI2_DMA_INSTANCE, HAL_SPI2_DMA_TX_CHANNEL, HAL_SPI2_DMA_REQUEST))
        return false;

    if (!dma_channel_link(HAL_SPI3_DMA_INSTANCE, HAL_SPI3_DMA_RX_CHANNEL, HAL_SPI3_DMA_REQUEST))
        return false;

    if (!dma_channel_link(HAL_SPI3_DMA_INSTANCE, HAL_SPI3_DMA_TX_CHANNEL, HAL_SPI3_DMA_REQUEST))
        return false;

    /* NRF 1 */    
    struct spi_cfg spi_cfg2 =
    {
        .inst = HAL_NRF1_SPI_INSTANCE,
        .clock_presc = HAL_SPI_PRESCALER,
        .data_size = HAL_SPI_DATA_SIZE,
        .cpol = HAL_SPI_CPOL,
        .cpha = HAL_SPI_CPHA,
        .lsb_first = HAL_SPI_LSB_FIRST,
        .hw_cs = true,
        .io_init = spi2_io_init,
        .dma_config = spi2_dma_config,
        .cs_set = spi2_cs_write,
    };

    if (!spi_init(&spi2_obj, &spi_cfg2))
        return false;

    struct nrf905_cfg nrf905_1_cfg = 
    {
        .io_init_cb = nrf905_io_init_cb1,
        .spi_transmit_cb = nrf905_spi_transmit_cb1,
        .pin_set_cb = nrf905_pin_set_cb1,
        .pin_get_cb = nrf905_pin_get_cb1,

        .ch_no = HAL_RADIO_CHANNEL,
        .pll_freq = HAL_RADIO_BAND,
        .pa_pwr = HAL_RADIO_TX_POWER,
        .rx_red_pwr = HAL_RADIO_RX_REDUCED_POWER,
        .auto_retran = HAL_RADIO_AUTO_RETRANSMIT,
        .rx_afw = HAL_RADIO_RX_ADDRESS_WIDTH,
        .tx_afw = HAL_RADIO_TX_ADDRESS_WIDTH,
        .rx_pw = HAL_RADIO_RX_PAYLOAD_SIZE,
        .tx_pw = HAL_RADIO_TX_PAYLOAD_SIZE,
        .rx_addr = HAL_RADIO_RX_ADDRESS,
        .up_clk_freq = HAL_RADIO_OUTPUT_CLOCK_FREQ,
        .up_clk_en = HAL_RADIO_OUTPUT_CLOCK_ENABLE,
        .xof = HAL_RADIO_CRYSTAL_FREQ,
        .crc_en = HAL_RADIO_CRC_ENABLE,
        .crc_16 = HAL_RADIO_CRC_16_BIT,
    };

    if (!nrf905_init(&nrf905_1_obj, &nrf905_1_cfg))
        return false;

    /* Allow the nRF905 to enter standby after PWR_UP rises. */
    systick_delay(HAL_RADIO_STARTUP_DELAY_MS);

    /* R_CONFIG(0): the command returns status; the dummy byte clocks out register 0. */
    uint8_t data_tx[] = {HAL_RADIO_READ_CONFIG_0, HAL_SPI_DUMMY_BYTE};
    uint8_t data_rx[sizeof(data_tx)] = {0};

    /* Keep CS low until both DMA channels complete. */
    spi2_dma_rx_done = false;
    spi2_dma_tx_done = false;
    spi_transmit(&spi2_obj, data_rx, data_tx, sizeof(data_tx));
    while (!spi2_dma_rx_done || !spi2_dma_tx_done) {}

    printf("nRF905 1 status: 0x%02X, CONFIG[0]: 0x%02X\n", (unsigned int)data_rx[0], (unsigned int)data_rx[1]);

    /* NRF 2 */
    struct spi_cfg spi_cfg3 =
    {
        .inst = HAL_NRF2_SPI_INSTANCE,
        .clock_presc = HAL_SPI_PRESCALER,
        .data_size = HAL_SPI_DATA_SIZE,
        .cpol = HAL_SPI_CPOL,
        .cpha = HAL_SPI_CPHA,
        .lsb_first = HAL_SPI_LSB_FIRST,
        .hw_cs = true,
        .io_init = spi3_io_init,
        .dma_config = spi3_dma_config,
        .cs_set = spi3_cs_write,
    };

    if (!spi_init(&spi3_obj, &spi_cfg3))
        return false;

    struct nrf905_cfg nrf905_2_cfg = 
    {
        .io_init_cb = nrf905_io_init_cb2,
        .spi_transmit_cb = nrf905_spi_transmit_cb2,
        .pin_set_cb = nrf905_pin_set_cb2,
        .pin_get_cb = nrf905_pin_get_cb2,

        .ch_no = HAL_RADIO_CHANNEL,
        .pll_freq = HAL_RADIO_BAND,
        .pa_pwr = HAL_RADIO_TX_POWER,
        .rx_red_pwr = HAL_RADIO_RX_REDUCED_POWER,
        .auto_retran = HAL_RADIO_AUTO_RETRANSMIT,
        .rx_afw = HAL_RADIO_RX_ADDRESS_WIDTH,
        .tx_afw = HAL_RADIO_TX_ADDRESS_WIDTH,
        .rx_pw = HAL_RADIO_RX_PAYLOAD_SIZE,
        .tx_pw = HAL_RADIO_TX_PAYLOAD_SIZE,
        .rx_addr = HAL_RADIO_RX_ADDRESS,
        .up_clk_freq = HAL_RADIO_OUTPUT_CLOCK_FREQ,
        .up_clk_en = HAL_RADIO_OUTPUT_CLOCK_ENABLE,
        .xof = HAL_RADIO_CRYSTAL_FREQ,
        .crc_en = HAL_RADIO_CRC_ENABLE,
        .crc_16 = HAL_RADIO_CRC_16_BIT,
    };

    if (!nrf905_init(&nrf905_2_obj, &nrf905_2_cfg))
        return false;

    /* Allow the nRF905 to enter standby after PWR_UP rises. */
    systick_delay(HAL_RADIO_STARTUP_DELAY_MS);

    /* R_CONFIG(0): the command returns status; the dummy byte clocks out register 0. */
    uint8_t data_tx3[] = {HAL_RADIO_READ_CONFIG_0, HAL_SPI_DUMMY_BYTE};
    uint8_t data_rx3[sizeof(data_tx3)] = {0};

    /* Keep CS low until both DMA channels complete. */
    spi3_dma_rx_done = false;
    spi3_dma_tx_done = false;
    spi_transmit(&spi3_obj, data_rx3, data_tx3, sizeof(data_tx3));
    while (!spi3_dma_rx_done || !spi3_dma_tx_done) {}

    printf("nRF905 2 status: 0x%02X, CONFIG[0]: 0x%02X\n", (unsigned int)data_rx3[0], (unsigned int)data_rx3[1]);

    return true;
}

void hal_radio_test(void)
{
     nrf905_set_mode(&nrf905_1_obj, NRF905_MODE_SHOCKBURST_TX_RADIO_EN); 
     nrf905_set_mode(&nrf905_2_obj, NRF905_MODE_SHOCKBURST_RX_RADIO_EN); 
    
    unsigned char payload[] = HAL_RADIO_TEST_PAYLOAD;
    uint8_t payload_len = strlen((char*)payload);
    uint32_t tx_addr = HAL_RADIO_TEST_TX_ADDRESS;
    printf("Sending %s to %02lX from %p...\n", payload, tx_addr, &nrf905_1_obj);
    nrf905_set_tx_data(&nrf905_1_obj, tx_addr, payload, payload_len);   
    nrf905_set_mode(&nrf905_1_obj, NRF905_MODE_SHOCKBURST_TX_RADIO_EN); 
    
    systick_delay(HAL_RADIO_TEST_DELAY_MS);
    
    enum nrf905_status status2 = 0;
    nrf905_get_status(&nrf905_2_obj, &status2);
    printf("Got status: AM: %d, DR: %d on %p\n", !!(status2 & NRF905_STATUS_AM), !!(status2 & NRF905_STATUS_DR), &nrf905_2_obj);
    
    uint8_t received_data[sizeof(HAL_RADIO_TEST_PAYLOAD) + 1] = {0};
    nrf905_get_rx_data(&nrf905_2_obj, received_data, payload_len);
    received_data[sizeof(HAL_RADIO_TEST_PAYLOAD)] = 0;
    printf("Received data: %s, on %p\n", received_data, &nrf905_2_obj);

    nrf905_set_mode(&nrf905_1_obj, NRF905_MODE_STANDBY); 
    nrf905_set_mode(&nrf905_2_obj, NRF905_MODE_STANDBY); 
}

static uint8_t request_data[HAL_RADIO_TX_PAYLOAD_SIZE + 1] = {0};
static uint8_t received_data[HAL_RADIO_TX_PAYLOAD_SIZE + 1] = {0};

void hal_radio_rx_config(void)
{
    nrf905_set_mode(&nrf905_2_obj, NRF905_MODE_SHOCKBURST_RX_RADIO_EN); 

    /* Prepare SPI3 for RX when DR is high */
    SPI3->CR1 &= ~SPI_CR1_SPE;
    SPI3->CR2 |= SPI_CR2_RXDMAEN;
    SPI3->CR2 |= SPI_CR2_TXDMAEN;

    request_data[0] = 36; // NRF905_INST_R_RX_PAYLOAD;
    spi3_dma_config_internal((uintptr_t)&SPI3->DR, received_data, request_data, HAL_RADIO_TX_PAYLOAD_SIZE + 1, true, false, NULL, NULL);

    /* Prepare TIM4 input capture DMA requet generation on NRF 2 DR pin */
    RCC->APB1ENR1 |= RCC_APB1ENR1_TIM3EN;
    (void)RCC->APB1ENR1;

    TIM3->CCMR1 |= TIM_CCMR1_CC1S_0; // CC1 channel is configured as input, IC1 is mapped on TI1
    TIM3->CCMR1 |= TIM_CCMR1_IC1F_1 | TIM_CCMR1_IC1F_0; // IC1 filter: fSAMPLING=fckint/2, N=8
    TIM3->CCMR1 &= ~TIM_CCMR1_IC1PSC; // no prescaler, capture is done each time an edge is detected
    TIM3->CCER |= TIM_CCER_CC1P; // Bith edges
    TIM3->CCER |= TIM_CCER_CC1NP; // Both edges
    TIM3->CCER |= TIM_CCER_CC1E; // CC1 input capture enabled
    TIM3->DIER |= TIM_DIER_CC1DE; // CC1 DMA request enable

    /* Configure DMA channel and request for enabling SPI transfer on DR rising edge */
    static uint32_t spi3_cr1_data[2] = {0};
    spi3_cr1_data[0] = SPI3->CR1 | SPI_CR1_SPE;
    spi3_cr1_data[1] = SPI3->CR1 & ~SPI_CR1_SPE;

    dma_init(DMA_INSTANCE_1);
    struct dma_channel_cfg tim3_spi_start_cfg =
    {
        .inst = DMA_INSTANCE_1,
        .channel = DMA_CHANNEL_6,
        .tc = NULL,
        .peripheral_address = (uintptr_t)&SPI3->CR1,
        .memory_address = (uintptr_t)&spi3_cr1_data,
        .count = sizeof(spi3_cr1_data) / sizeof(uint32_t),
        .peripheral_size = DMA_DATA_SIZE_32_BIT,
        .memory_size = DMA_DATA_SIZE_32_BIT,
        .direction = DMA_MEMORY_TO_PERIPHERAL,
        .priority = DMA_PRIORITY_LOW,
        .memory_increment = true,
        .peripheral_increment = false,
        .circular = true,
        .memory_to_memory = false,
    };

    dma_channel_config(&tim3_spi_start_cfg);
    dma_channel_link(DMA_INSTANCE_1, DMA_CHANNEL_6, DMA_REQUEST_5);

    /* Remove tc callback from SPI3 DMA RX and TX config */
    DMA2_Channel1->CCR &= ~DMA_CCR_TCIE;
    DMA2_Channel2->CCR &= ~DMA_CCR_TCIE;
}

void hal_radio_rx_config2(void)
{
    DMA2_Channel2->CCR &= ~DMA_CCR_EN;
    DMA2_Channel2->CNDTR = HAL_RADIO_TX_PAYLOAD_SIZE + 1;
    DMA2_Channel2->CCR |= DMA_CCR_EN;

}

void hal_radio_loopback_test(void)
{
    nrf905_set_mode(&nrf905_1_obj, NRF905_MODE_SHOCKBURST_TX_RADIO_EN); 

    unsigned char payload[] = HAL_RADIO_TEST_PAYLOAD;
    uint8_t payload_len = strlen((char*)payload);
    uint32_t tx_addr = HAL_RADIO_TEST_TX_ADDRESS;
    printf("Sending %s to %02lX from %p...\n", payload, tx_addr, &nrf905_1_obj);
    nrf905_set_tx_data(&nrf905_1_obj, tx_addr, payload, payload_len);   
    nrf905_set_mode(&nrf905_1_obj, NRF905_MODE_SHOCKBURST_TX_RADIO_EN); 
    
    systick_delay(HAL_RADIO_TEST_DELAY_MS);

    /* Hardware magic */

    printf("Received data: %.*s, on %p\n", payload_len, received_data + 1, &nrf905_2_obj);

    memset(received_data + 1, 0x00, payload_len);

    nrf905_set_mode(&nrf905_1_obj, NRF905_MODE_STANDBY); 
}

void hal_led_toggle(void)
{
    GPIO_TOGGLE(HAL_LED_PORT, HAL_LED_PIN);
}

void hal_wdg_feed(void)
{
    wdg_feed();
}

void hal_delay(uint32_t ms)
{
    systick_delay(ms);
}

//------------------------------------------------------------------------------



