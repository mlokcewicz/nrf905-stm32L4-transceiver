//------------------------------------------------------------------------------

/// @file gpio.h
/// @note Copyright (C) Michał Łokcewicz. All rights reserved.

//------------------------------------------------------------------------------

#ifndef GPIO_H_
#define GPIO_H_

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------

#include <stm32l4xx.h>

//------------------------------------------------------------------------------

#define GPIO_MODE_INPUT  0b00
#define GPIO_MODE_OUTPUT 0b01
#define GPIO_MODE_AF     0b10
#define GPIO_MODE_ANALOG 0b11

#define GPIO_OTYPE_PP     0b0
#define GPIO_OTYPE_OD     0b1

#define GPIO_OSPEED_LOW   0b00
#define GPIO_OSPEED_MED   0b01
#define GPIO_OSPEED_HI    0b10
#define GPIO_OSPEED_VHI   0b11

#define GPIO_PUPD_NONE    0b00
#define GPIO_PUPD_PU      0b01
#define GPIO_PUPD_PD      0b10

//------------------------------------------------------------------------------

#define GPIO_CONFIG(port, pin, mode, otype, ospeed, pupd, af) \
do { \
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIO##port##EN;   \
    while(!(RCC->AHB2ENR & RCC_AHB2ENR_GPIO##port##EN)); \
    GPIO##port->MODER &= ~GPIO_MODER_MODE##pin; \
    GPIO##port->MODER |= mode << GPIO_MODER_MODE##pin##_Pos; \
    GPIO##port->OTYPER &= ~(GPIO_OTYPER_OT##pin); \
    GPIO##port->OTYPER |= otype << GPIO_OTYPER_OT##pin##_Pos; \
    GPIO##port->OSPEEDR &= ~(GPIO_OSPEEDR_OSPEED##pin); \
    GPIO##port->OSPEEDR |= ospeed << GPIO_OSPEEDR_OSPEED##pin##_Pos; \
    GPIO##port->PUPDR &= ~(GPIO_PUPDR_PUPD##pin##_Msk); \
    GPIO##port->PUPDR |= pupd << GPIO_PUPDR_PUPD##pin##_Pos; \
    if ((mode) == GPIO_MODE_AF) { \
        GPIO##port->AFR[(pin) / 8U] &= ~(GPIO_AFRL_AFSEL0_Msk << (((pin) % 8U) * (GPIO_AFRL_AFSEL1_Pos - GPIO_AFRL_AFSEL0_Pos))); \
        GPIO##port->AFR[(pin) / 8U] |=  ((af) << (((pin) % 8U) * (GPIO_AFRL_AFSEL1_Pos - GPIO_AFRL_AFSEL0_Pos))); } \
} while (0) 

#define GPIO_SET(port, pin, state) \
do { \
    state ? (GPIO##port->BSRR = 1 << pin) : (GPIO##port->BRR = 1 << pin); \
} while (0) 

#define GPIO_TOGGLE(port, pin) \
do { \
    GPIO##port->ODR ^= 1 << pin; \
} while (0) 

#define GPIO_GET(port, pin) (GPIO##port->IDR & (1 << pin))

//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif /* GPIO_H_ */

//------------------------------------------------------------------------------
