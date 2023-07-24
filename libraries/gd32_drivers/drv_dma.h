/*
 * Copyright (c) shenzhen hyw
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date                  Author       Notes
 * 2022-3-9 19:41:27     fengyang      first implementation
 */

#ifndef __DRV_USART_H__
#define __DRV_USART_H__

#include <rthw.h>
#include <rtthread.h>
#include <board.h>

#ifdef __cplusplus
extern "C" {
#endif

//#define UART_ENABLE_IRQ(n)            NVIC_EnableIRQ((n))
//#define UART_DISABLE_IRQ(n)           NVIC_DisableIRQ((n))

struct dma_config {
    uint32_t dma_periph;
    rcu_periph_enum dma_clk;
    dma_channel_enum  channelx;
    dma_parameter_struct  dma_init_config;
    IRQn_Type dma_irq;
};

#ifdef __cplusplus
}
#endif

#endif /* __DRV_USART_H__ */
