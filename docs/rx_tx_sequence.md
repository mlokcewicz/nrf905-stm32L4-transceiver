# RX

After initialization, reception and forwarding to UART are entirely hardware driven: **no CPU execution, no polling, no interrupts**.

```plantuml
@startuml RX_sequence
title RX — fully hardware driven, no CPU involvement, no interrupts
autonumber
hide footbox
participant "CPU\nNo RX processing\nNo polling / no RX interrupts" as CPU
actor "Radio transmitter" as Sender
participant "nRF905 RX" as NRF
participant "TIM8\nDR: PC6 / TI1" as T8
participant "DMA2\nCh7 / Ch2 / Ch1" as CTRL_SPI
participant "SPI2\nNSS: PB12" as SPI
participant "DMA1\nCh5: SPI2 TX\nCh4: SPI2 RX" as SDMA
participant "RAM\nRX buffer" as RAM
participant "TIM1\nDR: PA8 / TI1" as T1
participant "DMA1\nCh7 / Ch6" as CTRL_UART
participant "DMA2 Ch3\nUART4 TX" as UDMA
participant "UART4\nTX: PC10" as UART
actor "UART host" as Host

== Packet received: DR rising edge ==
Sender -> NRF: Transmit radio packet over the air\nDestination address + 32-byte payload + CRC
NRF -> NRF: Receive packet and validate address / CRC
NRF --> T8: DR: 0 -> 1
T8 --> CTRL_SPI: CC2 capture: DMA request
CTRL_SPI -> SPI: Ch7: write CR1 with SPE=1\nEnable SPI2, NSS=0
T8 -> T8: Filtered TI1FP1 triggers counter
T8 --> CTRL_SPI: First period, CNT=5: CC4 DMA request
CTRL_SPI -> SDMA: Ch2: write Ch5 CCR with EN=0
T8 --> CTRL_SPI: Second period, CNT=5: CC4 DMA request
CTRL_SPI -> SDMA: Ch2: reload Ch5 CNDTR=33
T8 --> CTRL_SPI: Update after two periods
CTRL_SPI -> SDMA: Ch1: write Ch5 CCR with EN=1
T8 -> T8: One-pulse mode stops counter

== Read RX payload through SPI2 ==
loop 33 SPI bytes: command + 32-byte payload
SPI --> SDMA: TX DMA request
SDMA -> SPI: Ch5: command buffer byte -> SPI2 DR
SPI -> NRF: MOSI: R_RX_PAYLOAD (0x24), then dummy bytes\nSPI2 generates SCK
NRF -> SPI: MISO: status byte, then payload bytes
SPI --> SDMA: RX DMA request
SDMA -> RAM: Ch4: SPI2 DR -> RX buffer
end
SDMA -> SDMA: Ch5: normal transfer complete\nCh4: circular reload for next packet
NRF -> NRF: Payload read complete: DR=0

== DR falling edge: stop SPI2 and trigger UART forwarding ==
par Stop SPI2
NRF --> T8: DR: 1 -> 0
T8 --> CTRL_SPI: CC2 capture: DMA request
CTRL_SPI -> SPI: Ch7: write CR1 with SPE=0\nDisable SPI2, NSS=1
CTRL_SPI -> CTRL_SPI: Circular reload: next CC2 enables SPI2
else Start UART transfer sequence
NRF --> T1: Same DR: 1 -> 0
T1 -> T1: Filtered TI1FP1 triggers counter
T1 --> CTRL_UART: First period, CNT=5: CC3 DMA request
CTRL_UART -> UDMA: Ch7: write Ch3 CCR with EN=0
T1 --> CTRL_UART: Second period, CNT=5: CC3 DMA request
CTRL_UART -> UDMA: Ch7: reload Ch3 CNDTR=33
T1 --> CTRL_UART: Update after two periods
CTRL_UART -> UDMA: Ch6: write Ch3 CCR with EN=1
T1 -> T1: One-pulse mode stops counter
end

== Send received payload to UART host — DMA only, no CPU ==
loop 32 payload bytes + prefilled newline (status byte skipped)
UART --> UDMA: TX DMA request
RAM -> UDMA: RX buffer byte, starting at offset 1
UDMA -> UART: Write UART4 TDR
UART -> Host: Transmit byte at 115200 baud, 8N1
end
UDMA -> UDMA: Transfer complete: CNDTR=0, no interrupt
UART -> UART: Finish transmitting last byte
NRF -> NRF: Continue receiving, CE remains high
@enduml
```


# TX

UART reception and SPI transfers use DMA interrupts to advance the TX sequence. Radio transmission completion stops TX through TIM4 and DMA without CPU involvement.

```plantuml
@startuml TX_sequence
title TX — DMA interrupts load the packet; hardware stops radio transmission
autonumber
hide footbox
actor "UART host" as Host
participant "UART4\nRX: PC11 / TX: PC10" as UART
participant "DMA2 Ch5\nUART4 RX" as URX
participant "CPU / DMA interrupt handlers" as CPU
participant "RAM\nTX payload buffer" as RAM
participant "DMA1\nCh3: SPI1 TX\nCh2: SPI1 RX" as SDMA
participant "SPI1\nNSS: PA4" as SPI
participant "nRF905 TX\nPWR: PA0\nCE: PC8 / TX_EN: PC5" as NRF
participant "TIM4_CH1\nDR: PB6" as TIM
participant "DMA1 Ch1" as STOP
actor "Radio receiver" as Receiver

== Receive payload from UART ==
loop 32 payload bytes
Host -> UART: Receive byte
UART --> URX: RX DMA request
URX -> RAM: UART4 RDR -> payload buffer[i]
end
URX --> CPU: DMA2 Ch5 transfer-complete interrupt\nClear TC flag, call uart4_rx_dma_tc()
CPU -> CPU: uart4_rx_done=true\nTX state=HAL_TX_ADDR_SENDING

== Write destination address ==
CPU -> CPU: nrf905_set_tx_addr(CONFIG_RADIO_TX_ADDRESS)
CPU -> SPI: spi_transmit(): disable SPI, enable DMA requests
CPU -> SDMA: Start 5-byte transfer\nTX: 0x22 + 4 address bytes (LSB first)\nRX: discard into dummy_rx
CPU -> SPI: SPE=1, NSS=0
SDMA -> SPI: Write command and address bytes
SPI -> NRF: W_TX_ADDR (0x22) + destination address
SPI --> SDMA: Read and discard received bytes
SDMA --> CPU: Ch3 TC interrupt: spi1_dma_tx_tc()\nSet TX done flag
SDMA --> CPU: Ch2 TC interrupt: spi1_dma_rx_tc()
CPU -> SPI: spi_transmit_finish(): SPE=0, NSS=1\nDisable SPI DMA requests
CPU -> CPU: Set RX done flag\nTX state=HAL_TX_DATA_SENDING

== Write payload ==
CPU -> CPU: nrf905_set_tx_data(payload, 32)
CPU -> SPI: spi_transmit(): disable SPI, enable DMA requests
CPU -> SDMA: Start 33-byte transfer\nTX: 0x20 + 32 payload bytes\nRX: discard into dummy_rx
CPU -> SPI: SPE=1, NSS=0
SDMA -> SPI: Write command and payload bytes
SPI -> NRF: W_TX_PAYLOAD (0x20) + payload
SPI --> SDMA: Read and discard received bytes
SDMA --> CPU: Ch3 TC interrupt: spi1_dma_tx_tc()\nSet TX done flag
SDMA --> CPU: Ch2 TC interrupt: spi1_dma_rx_tc()
CPU -> SPI: spi_transmit_finish(): SPE=0, NSS=1\nDisable SPI DMA requests
CPU -> NRF: Set TX mode: PWR=1, then CE=1, then TX_EN=1
CPU -> CPU: TX state=HAL_TX_IDLE

== Send packet over the air ==
NRF -> Receiver: Transmit packet to CONFIG_RADIO_TX_ADDRESS\nPreamble + address + 32-byte payload + CRC

== Transmission complete — hardware stops TX, no CPU ==
NRF --> TIM: Transmission complete: DR 0 -> 1
TIM --> STOP: CC1 capture: DMA request
STOP -> NRF: Write stop mask to GPIOC BSRR\nClear CE and TX_EN simultaneously
NRF -> NRF: Enter standby, PWR remains high\nDR returns to 0
STOP -> STOP: Circular reload for next transmission\nNo transfer-complete interrupt
@enduml
```

