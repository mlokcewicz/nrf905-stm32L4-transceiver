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
/* Hardware definitions */

#define HAL_UART_TX_PORT                    A
#define HAL_UART_TX_PIN                     2
#define HAL_UART_RX_PORT                    A
#define HAL_UART_RX_PIN                     3

#define HAL_NRF_TX_SCK_PORT                 B
#define HAL_NRF_TX_SCK_PIN                  3
#define HAL_NRF_TX_MISO_PORT                A
#define HAL_NRF_TX_MISO_PIN                 6
#define HAL_NRF_TX_MOSI_PORT                A
#define HAL_NRF_TX_MOSI_PIN                 7
#define HAL_NRF_TX_CS_PORT                  A
#define HAL_NRF_TX_CS_PIN                   4
#define HAL_NRF_RX_SCK_PORT                 B
#define HAL_NRF_RX_SCK_PIN                  13
#define HAL_NRF_RX_MISO_PORT                B
#define HAL_NRF_RX_MISO_PIN                 14
#define HAL_NRF_RX_MOSI_PORT                B
#define HAL_NRF_RX_MOSI_PIN                 15
#define HAL_NRF_RX_CS_PORT                  B
#define HAL_NRF_RX_CS_PIN                   12
#define HAL_NRF_TX_TXE_PORT                 C
#define HAL_NRF_TX_TXE_PIN                  5
#define HAL_NRF_TX_PWR_PORT                 A
#define HAL_NRF_TX_PWR_PIN                  0
#define HAL_NRF_TX_CE_PORT                  C
#define HAL_NRF_TX_CE_PIN                   8
#define HAL_NRF_TX_DR_PORT                  C
#define HAL_NRF_TX_DR_PIN                   9
#define HAL_NRF_RX_CE_PORT                  C
#define HAL_NRF_RX_CE_PIN                   0
#define HAL_NRF_RX_TXE_PORT                 C
#define HAL_NRF_RX_TXE_PIN                  1
#define HAL_NRF_RX_PWR_PORT                 C
#define HAL_NRF_RX_PWR_PIN                  2
#define HAL_NRF_RX_DR_PORT                  C
#define HAL_NRF_RX_DR_PIN                   6
#define HAL_LED_PORT                        A
#define HAL_LED_PIN                         5

/* GPIO alternate functions */
#define HAL_UART_AF                         7
#define HAL_NRF_TX_SPI_AF                   5
#define HAL_NRF_RX_SPI_AF                   5
#define HAL_NRF_RX_DR_AF                    3

/* Clock tree and watchdog */
#define HAL_IRQ_PRIORITY_GROUPING           0x00000003U
#define HAL_VOLTAGE_SCALING                 PWR_VOLTAGE_SCALING_RANGE1
#define HAL_FLASH_LATENCY                   FLASH_ACR_LATENCY_2WS
#define HAL_WDG_PRESCALER                   IWDG_PRESCALER_256
#define HAL_WDG_RELOAD                      0x0FFFu
#define HAL_LSI_ON                          true
#define HAL_HSI_ON                          true
#define HAL_HSE_ON                          false
#define HAL_PLL_ON                          true
#define HAL_PLL_M                           RCC_PLLM_DIV2
#define HAL_PLL_N                           10
#define HAL_PLL_R                           RCC_PLLR_DIV2
#define HAL_PLL_R_ENABLE                    true
#define HAL_PLL_SOURCE                      RCC_PLLCFGR_PLLSRC_HSI
#define HAL_AHB_PRESCALER                   RCC_CFGR_HPRE_DIV1
#define HAL_APB1_PRESCALER                  RCC_CFGR_PPRE1_DIV1
#define HAL_APB2_PRESCALER                  RCC_CFGR_PPRE2_DIV1
#define HAL_SYSCLK_SOURCE                   RCC_CFGR_SW_PLL

/* USART configuration */
#define HAL_UART_INSTANCE                   USART_INSTANCE_2
#define HAL_UART_RX_TX                      (USART_TX_ENABLE | USART_RX_ENABLE)
#define HAL_UART_DATA_SIZE                  USART_DATASIZE_8_BIT
#define HAL_UART_PARITY                     USART_PARITY_DISABLED
#define HAL_UART_STOP_BITS                  USART_STOP_BITS_1
#define HAL_UART_OVERSAMPLING               USART_OVERSAMPLING_16
#define HAL_UART_BAUDRATE                   115200

/* Radio SPI configuration */
#define HAL_NRF_TX_SPI_INSTANCE             SPI_INSTANCE_1
#define HAL_NRF_RX_SPI_INSTANCE             SPI_INSTANCE_2
#define HAL_NRF_SPI_PRESCALER               SPI_CLOCK_PRESC_16
#define HAL_NRF_SPI_DATA_SIZE               SPI_DATASIZE_8_BIT
#define HAL_NRF_SPI_CPOL                    false
#define HAL_NRF_SPI_CPHA                    false
#define HAL_NRF_SPI_LSB_FIRST               false

#define HAL_NRF_SPI_HW_CS                   true
#define HAL_NRF_RX_SPI_BASE                 SPI2

/* SPI DMA mapping */
#define HAL_NRF_TX_SPI_DMA_INSTANCE         DMA_INSTANCE_1
#define HAL_NRF_TX_SPI_DMA_RX_CHANNEL       DMA_CHANNEL_2
#define HAL_NRF_TX_SPI_DMA_TX_CHANNEL       DMA_CHANNEL_3
#define HAL_NRF_TX_SPI_DMA_REQUEST          DMA_REQUEST_1
#define HAL_NRF_RX_SPI_DMA_INSTANCE         DMA_INSTANCE_1
#define HAL_NRF_RX_SPI_DMA_RX_CHANNEL       DMA_CHANNEL_4
#define HAL_NRF_RX_SPI_DMA_TX_CHANNEL       DMA_CHANNEL_5
#define HAL_NRF_RX_SPI_DMA_REQUEST          DMA_REQUEST_1

/* USART2 DMA mapping */
#define HAL_USART_DMA_INSTANCE              DMA_INSTANCE_1
#define HAL_USART_DMA_RX_CHANNEL            DMA_CHANNEL_6
#define HAL_USART_DMA_TX_CHANNEL            DMA_CHANNEL_7
#define HAL_USART_DMA_REQUEST               DMA_REQUEST_2


/* Shared radio configuration */
#define HAL_RADIO_RX_REDUCED_POWER          false
#define HAL_RADIO_AUTO_RETRANSMIT           false
#define HAL_RADIO_RX_ADDRESS_WIDTH          NRF905_ADDR_WIDTH_4_BYTE
#define HAL_RADIO_TX_ADDRESS_WIDTH          NRF905_ADDR_WIDTH_4_BYTE
#define HAL_RADIO_RX_PAYLOAD_SIZE           32
#define HAL_RADIO_TX_PAYLOAD_SIZE           32
#define HAL_RADIO_OUTPUT_CLOCK_FREQ         NRF905_UP_CLK_FREQ_4_MHZ
#define HAL_RADIO_OUTPUT_CLOCK_ENABLE       false
#define HAL_RADIO_CRYSTAL_FREQ              NRF905_XOF_16_MHZ
#define HAL_RADIO_CRC_ENABLE                true
#define HAL_RADIO_CRC_16_BIT                true

/* Radio diagnostics */
#define HAL_RADIO_STARTUP_DELAY_MS          3
#define HAL_RADIO_TEST_DELAY_MS             1000
#define HAL_RADIO_READ_CONFIG_0             0b00010000
#define HAL_NRF_SPI_DUMMY_BYTE              0x00
#define HAL_RADIO_TEST_PAYLOAD              "nRF905 RADIO TRX TEST MSG!"

//------------------------------------------------------------------------------
/* Type definitions */

enum hal_tx_state
{
    HAL_TX_IDLE,
    HAL_TX_ADDR_SENDING,
    HAL_TX_DATA_SENDING,
};

//------------------------------------------------------------------------------
/* Static objects */

static struct usart_obj usart2_obj = {0};
static volatile bool usart2_tx_done;
static volatile bool usart2_rx_done;

#if CONFIG_TRANSMITTER_EN
static struct spi_obj spi1_obj = {0};
static struct nrf905_obj nrf905_tx_obj;
static volatile bool spi1_dma_rx_done;
static volatile bool spi1_dma_tx_done;
static uint8_t usart2_rx_payload[HAL_RADIO_RX_PAYLOAD_SIZE];

static volatile enum hal_tx_state nrf905_tx_state;
#endif

#if CONFIG_RECEIVER_EN
static struct spi_obj spi2_obj = {0};
static struct nrf905_obj nrf905_rx_obj;
static volatile bool spi2_dma_rx_done;
static volatile bool spi2_dma_tx_done;
static uint8_t nrf_get_payload_request_data[HAL_RADIO_TX_PAYLOAD_SIZE + 1] = {0};
static uint8_t nrf_get_payload_received_data[HAL_RADIO_TX_PAYLOAD_SIZE + 1] = {0};
#endif

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
    usart_putc(&usart2_obj, ch);
    
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

static void usart2_rx_dma_tc(void)
{
    usart2_rx_done = true;

#if CONFIG_TRANSMITTER_EN
    nrf905_tx_state = HAL_TX_ADDR_SENDING;
    printf("Sending %.*s to %02lX from %p...\n", HAL_RADIO_TX_PAYLOAD_SIZE, usart2_rx_payload, CONFIG_RADIO_TX_ADDRESS, &nrf905_tx_obj);
    nrf905_set_tx_addr(&nrf905_tx_obj, CONFIG_RADIO_TX_ADDRESS);
#endif
}

static void usart2_tx_dma_tc(void)
{
    usart_send_finish(&usart2_obj);
    usart2_tx_done = true;
}

static void usart2_dma_config(uintptr_t periph_address, uint8_t *data, uint32_t len, bool is_rx)
{
    if (is_rx)
    {
        struct dma_channel_cfg rx_cfg =
        {
            .inst = HAL_USART_DMA_INSTANCE,
            .channel = HAL_USART_DMA_RX_CHANNEL,
            .tc = usart2_rx_dma_tc,
            .peripheral_address = periph_address,
            .memory_address = (uintptr_t)data,
            .count = len,
            .peripheral_size = DMA_DATA_SIZE_8_BIT,
            .memory_size = DMA_DATA_SIZE_8_BIT,
            .direction = DMA_PERIPHERAL_TO_MEMORY,
            .priority = DMA_PRIORITY_LOW,
            .memory_increment = true,
            .peripheral_increment = false,
            .circular = true,
            .memory_to_memory = false,
        };

        dma_channel_config(&rx_cfg);
    }
    else
    {
        struct dma_channel_cfg tx_cfg =
        {
            .inst = HAL_USART_DMA_INSTANCE,
            .channel = HAL_USART_DMA_TX_CHANNEL,
            .tc = usart2_tx_dma_tc,
            .peripheral_address = periph_address,
            .memory_address = (uintptr_t)data,
            .count = len,
            .peripheral_size = DMA_DATA_SIZE_8_BIT,
            .memory_size = DMA_DATA_SIZE_8_BIT,
            .direction = DMA_MEMORY_TO_PERIPHERAL,
            .priority = DMA_PRIORITY_LOW,
            .memory_increment = true,
            .peripheral_increment = false,
            .circular = false,
            .memory_to_memory = false,
        };

        dma_channel_config(&tx_cfg);
    }
}

#if CONFIG_TRANSMITTER_EN
static void spi1_dma_rx_tc(void)
{
    spi_transmit_finish(&spi1_obj);
    spi1_dma_rx_done = true;

#if CONFIG_TRANSMITTER_EN
    if (nrf905_tx_state == HAL_TX_ADDR_SENDING)
    {
        nrf905_tx_state = HAL_TX_DATA_SENDING;
        nrf905_set_tx_data(&nrf905_tx_obj, usart2_rx_payload, HAL_RADIO_TX_PAYLOAD_SIZE);   
    }
    else if (nrf905_tx_state == HAL_TX_DATA_SENDING)
    {
        nrf905_set_mode(&nrf905_tx_obj, NRF905_MODE_SHOCKBURST_TX_RADIO_EN); 
        
        // TODO: Remove this section and implement hw-driven RX
        volatile uint32_t delay = 0;
        for (delay = 0; delay < 100000; delay++) {} // Wait for the transmission to finish
        nrf905_set_mode(&nrf905_tx_obj, NRF905_MODE_STANDBY); 

        printf("Received data: %.*s, on %p\n", HAL_RADIO_TX_PAYLOAD_SIZE, nrf_get_payload_received_data + 1, &nrf905_rx_obj);

        memset(nrf_get_payload_received_data + 1, 0x00, HAL_RADIO_TX_PAYLOAD_SIZE);
        
        nrf905_tx_state = HAL_TX_IDLE;

        // hal_radio_rx_config_reload(); // remove
    }
#endif
}

static void spi1_dma_tx_tc(void)
{
    spi1_dma_tx_done = true;
}
#endif

#if CONFIG_RECEIVER_EN
static void spi2_dma_rx_tc(void)
{
    spi_transmit_finish(&spi2_obj);
    spi2_dma_rx_done = true;
}

static void spi2_dma_tx_tc(void)
{
    spi2_dma_tx_done = true;
}
#endif

#if CONFIG_TRANSMITTER_EN
static void spi1_dma_config(uintptr_t periph_address, uint8_t *data_rx, uint8_t *data_tx, uint32_t len)
{
    static uint8_t dummy_rx;
    static uint8_t dummy_tx = HAL_NRF_SPI_DUMMY_BYTE;

    struct dma_channel_cfg rx_cfg =
    {
        .inst = HAL_NRF_TX_SPI_DMA_INSTANCE,
        .channel = HAL_NRF_TX_SPI_DMA_RX_CHANNEL,
        .tc = spi1_dma_rx_tc,
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
        .inst = HAL_NRF_TX_SPI_DMA_INSTANCE,
        .channel = HAL_NRF_TX_SPI_DMA_TX_CHANNEL,
        .tc = spi1_dma_tx_tc,
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
#endif

#if CONFIG_RECEIVER_EN
static void spi2_dma_config_internal(uintptr_t periph_address, uint8_t *data_rx, uint8_t *data_tx, uint32_t len, bool circular_rx, bool circular_tx, void (*rx_tc)(void), void (*tx_tc)(void))
{
    static uint8_t dummy_rx;
    static uint8_t dummy_tx = HAL_NRF_SPI_DUMMY_BYTE;

    struct dma_channel_cfg rx_cfg =
    {
        .inst = HAL_NRF_RX_SPI_DMA_INSTANCE,
        .channel = HAL_NRF_RX_SPI_DMA_RX_CHANNEL,
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
        .inst = HAL_NRF_RX_SPI_DMA_INSTANCE,
        .channel = HAL_NRF_RX_SPI_DMA_TX_CHANNEL,
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

static void spi2_dma_config(uintptr_t periph_address, uint8_t *data_rx, uint8_t *data_tx, uint32_t len)
{
    spi2_dma_config_internal(periph_address, data_rx, data_tx, len, false, false, spi2_dma_rx_tc, spi2_dma_tx_tc);
}
#endif

#if CONFIG_TRANSMITTER_EN
static void spi1_cs_write(bool state)
{
    GPIO_SET(HAL_NRF_TX_CS_PORT, HAL_NRF_TX_CS_PIN, state);
}

static bool spi1_io_init(void)
{
    GPIO_CONFIG(HAL_NRF_TX_SCK_PORT, HAL_NRF_TX_SCK_PIN, GPIO_MODE_AF, GPIO_OTYPE_PP, GPIO_OSPEED_HI, GPIO_PUPD_NONE, HAL_NRF_TX_SPI_AF); /* SPI1 SCK */
    GPIO_CONFIG(HAL_NRF_TX_MISO_PORT, HAL_NRF_TX_MISO_PIN, GPIO_MODE_AF, GPIO_OTYPE_PP, GPIO_OSPEED_HI, GPIO_PUPD_NONE, HAL_NRF_TX_SPI_AF); /* SPI1 MISO */
    GPIO_CONFIG(HAL_NRF_TX_MOSI_PORT, HAL_NRF_TX_MOSI_PIN, GPIO_MODE_AF, GPIO_OTYPE_PP, GPIO_OSPEED_HI, GPIO_PUPD_NONE, HAL_NRF_TX_SPI_AF); /* SPI1 MOSI */
    GPIO_CONFIG(HAL_NRF_TX_CS_PORT, HAL_NRF_TX_CS_PIN, GPIO_MODE_AF, GPIO_OTYPE_PP, GPIO_OSPEED_LOW, GPIO_PUPD_PU, HAL_NRF_TX_SPI_AF);

    return true;
}
#endif


#if CONFIG_RECEIVER_EN
static void spi2_cs_write(bool state)
{
    GPIO_SET(HAL_NRF_RX_CS_PORT, HAL_NRF_RX_CS_PIN, state);
}

static bool spi2_io_init(void)
{
    GPIO_CONFIG(HAL_NRF_RX_SCK_PORT, HAL_NRF_RX_SCK_PIN, GPIO_MODE_AF, GPIO_OTYPE_PP, GPIO_OSPEED_HI, GPIO_PUPD_NONE, HAL_NRF_RX_SPI_AF); /* SPI2 SCK */
    GPIO_CONFIG(HAL_NRF_RX_MISO_PORT, HAL_NRF_RX_MISO_PIN, GPIO_MODE_AF, GPIO_OTYPE_PP, GPIO_OSPEED_HI, GPIO_PUPD_NONE, HAL_NRF_RX_SPI_AF); /* SPI2 MISO */
    GPIO_CONFIG(HAL_NRF_RX_MOSI_PORT, HAL_NRF_RX_MOSI_PIN, GPIO_MODE_AF, GPIO_OTYPE_PP, GPIO_OSPEED_HI, GPIO_PUPD_NONE, HAL_NRF_RX_SPI_AF); /* SPI2 MOSI */
    GPIO_CONFIG(HAL_NRF_RX_CS_PORT, HAL_NRF_RX_CS_PIN, GPIO_MODE_AF, GPIO_OTYPE_PP, GPIO_OSPEED_LOW, GPIO_PUPD_PU, HAL_NRF_RX_SPI_AF);

    return true;
}
#endif

//------------------------------------------------------------------------------
/* External drivers initialization */

#if CONFIG_TRANSMITTER_EN
static bool nrf905_tx_io_init_cb(void)
{
    GPIO_CONFIG(HAL_NRF_TX_TXE_PORT, HAL_NRF_TX_TXE_PIN, GPIO_MODE_OUTPUT, GPIO_OTYPE_PP, GPIO_OSPEED_LOW, GPIO_PUPD_NONE, 0);
    GPIO_CONFIG(HAL_NRF_TX_PWR_PORT, HAL_NRF_TX_PWR_PIN, GPIO_MODE_OUTPUT, GPIO_OTYPE_PP, GPIO_OSPEED_LOW, GPIO_PUPD_NONE, 0);
    GPIO_CONFIG(HAL_NRF_TX_CE_PORT, HAL_NRF_TX_CE_PIN, GPIO_MODE_OUTPUT, GPIO_OTYPE_PP, GPIO_OSPEED_LOW, GPIO_PUPD_NONE, 0);
    GPIO_CONFIG(HAL_NRF_TX_DR_PORT, HAL_NRF_TX_DR_PIN, GPIO_MODE_INPUT, GPIO_OTYPE_OD, GPIO_OSPEED_LOW, GPIO_PUPD_NONE, 0);

    return true;
}

static bool nrf905_tx_spi_transmit_cb(uint8_t *data_tx, uint8_t *data_rx, uint32_t len)
{
    spi1_dma_rx_done = false;
    spi1_dma_tx_done = false;

    spi_transmit(&spi1_obj, data_rx, data_tx, len);

    if (nrf905_tx_state != HAL_TX_IDLE)
        return true; 

    while (!spi1_dma_rx_done || !spi1_dma_tx_done) {}

    return true;
}

static bool nrf905_tx_pin_set_cb(enum nrf905_pin pin, bool state)
{
    switch (pin)
    {
        case NRF905_PIN_CS: spi_cs_set(&spi1_obj, state); break;
        case NRF905_PIN_PWR: GPIO_SET(HAL_NRF_TX_PWR_PORT, HAL_NRF_TX_PWR_PIN, state); break;
        case NRF905_PIN_CE: GPIO_SET(HAL_NRF_TX_CE_PORT, HAL_NRF_TX_CE_PIN, state); break;
        case NRF905_PIN_TXE: GPIO_SET(HAL_NRF_TX_TXE_PORT, HAL_NRF_TX_TXE_PIN, state); break;
        default: return false;
    }

    return true;
}

static bool nrf905_tx_pin_get_cb(enum nrf905_pin pin)
{
    switch (pin)
    {
        case NRF905_PIN_PWR: return GPIO_GET(HAL_NRF_TX_PWR_PORT, HAL_NRF_TX_PWR_PIN);
        case NRF905_PIN_CE: return GPIO_GET(HAL_NRF_TX_CE_PORT, HAL_NRF_TX_CE_PIN);
        case NRF905_PIN_TXE: return GPIO_GET(HAL_NRF_TX_TXE_PORT, HAL_NRF_TX_TXE_PIN);
        case NRF905_PIN_DRDY: return GPIO_GET(HAL_NRF_TX_DR_PORT, HAL_NRF_TX_DR_PIN);
        default: return false;
    }

    return true;
}
#endif


#if CONFIG_RECEIVER_EN
static bool nrf905_rx_io_init_cb(void)
{
    GPIO_CONFIG(HAL_NRF_RX_CE_PORT, HAL_NRF_RX_CE_PIN, GPIO_MODE_OUTPUT, GPIO_OTYPE_PP, GPIO_OSPEED_LOW, GPIO_PUPD_NONE, 0);
    GPIO_CONFIG(HAL_NRF_RX_TXE_PORT, HAL_NRF_RX_TXE_PIN, GPIO_MODE_OUTPUT, GPIO_OTYPE_PP, GPIO_OSPEED_LOW, GPIO_PUPD_NONE, 0);
    GPIO_CONFIG(HAL_NRF_RX_PWR_PORT, HAL_NRF_RX_PWR_PIN, GPIO_MODE_OUTPUT, GPIO_OTYPE_PP, GPIO_OSPEED_LOW, GPIO_PUPD_NONE, 0);
    GPIO_CONFIG(HAL_NRF_RX_DR_PORT, HAL_NRF_RX_DR_PIN, GPIO_MODE_AF, GPIO_OTYPE_OD, GPIO_OSPEED_LOW, GPIO_PUPD_NONE, HAL_NRF_RX_DR_AF);

    return true;
}

static bool nrf905_rx_spi_transmit_cb(uint8_t *data_tx, uint8_t *data_rx, uint32_t len)
{
    spi2_dma_rx_done = false;
    spi2_dma_tx_done = false;

    spi_transmit(&spi2_obj, data_rx, data_tx, len);

    while (!spi2_dma_rx_done || !spi2_dma_tx_done) {}

    return true;
}

static bool nrf905_rx_pin_set_cb(enum nrf905_pin pin, bool state)
{
    switch (pin)
    {
        case NRF905_PIN_CS: spi_cs_set(&spi2_obj, state); break;
        case NRF905_PIN_PWR: GPIO_SET(HAL_NRF_RX_PWR_PORT, HAL_NRF_RX_PWR_PIN, state); break;
        case NRF905_PIN_CE: GPIO_SET(HAL_NRF_RX_CE_PORT, HAL_NRF_RX_CE_PIN, state); break;
        case NRF905_PIN_TXE: GPIO_SET(HAL_NRF_RX_TXE_PORT, HAL_NRF_RX_TXE_PIN, state); break;
        default: return false;
    }

    return true;
}

static bool nrf905_rx_pin_get_cb(enum nrf905_pin pin)
{
    switch (pin)
    {
        case NRF905_PIN_PWR: return GPIO_GET(HAL_NRF_RX_PWR_PORT, HAL_NRF_RX_PWR_PIN);
        case NRF905_PIN_CE: return GPIO_GET(HAL_NRF_RX_CE_PORT, HAL_NRF_RX_CE_PIN);
        case NRF905_PIN_TXE: return GPIO_GET(HAL_NRF_RX_TXE_PORT, HAL_NRF_RX_TXE_PIN);
        case NRF905_PIN_DRDY: return GPIO_GET(HAL_NRF_RX_DR_PORT, HAL_NRF_RX_DR_PIN);
        default: return false;
    }

    return true;
}
#endif

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
    GPIO_CONFIG(HAL_LED_PORT, HAL_LED_PIN, GPIO_MODE_OUTPUT, GPIO_OTYPE_PP, GPIO_OSPEED_LOW, GPIO_PUPD_NONE, 0);

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
        .dma_config = usart2_dma_config,
    };

    if (!usart_init(&usart2_obj, &usart_cfg))
        return false;

    dma_init(HAL_USART_DMA_INSTANCE);

#if CONFIG_TRANSMITTER_EN
    dma_init(HAL_NRF_TX_SPI_DMA_INSTANCE);

    if (!dma_channel_link(HAL_NRF_TX_SPI_DMA_INSTANCE, HAL_NRF_TX_SPI_DMA_RX_CHANNEL, HAL_NRF_TX_SPI_DMA_REQUEST))
        return false;

    if (!dma_channel_link(HAL_NRF_TX_SPI_DMA_INSTANCE, HAL_NRF_TX_SPI_DMA_TX_CHANNEL, HAL_NRF_TX_SPI_DMA_REQUEST))
        return false;
#endif

#if CONFIG_RECEIVER_EN
    dma_init(HAL_NRF_RX_SPI_DMA_INSTANCE);

    if (!dma_channel_link(HAL_NRF_RX_SPI_DMA_INSTANCE, HAL_NRF_RX_SPI_DMA_RX_CHANNEL, HAL_NRF_RX_SPI_DMA_REQUEST))
        return false;

    if (!dma_channel_link(HAL_NRF_RX_SPI_DMA_INSTANCE, HAL_NRF_RX_SPI_DMA_TX_CHANNEL, HAL_NRF_RX_SPI_DMA_REQUEST))
        return false;
#endif

    if (!dma_channel_link(HAL_USART_DMA_INSTANCE, HAL_USART_DMA_RX_CHANNEL, HAL_USART_DMA_REQUEST)) /* USART2 RX */
        return false;

    if (!dma_channel_link(HAL_USART_DMA_INSTANCE, HAL_USART_DMA_TX_CHANNEL, HAL_USART_DMA_REQUEST)) /* USART2 RX */
        return false;

#if CONFIG_TRANSMITTER_EN
    /* NRF TX */    
    struct spi_cfg spi_cfg1 =
    {
        .inst = HAL_NRF_TX_SPI_INSTANCE,
        .clock_presc = HAL_NRF_SPI_PRESCALER,
        .data_size = HAL_NRF_SPI_DATA_SIZE,
        .cpol = HAL_NRF_SPI_CPOL,
        .cpha = HAL_NRF_SPI_CPHA,
        .lsb_first = HAL_NRF_SPI_LSB_FIRST,
        .hw_cs = HAL_NRF_SPI_HW_CS,
        .io_init = spi1_io_init,
        .dma_config = spi1_dma_config,
        .cs_set = spi1_cs_write,
    };

    if (!spi_init(&spi1_obj, &spi_cfg1))
        return false;

    struct nrf905_cfg nrf905_tx_cfg = 
    {
        .io_init_cb = nrf905_tx_io_init_cb,
        .spi_transmit_cb = nrf905_tx_spi_transmit_cb,
        .pin_set_cb = nrf905_tx_pin_set_cb,
        .pin_get_cb = nrf905_tx_pin_get_cb,

        .ch_no = CONFIG_RADIO_CHANNEL,
        .pll_freq = CONFIG_RADIO_BAND,
        .pa_pwr = CONFIG_RADIO_TX_POWER,
        .rx_red_pwr = HAL_RADIO_RX_REDUCED_POWER,
        .auto_retran = HAL_RADIO_AUTO_RETRANSMIT,
        .rx_afw = HAL_RADIO_RX_ADDRESS_WIDTH,
        .tx_afw = HAL_RADIO_TX_ADDRESS_WIDTH,
        .rx_pw = HAL_RADIO_RX_PAYLOAD_SIZE,
        .tx_pw = HAL_RADIO_TX_PAYLOAD_SIZE,
        .rx_addr = CONFIG_RADIO_RX_ADDRESS,
        .up_clk_freq = HAL_RADIO_OUTPUT_CLOCK_FREQ,
        .up_clk_en = HAL_RADIO_OUTPUT_CLOCK_ENABLE,
        .xof = HAL_RADIO_CRYSTAL_FREQ,
        .crc_en = HAL_RADIO_CRC_ENABLE,
        .crc_16 = HAL_RADIO_CRC_16_BIT,
    };

    if (!nrf905_init(&nrf905_tx_obj, &nrf905_tx_cfg))
    {
        printf("nRF905 TX init failed!\n");   
        return false;
    }
#endif

#if CONFIG_RECEIVER_EN
        /* NRF RX */
    struct spi_cfg spi_cfg2 =
    {
        .inst = HAL_NRF_RX_SPI_INSTANCE,
        .clock_presc = HAL_NRF_SPI_PRESCALER,
        .data_size = HAL_NRF_SPI_DATA_SIZE,
        .cpol = HAL_NRF_SPI_CPOL,
        .cpha = HAL_NRF_SPI_CPHA,
        .lsb_first = HAL_NRF_SPI_LSB_FIRST,
        .hw_cs = HAL_NRF_SPI_HW_CS,
        .io_init = spi2_io_init,
        .dma_config = spi2_dma_config,
        .cs_set = spi2_cs_write,
    };

    if (!spi_init(&spi2_obj, &spi_cfg2))
        return false;

    struct nrf905_cfg nrf905_rx_cfg = 
    {
        .io_init_cb = nrf905_rx_io_init_cb,
        .spi_transmit_cb = nrf905_rx_spi_transmit_cb,
        .pin_set_cb = nrf905_rx_pin_set_cb,
        .pin_get_cb = nrf905_rx_pin_get_cb,

        .ch_no = CONFIG_RADIO_CHANNEL,
        .pll_freq = CONFIG_RADIO_BAND,
        .pa_pwr = CONFIG_RADIO_TX_POWER,
        .rx_red_pwr = HAL_RADIO_RX_REDUCED_POWER,
        .auto_retran = HAL_RADIO_AUTO_RETRANSMIT,
        .rx_afw = HAL_RADIO_RX_ADDRESS_WIDTH,
        .tx_afw = HAL_RADIO_TX_ADDRESS_WIDTH,
        .rx_pw = HAL_RADIO_RX_PAYLOAD_SIZE,
        .tx_pw = HAL_RADIO_TX_PAYLOAD_SIZE,
        .rx_addr = CONFIG_RADIO_RX_ADDRESS,
        .up_clk_freq = HAL_RADIO_OUTPUT_CLOCK_FREQ,
        .up_clk_en = HAL_RADIO_OUTPUT_CLOCK_ENABLE,
        .xof = HAL_RADIO_CRYSTAL_FREQ,
        .crc_en = HAL_RADIO_CRC_ENABLE,
        .crc_16 = HAL_RADIO_CRC_16_BIT,
    };

    if (!nrf905_init(&nrf905_rx_obj, &nrf905_rx_cfg))
    {
        printf("nRF905 RX init failed!\n");
        return false;
    }
#endif

    return true;
}

void hal_radio_rx_config(void) // TODO: Remove direct register access
{
#if CONFIG_RECEIVER_EN
    nrf905_set_mode(&nrf905_rx_obj, NRF905_MODE_SHOCKBURST_RX_RADIO_EN); 

    /* Prepare SPI2 for RX when DR is high */
    HAL_NRF_RX_SPI_BASE->CR1 &= ~SPI_CR1_SPE;
    HAL_NRF_RX_SPI_BASE->CR2 &= ~SPI_CR2_RXDMAEN;
    HAL_NRF_RX_SPI_BASE->CR2 &= ~SPI_CR2_TXDMAEN;
    
    nrf_get_payload_request_data[0] = 36; // NRF905_INST_R_RX_PAYLOAD;
    spi2_dma_config_internal((uintptr_t)&HAL_NRF_RX_SPI_BASE->DR, nrf_get_payload_received_data, nrf_get_payload_request_data, HAL_RADIO_TX_PAYLOAD_SIZE + 1, true, false, NULL, NULL);
    
    /* Disable DMA channel to prevent unwanted transfers on first DR signal*/
    DMA1_Channel5->CCR &= ~DMA_CCR_EN;

    /* Enable SPI DMA request gneration */
    HAL_NRF_RX_SPI_BASE->CR2 |= SPI_CR2_RXDMAEN;
    HAL_NRF_RX_SPI_BASE->CR2 |= SPI_CR2_TXDMAEN;

    /* Prepare TIM8 input capture DMA requet generation on NRF RX DR pin */
    RCC->APB2ENR |= RCC_APB2ENR_TIM8EN;
    (void)RCC->APB2ENR;

    /* Disable DMA requests generation */
    TIM8->DIER = 0; 

    /* Generate CC1 event on the rising edge captured by CH1 */
    TIM8->CCMR1 |= TIM_CCMR1_CC1S_0 | TIM_CCMR1_CC2S_1; // CC1 channel is configured as input, IC1 is mapped on TI1, CC2 channel is configured as input, IC2 is mapped on TI1
    TIM8->CCMR1 |= TIM_CCMR1_IC1F_1 | TIM_CCMR1_IC1F_0; // IC1 filter: fSAMPLING=fckint/2, N=8
    TIM8->CCMR1 &= ~TIM_CCMR1_IC1PSC; // no prescaler, capture is done each time an edge is detected
    TIM8->CCER &= ~TIM_CCER_CC1P;  // Raising edges
    TIM8->CCER &= ~TIM_CCER_CC1NP; // Rising edges
    TIM8->CCER |= TIM_CCER_CC1E; // CC1 input capture enabled

    /* Trigger 2 clock cycles to generate DMA reqests for enabling SPI RX DMA */
    TIM8->SMCR |= TIM_SMCR_TS_2 | TIM_SMCR_TS_0; // Trigger selection: TI1FP1 TS: 101
    TIM8->SMCR |= TIM_SMCR_SMS_2 | TIM_SMCR_SMS_1; // Slave mode selection: Trigger Mode SMS: 0110
    TIM8->ARR = 100 - 1; // Auto-reload value: 100 clock cycles
    TIM8->CCR4 = 50; // Compare value: 50 clock cycles
    TIM8->RCR = 1; // Repetition coun ter: 2 repetition
    TIM8->CR1 |= TIM_CR1_OPM; // One-pulse mode enabled (only 2 DMA requests will be generated)

    TIM8->EGR = TIM_EGR_UG; // Load RCR, ARR and CCRx registers with their preload values before DMA requests are enabked
    TIM8->SR = 0;

    TIM8->DIER |= TIM_DIER_CC4DE; // 

    // repetition #1:
    // CNT 0 ... 50 ... 99
    //           ^
    //           DMA request #1

    // repetition #2:
    // CNT 0 ... 50 ... 99
    //           ^
    //           DMA request #2

    /* Generate CC2 events for DMA requests for enabling DMA to enable SPI */
    TIM8->CCER |= TIM_CCER_CC2P; // Both edges
    TIM8->CCER |= TIM_CCER_CC2NP; // Both edges
    TIM8->CCER |= TIM_CCER_CC2E; // CC2 input capture enabled
    TIM8->DIER |= TIM_DIER_CC2DE; // CC2 DMA request enable

    /* Generate UP events for DMA requests for enabling DMA to disable SPI TX DMA */
    TIM8->DIER |= TIM_DIER_UDE; 

    /* Configure DMA channel and request for enabling SPI transfer on DR rising edge */
    static uint32_t spi2_cr1_data[2] = {0};
    spi2_cr1_data[0] = HAL_NRF_RX_SPI_BASE->CR1 | SPI_CR1_SPE;
    spi2_cr1_data[1] = HAL_NRF_RX_SPI_BASE->CR1 & ~SPI_CR1_SPE;

    dma_init(DMA_INSTANCE_2);
    struct dma_channel_cfg tim8_spi_start_cfg =
    {
        .inst = DMA_INSTANCE_2,
        .channel = DMA_CHANNEL_7,
        .tc = NULL,
        .peripheral_address = (uintptr_t)&HAL_NRF_RX_SPI_BASE->CR1,
        .memory_address = (uintptr_t)&spi2_cr1_data,
        .count = sizeof(spi2_cr1_data) / sizeof(uint32_t),
        .peripheral_size = DMA_DATA_SIZE_32_BIT,
        .memory_size = DMA_DATA_SIZE_32_BIT,
        .direction = DMA_MEMORY_TO_PERIPHERAL,
        .priority = DMA_PRIORITY_LOW,
        .memory_increment = true,
        .peripheral_increment = false,
        .circular = true,
        .memory_to_memory = false,
    };

    dma_channel_config(&tim8_spi_start_cfg);
    dma_channel_link(DMA_INSTANCE_2, DMA_CHANNEL_7, DMA_REQUEST_7);

    /* Configure DMA channel and request for enabling disabling SPI RX DMA on DR rising edge */
    static uint32_t nrf_rx_dma_spi_rx_rearm_cfg[2] = {0};
    nrf_rx_dma_spi_rx_rearm_cfg[0] =  DMA1_Channel5->CCR & ~DMA_CCR_EN; /* Disable DMA channel in CCR */
    nrf_rx_dma_spi_rx_rearm_cfg[1] =  HAL_RADIO_TX_PAYLOAD_SIZE + 1;    /* Set the number of data items to transfer in CNDTR */

    struct dma_channel_cfg tim8_nrf_rx_dma_spi_rx_rearm_cfg =
    {
        .inst = DMA_INSTANCE_2,
        .channel = DMA_CHANNEL_2,
        .tc = NULL,
        .peripheral_address = (uintptr_t)DMA1_Channel5,
        .memory_address = (uintptr_t)&nrf_rx_dma_spi_rx_rearm_cfg,
        .count = sizeof(nrf_rx_dma_spi_rx_rearm_cfg) / sizeof(uint32_t),
        .peripheral_size = DMA_DATA_SIZE_32_BIT,
        .memory_size = DMA_DATA_SIZE_32_BIT,
        .direction = DMA_MEMORY_TO_PERIPHERAL,
        .priority = DMA_PRIORITY_LOW,
        .memory_increment = true,
        .peripheral_increment = true,
        .circular = true,
        .memory_to_memory = false,
    };

    dma_channel_config(&tim8_nrf_rx_dma_spi_rx_rearm_cfg);
    dma_channel_link(DMA_INSTANCE_2, DMA_CHANNEL_2, DMA_REQUEST_7);

        /* Configure DMA channel and request for enabling disabling SPI RX DMA on DR rising edge */
    static uint32_t nrf_rx_dma_spi_rx_enable_cfg = 0;
    nrf_rx_dma_spi_rx_enable_cfg =  DMA1_Channel5->CCR | DMA_CCR_EN; /* Enable DMA channel in CCR */

    struct dma_channel_cfg tim8_nrf_rx_dma_spi_rx_enable_cfg =
    {
        .inst = DMA_INSTANCE_2,
        .channel = DMA_CHANNEL_1,
        .tc = NULL,
        .peripheral_address = (uintptr_t)DMA1_Channel5,
        .memory_address = (uintptr_t)&nrf_rx_dma_spi_rx_enable_cfg,
        .count = sizeof(nrf_rx_dma_spi_rx_enable_cfg) / sizeof(uint32_t),
        .peripheral_size = DMA_DATA_SIZE_32_BIT,
        .memory_size = DMA_DATA_SIZE_32_BIT,
        .direction = DMA_MEMORY_TO_PERIPHERAL,
        .priority = DMA_PRIORITY_LOW,
        .memory_increment = true,
        .peripheral_increment = true,
        .circular = true,
        .memory_to_memory = false,
    };

    dma_channel_config(&tim8_nrf_rx_dma_spi_rx_enable_cfg);
    dma_channel_link(DMA_INSTANCE_2, DMA_CHANNEL_1, DMA_REQUEST_7);
#endif
}

void hal_radio_rx_config_reload(void) // TODO: Remove
{
#if CONFIG_RECEIVER_EN
    DMA1_Channel5->CCR &= ~DMA_CCR_EN;
    DMA1_Channel5->CNDTR = HAL_RADIO_TX_PAYLOAD_SIZE + 1;
    DMA1_Channel5->CCR |= DMA_CCR_EN;
#endif
}

void hal_radio_tx_send(void)
{
#if CONFIG_TRANSMITTER_EN
    printf("Sending %.*s to %02lX from %p...\n", HAL_RADIO_TX_PAYLOAD_SIZE, usart2_rx_payload, CONFIG_RADIO_TX_ADDRESS, &nrf905_tx_obj);
    nrf905_set_tx_addr(&nrf905_tx_obj, CONFIG_RADIO_TX_ADDRESS);
    nrf905_set_tx_data(&nrf905_tx_obj, usart2_rx_payload, HAL_RADIO_TX_PAYLOAD_SIZE);   
    nrf905_set_mode(&nrf905_tx_obj, NRF905_MODE_SHOCKBURST_TX_RADIO_EN); 

    systick_delay(HAL_RADIO_TEST_DELAY_MS); // TODO: Use DR

    nrf905_set_mode(&nrf905_tx_obj, NRF905_MODE_STANDBY); 

#if CONFIG_RECEIVER_EN
    /* Hardware magic */

    printf("Received data: %.*s, on %p\n", HAL_RADIO_TX_PAYLOAD_SIZE, nrf_get_payload_received_data + 1, &nrf905_rx_obj);

    memset(nrf_get_payload_received_data + 1, 0x00, HAL_RADIO_TX_PAYLOAD_SIZE);
#endif
#endif
}

void hal_radio_loopback_test(void)
{
#if CONFIG_TRANSMITTER_EN && CONFIG_RECEIVER_EN
     nrf905_set_mode(&nrf905_tx_obj, NRF905_MODE_SHOCKBURST_TX_RADIO_EN); 
     nrf905_set_mode(&nrf905_rx_obj, NRF905_MODE_SHOCKBURST_RX_RADIO_EN); 
    
    unsigned char payload[] = HAL_RADIO_TEST_PAYLOAD;
    uint8_t payload_len = strlen((char*)payload);
    uint32_t tx_addr = CONFIG_RADIO_TX_ADDRESS;
    printf("Sending %s to %02lX from %p...\n", payload, tx_addr, &nrf905_tx_obj);
    nrf905_set_tx_addr(&nrf905_tx_obj, tx_addr);
    nrf905_set_tx_data(&nrf905_tx_obj, payload, payload_len);   
    nrf905_set_mode(&nrf905_tx_obj, NRF905_MODE_SHOCKBURST_TX_RADIO_EN); 
    
    systick_delay(HAL_RADIO_TEST_DELAY_MS);
    
    enum nrf905_status status2 = 0;
    nrf905_get_status(&nrf905_rx_obj, &status2);
    printf("Got status: AM: %d, DR: %d on %p\n", !!(status2 & NRF905_STATUS_AM), !!(status2 & NRF905_STATUS_DR), &nrf905_rx_obj);
    
    uint8_t received_data[sizeof(HAL_RADIO_TEST_PAYLOAD) + 1] = {0};
    nrf905_get_rx_data(&nrf905_rx_obj, received_data, payload_len);
    received_data[sizeof(HAL_RADIO_TEST_PAYLOAD)] = 0;
    printf("Received data: %s, on %p\n", received_data, &nrf905_rx_obj);

    nrf905_set_mode(&nrf905_tx_obj, NRF905_MODE_STANDBY); 
    nrf905_set_mode(&nrf905_rx_obj, NRF905_MODE_SHOCKBURST_RX_RADIO_EN); 
#endif
}

void hal_radio_loopback_test_hw_driven(void)
{
#if CONFIG_TRANSMITTER_EN && CONFIG_RECEIVER_EN

    unsigned char payload[] = HAL_RADIO_TEST_PAYLOAD;
    uint8_t payload_len = strlen((char*)payload);
    uint32_t tx_addr = CONFIG_RADIO_RX_ADDRESS;
    printf("Sending %s to %02lX from %p...\n", payload, tx_addr, &nrf905_tx_obj);
    nrf905_set_tx_addr(&nrf905_tx_obj, tx_addr);
    nrf905_set_tx_data(&nrf905_tx_obj, payload, payload_len);
    nrf905_set_mode(&nrf905_tx_obj, NRF905_MODE_SHOCKBURST_TX_RADIO_EN);

    systick_delay(HAL_RADIO_TEST_DELAY_MS);

    nrf905_set_mode(&nrf905_tx_obj, NRF905_MODE_STANDBY); 

    /* Hardware magic */

    printf("Received data: %.*s, on %p\n", payload_len, nrf_get_payload_received_data + 1, &nrf905_rx_obj);

    memset(nrf_get_payload_received_data + 1, 0x00, payload_len);
#endif
}

void hal_uart_rx_start(void)
{
#if CONFIG_TRANSMITTER_EN
    usart2_rx_done = false;
    usart_receive(&usart2_obj, usart2_rx_payload, sizeof(usart2_rx_payload));
#endif
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

#if 0
void hal_radio_rx_config_new(void)
{
    nrf905_set_mode(&nrf905_rx_obj, NRF905_MODE_SHOCKBURST_RX_RADIO_EN); 

    /* Prepare SPI2 for RX when DR is high */
    HAL_NRF_RX_SPI_BASE->CR1 &= ~SPI_CR1_SPE;
    HAL_NRF_RX_SPI_BASE->CR2 |= SPI_CR2_RXDMAEN;
    HAL_NRF_RX_SPI_BASE->CR2 &= ~SPI_CR2_TXDMAEN; // TX DMA requests will be generated by TIM1

    nrf_get_payload_request_data[0] = 36; // NRF905_INST_R_RX_PAYLOAD;
    spi2_dma_config_internal((uintptr_t)&HAL_NRF_RX_SPI_BASE->DR, nrf_get_payload_received_data, nrf_get_payload_request_data, HAL_RADIO_TX_PAYLOAD_SIZE + 1, true, false, NULL, NULL);

    DMA1_Channel5->CCR &= ~DMA_CCR_EN; // SPI2 TX DMA is not used anymore

    /* Prepare TIM2 input capture on NRF RX DR pin */
    RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN;
    (void)RCC->APB1ENR1;

    TIM2->CR1 = 0;
    TIM2->CR2 = 0;
    TIM2->SMCR = 0;
    TIM2->DIER = 0;
    TIM2->CCMR1 = 0;
    TIM2->CCER = 0;

    TIM2->CCMR1 |= TIM_CCMR1_CC1S_0; // IC1 mapped directly on TI1
    TIM2->CCMR1 |= TIM_CCMR1_IC1F_1 | TIM_CCMR1_IC1F_0; // IC1 filter: fSAMPLING=fckint/2, N=8
    TIM2->CCMR1 &= ~TIM_CCMR1_IC1PSC; // no prescaler
    TIM2->CCER |= TIM_CCER_CC1E; // CH1 enabled, rising edge used by TI1FP1

    TIM2->CCMR1 |= TIM_CCMR1_CC2S_1; // IC2 mapped indirectly on TI1
    TIM2->CCMR1 |= TIM_CCMR1_IC2F_1 | TIM_CCMR1_IC2F_0; // same filter
    TIM2->CCMR1 &= ~TIM_CCMR1_IC2PSC; // no prescaler
    TIM2->CCER |= TIM_CCER_CC2P; // Both edges
    TIM2->CCER |= TIM_CCER_CC2NP; // Both edges
    TIM2->CCER |= TIM_CCER_CC2E; // CH2 input capture enabled
    TIM2->DIER |= TIM_DIER_CC2DE; // CH2 DMA request enable

    TIM2->SMCR |= TIM_SMCR_TS_2 | TIM_SMCR_TS_0; // TI1FP1 selected as trigger input
    TIM2->SMCR |= TIM_SMCR_SMS_2; // reset mode: rising edge generates update event

    TIM2->CR2 |= TIM_CR2_MMS_1; // update event is exported as TRGO


    /* Configure DMA channel for enabling/disabling SPI on DR edges */
    static uint32_t spi2_cr1_data[2] = {0};
    spi2_cr1_data[0] = HAL_NRF_RX_SPI_BASE->CR1 | SPI_CR1_SPE;
    spi2_cr1_data[1] = HAL_NRF_RX_SPI_BASE->CR1 & ~SPI_CR1_SPE;

    dma_init(DMA_INSTANCE_1);

    struct dma_channel_cfg tim2_spi_start_cfg =
    {
        .inst = DMA_INSTANCE_1,
        .channel = DMA_CHANNEL_7,
        .tc = NULL,
        .peripheral_address = (uintptr_t)&HAL_NRF_RX_SPI_BASE->CR1,
        .memory_address = (uintptr_t)&spi2_cr1_data,
        .count = sizeof(spi2_cr1_data) / sizeof(uint32_t),
        .peripheral_size = DMA_DATA_SIZE_32_BIT,
        .memory_size = DMA_DATA_SIZE_32_BIT,
        .direction = DMA_MEMORY_TO_PERIPHERAL,
        .priority = DMA_PRIORITY_LOW,
        .memory_increment = true,
        .peripheral_increment = false,
        .circular = true,
        .memory_to_memory = false,
    };

    dma_channel_config(&tim2_spi_start_cfg);
    dma_channel_link(DMA_INSTANCE_1, DMA_CHANNEL_7, DMA_REQUEST_7); // TIM2_CH2 -> DMA1 Channel 7

    /* TIM1 generates SPI TX DMA requests */
    RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;
    (void)RCC->APB2ENR;

    TIM1->CR1 = 0;
    TIM1->CR2 = 0;
    TIM1->SMCR = 0;
    TIM1->DIER = 0;
    TIM1->CCMR1 = 0;
    TIM1->CCER = 0;

    TIM1->PSC = 0;
    TIM1->ARR = 160 - 1; // one DMA request every 4 us at 40 MHz
    TIM1->CCR1 = 16; // first request 0.4 us after TIM1 start
    TIM1->RCR = HAL_RADIO_TX_PAYLOAD_SIZE; // 33 periods: command + 32 dummy bytes

    TIM1->CR1 |= TIM_CR1_OPM; // stop automatically after 33 periods

    TIM1->SMCR |= TIM_SMCR_TS_0; // ITR1 = TIM2 TRGO
    TIM1->SMCR |= TIM_SMCR_SMS_2 | TIM_SMCR_SMS_1; // trigger mode

    TIM1->EGR = TIM_EGR_UG;
    TIM1->SR = 0;

    TIM1->DIER |= TIM_DIER_CC1DE; // CH1 compare generates DMA request


    /* DMA for TIM1 generated SPI TX requests */
    struct dma_channel_cfg tim1_spi_tx_cfg =
    {
        .inst = DMA_INSTANCE_1,
        .channel = DMA_CHANNEL_2,
        .tc = NULL,
        .peripheral_address = (uintptr_t)&HAL_NRF_RX_SPI_BASE->DR,
        .memory_address = (uintptr_t)nrf_get_payload_request_data,
        .count = HAL_RADIO_TX_PAYLOAD_SIZE + 1,
        .peripheral_size = DMA_DATA_SIZE_8_BIT,
        .memory_size = DMA_DATA_SIZE_8_BIT,
        .direction = DMA_MEMORY_TO_PERIPHERAL,
        .priority = DMA_PRIORITY_LOW,
        .memory_increment = true,
        .peripheral_increment = false,
        .circular = true,
        .memory_to_memory = false,
    };

    dma_channel_config(&tim1_spi_tx_cfg);
    dma_channel_link(DMA_INSTANCE_1, DMA_CHANNEL_2, DMA_REQUEST_7); // TIM1_CH1 -> DMA1 Channel 2

    TIM2->SR = 0;
    TIM2->CR1 |= TIM_CR1_CEN;
}

#endif