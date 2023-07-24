/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-06-21     flybreak     first version
 */

#include <rtthread.h>

#include "mb.h"
#include "user_mb_app.h"

#ifdef PKG_MODBUS_SLAVE_SAMPLE
#define SLAVE_ADDR      MB_SAMPLE_SLAVE_ADDR
#define PORT_NUM        MB_SLAVE_USING_PORT_NUM
#define PORT_BAUDRATE   MB_SLAVE_USING_PORT_BAUDRATE
#else
#define SLAVE_ADDR      0x01
#define PORT_NUM        2
#define PORT_BAUDRATE   9600
#endif

#define PORT_PARITY     MB_PAR_NONE

#define MB_POLL_THREAD_PRIORITY  11
#define MB_SEND_THREAD_PRIORITY  RT_THREAD_PRIORITY_MAX - 1

#define MB_POLL_CYCLE_MS 1

//extern USHORT usSRegHoldBuf[S_REG_HOLDING_NREGS];

//static void send_thread_entry(void *parameter)
//{
//    USHORT         *usRegHoldingBuf;
//    usRegHoldingBuf = CCR_DATA.usSRegHoldBuf;
//    rt_base_t level;

//    while (1)
//    {
//        /* Test Modbus Master */
//        level = rt_hw_interrupt_disable();

//        usRegHoldingBuf[3] = (USHORT)(rt_tick_get() / 100);

//        rt_hw_interrupt_enable(level);

//        rt_thread_mdelay(1000);
//    }
//}

static void mb_slave_poll(void *parameter)
{
    if (rt_strstr(parameter, "RTU"))
    {
#ifdef PKG_MODBUS_SLAVE_RTU
        eMBInit(MB_RTU, SLAVE_ADDR, PORT_NUM, PORT_BAUDRATE, PORT_PARITY);
#else
        rt_kprintf("Error: Please open RTU mode first");
#endif
    }
    else if (rt_strstr(parameter, "ASCII"))
    {
#ifdef PKG_MODBUS_SLAVE_ASCII
        eMBInit(MB_ASCII, SLAVE_ADDR, PORT_NUM, PORT_BAUDRATE, PORT_PARITY);
#else
        rt_kprintf("Error: Please open ASCII mode first");
#endif
    }
    else if (rt_strstr(parameter, "TCP"))
    {
#ifdef PKG_MODBUS_SLAVE_TCP
        eMBTCPInit(0);
#else
        rt_kprintf("Error: Please open TCP mode first");
#endif
    }
    else
    {
        rt_kprintf("Error: unknown parameter");
    }
    eMBEnable();
    while (1)
    {
        eMBPoll();
        rt_thread_mdelay(MB_POLL_CYCLE_MS);
    }
}

static int mb_slave_samlpe(void)
{
    rt_thread_t tid1 = RT_NULL;

    tid1 = rt_thread_create("md_s_poll", mb_slave_poll, "RTU", 1024, MB_POLL_THREAD_PRIORITY, 10);
    if (tid1 != RT_NULL)
    {
        rt_thread_startup(tid1);
    }
    else
    {
        goto __exit;
    }
    return RT_EOK;

__exit:
    if (tid1)
        rt_thread_delete(tid1);
    return -RT_ERROR;
}

INIT_APP_EXPORT(mb_slave_samlpe);
