/*
 * STM32F2XX SPI
 *
 * Copyright (c) 2014 Alistair Francis <alistair@alistair23.me>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef HW_STM32F2XX_SPI_H
#define HW_STM32F2XX_SPI_H

#include "hw/sysbus.h"
#include "hw/ssi/ssi.h"
#include "qom/object.h"

#include "chardev/char-fe.h"
#include "hw/char/stm32f2xx_usart.h"

#define STM_SPI_CR1 0x00
#define STM_SPI_CR2 0x04
#define STM_SPI_SR 0x08
#define STM_SPI_DR 0x0C
#define STM_SPI_CRCPR 0x10
#define STM_SPI_RXCRCR 0x14
#define STM_SPI_TXCRCR 0x18
#define STM_SPI_I2SCFGR 0x1C
#define STM_SPI_I2SPR 0x20

#define STM_SPI_CR1_SPE (1 << 6)
#define STM_SPI_CR1_MSTR (1 << 2)

#define STM_SPI_SR_RXNE 1

#define TYPE_STM32F2XX_SPI "stm32f2xx-spi"
OBJECT_DECLARE_SIMPLE_TYPE(STM32F2XXSPIState, STM32F2XX_SPI)

// custom

/*
 * A small enum for the frame-parsing state machine.
 * This example is based on a frame that looks like:
 *   [Start = 0x01][Length][Data ...][Checksum][End = 0x0A]
 */
typedef enum
{
    FRAME_IDLE = 0,
    FRAME_GOT_START,
    FRAME_GOT_LENGTH,
    FRAME_RECEIVING_DATA,
    FRAME_WAIT_FOR_CHECKSUM,
    FRAME_WAIT_FOR_END,
} FrameState;

//
struct STM32F2XXSPIState
{
    /* <private> */
    SysBusDevice parent_obj;

    /* <public> */
    MemoryRegion mmio;

    uint32_t spi_cr1;
    uint32_t spi_cr2;
    uint32_t spi_sr;
    uint32_t spi_dr;
    uint32_t spi_crcpr;
    uint32_t spi_rxcrcr;
    uint32_t spi_txcrcr;
    uint32_t spi_i2scfgr;
    uint32_t spi_i2spr;

    qemu_irq irq;
    SSIBus *ssi;

    CharBackend chr;
    /* Add UART for console output */

    /* Frame-buffering logic */
    FrameState frame_state;
    uint8_t frame_buffer[256];
    int frame_index;
    int expected_len;
};

#endif /* HW_STM32F2XX_SPI_H */
