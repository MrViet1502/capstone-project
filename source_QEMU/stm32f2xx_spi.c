/*
 * STM32F405 SPI
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

#include "qemu/osdep.h"
#include "qemu/log.h"
#include "qemu/module.h"
#include "hw/ssi/stm32f2xx_spi.h"
#include "migration/vmstate.h"

#ifndef STM_SPI_ERR_DEBUG
#define STM_SPI_ERR_DEBUG 0
#endif

#define DB_PRINT_L(lvl, fmt, args...)               \
    do                                              \
    {                                               \
        if (STM_SPI_ERR_DEBUG >= lvl)               \
        {                                           \
            qemu_log("%s: " fmt, __func__, ##args); \
        }                                           \
    } while (0)

#define DB_PRINT(fmt, args...) DB_PRINT_L(1, fmt, ##args)

// Khai báo trước hàm stm32f2xx_spi_write
static void stm32f2xx_spi_write(void *opaque, hwaddr addr, uint64_t val64, unsigned int size);

static void stm32f2xx_spi_reset(DeviceState *dev)
{
    STM32F2XXSPIState *s = STM32F2XX_SPI(dev);

    s->spi_cr1 = 0x00000000;
    s->spi_cr2 = 0x00000000;
    s->spi_sr = 0x0000000A;
    s->spi_dr = 0x0000000C;
    s->spi_crcpr = 0x00000007;
    s->spi_rxcrcr = 0x00000000;
    s->spi_txcrcr = 0x00000000;
    s->spi_i2scfgr = 0x00000000;
    s->spi_i2spr = 0x00000002;

    /* Reset our frame parsing state machine */
    s->frame_state = FRAME_IDLE;
    s->frame_index = 0;
    s->expected_len = 0;
    memset(s->frame_buffer, 0, sizeof(s->frame_buffer));
}

static void stm32f2xx_spi_transfer(STM32F2XXSPIState *s)
{
    DB_PRINT("SPI Transfer Start. Data to send: 0x%x\n", s->spi_dr);

    s->spi_dr = ssi_transfer(s->ssi, s->spi_dr);
    s->spi_sr |= STM_SPI_SR_RXNE;

    DB_PRINT("SPI Transfer Complete. Data received: 0x%x\n", s->spi_dr);
}

static uint64_t stm32f2xx_spi_read(void *opaque, hwaddr addr,
                                   unsigned int size)
{
    printf("stm32f2xx_spi_read is running...\n");

    DB_PRINT("Address: 0x%" HWADDR_PRIx "\n", addr);
    STM32F2XXSPIState *s = opaque;

    switch (addr)
    {
    case STM_SPI_CR1:
        return s->spi_cr1;
    case STM_SPI_CR2:
        qemu_log_mask(LOG_UNIMP, "%s: Interrupts and DMA are not implemented\n",
                      __func__);
        return s->spi_cr2;
    case STM_SPI_SR:
        return s->spi_sr;
    case STM_SPI_DR:
        // uint8_t recv_data[256];
        // printf("Start Read DR\n");
        // // Kiểm tra xem có dữ liệu sẵn có từ TCP hay không bằng select()

        // Có dữ liệu sẵn sàng, đọc nó
        // int ret = qemu_chr_fe_read_all(&s->chr, recv_data, 1);
        // if (ret > 0)
        // {
        //     printf("QEMU Received from TCP: %02X \n", recv_data[0]);
        //     if (recv_data[0] != 0x00)
        //     {
        //         s->spi_dr = recv_data[0];
        //     }
        //     else
        //     {
        //         s->spi_dr = recv_data[0];
        //     }
        // }

        // else
        {
            // Không có dữ liệu sẵn sàng, dùng giá trị mặc định
            s->spi_dr = 0x01;
            printf("No data received, using default value.\n");
        }
        return s->spi_dr;

    case STM_SPI_CRCPR:
        qemu_log_mask(LOG_UNIMP, "%s: CRC is not implemented, the registers "
                                 "are included for compatibility\n",
                      __func__);
        return s->spi_crcpr;
    case STM_SPI_RXCRCR:
        qemu_log_mask(LOG_UNIMP, "%s: CRC is not implemented, the registers "
                                 "are included for compatibility\n",
                      __func__);
        return s->spi_rxcrcr;
    case STM_SPI_TXCRCR:
        qemu_log_mask(LOG_UNIMP, "%s: CRC is not implemented, the registers "
                                 "are included for compatibility\n",
                      __func__);
        return s->spi_txcrcr;
    case STM_SPI_I2SCFGR:
        qemu_log_mask(LOG_UNIMP, "%s: I2S is not implemented, the registers "
                                 "are included for compatibility\n",
                      __func__);
        return s->spi_i2scfgr;
    case STM_SPI_I2SPR:
        qemu_log_mask(LOG_UNIMP, "%s: I2S is not implemented, the registers "
                                 "are included for compatibility\n",
                      __func__);
        return s->spi_i2spr;
    default:
        qemu_log_mask(LOG_GUEST_ERROR, "%s: Bad offset 0x%" HWADDR_PRIx "\n",
                      __func__, addr);
    }

    return 0;
}

static void stm32f2xx_spi_write(void *opaque, hwaddr addr,
                                uint64_t val64, unsigned int size)

{
    STM32F2XXSPIState *s = opaque;
    uint8_t byte = (uint8_t)val64; /* We'll treat 'value' as a single byte. */
    uint32_t value = val64;
    static uint8_t spi_buffer[256]; // Bộ đệm để lưu trữ dữ liệu SPI
    static uint8_t spi_index = 0;
    uint16_t EXPECTED_FRAME_SIZE = size;
    DB_PRINT("Address: 0x%" HWADDR_PRIx ", Value: 0x%x\n", addr, val64);

    printf("stm32f2xx_spi_write is running...\n");

    switch (addr)
    {
    case STM_SPI_CR1:
        s->spi_cr1 = value;
        return;
    case STM_SPI_CR2:
        qemu_log_mask(LOG_UNIMP, "%s: "
                                 "Interrupts and DMA are not implemented\n",
                      __func__);
        s->spi_cr2 = value;
        return;
    case STM_SPI_SR:
        /* Read only register, except for clearing the CRCERR bit, which
         * is not supported
         */
        return;
    case STM_SPI_DR:

        printf("SPI_Write_DR - Data: %08X\n", val64);
        printf("Frame state: %dd\n", s->frame_state);
        /* State machine to accumulate an entire frame:
         * Suppose your frame is:
         *   Start = 0x01
         *   Byte2 = length (bao nhieu byte)
         *   Next (length) bytes = payload
         *   1 byte checksum
         *   End marker = 0x0A
         *  We have: [Start][Length][Data...][Checksum][End]
         */
        switch (s->frame_state)
        {
        case FRAME_IDLE:
            if (byte == 0x01)
            {
                s->frame_buffer[0] = byte; /* store start */
                s->frame_index = 1;
                s->frame_state = FRAME_GOT_START;
                printf("SPI_Write_DR - IDLE.\n");
            }
            /* else ignore until we see 0x01 */
            break;

        case FRAME_GOT_START:
            /* Next byte is the length */
            s->frame_buffer[s->frame_index++] = byte;
            s->expected_len = byte; /* store length from the firmware */
            s->frame_state = FRAME_GOT_LENGTH;
            printf("SPI_Write_DR - GOT START.\n");
            break;

        case FRAME_GOT_LENGTH:
            /* Next 'expected_len' bytes is the payload */
            s->frame_buffer[s->frame_index++] = byte;
            if ((s->frame_index) >= (2 + s->expected_len))
            {
                /* We have: [Start][Length][Data...] => time for checksum */
                s->frame_state = FRAME_WAIT_FOR_CHECKSUM;
                printf("SPI_Write_DR - WAIT CHECKSUM.\n");
            }
            break;

        case FRAME_WAIT_FOR_CHECKSUM:
            s->frame_buffer[s->frame_index++] = byte;
            /* after this comes the end marker */
            s->frame_state = FRAME_WAIT_FOR_END;
            printf("SPI_Write_DR - WAIT END.\n");
            break;

        case FRAME_WAIT_FOR_END:
            s->frame_buffer[s->frame_index++] = byte;
            if (byte == 0x0A)
            {
                /* We now have a full frame:
                 * total_size = 2 + expected_len + 1 + 1
                 * i.e. start, length, data, checksum, end
                 */
                int total_size = 2 + s->expected_len + 1 + 1;
                if (s->frame_index == total_size)
                {
                    /* We got the correct number of bytes. Send them. */
                    if (qemu_chr_fe_backend_connected(&s->chr))
                    {
                        int ret = qemu_chr_fe_write_all(&s->chr,
                                                        s->frame_buffer,
                                                        total_size);
                        if (ret < 0)
                        {
                            printf("Error: Failed to send data over TCP.\n");
                        }
                        else
                        {
                            printf("Frame successfully sent over TCP: ");
                            for (int i = 0; i < s->frame_index; i++)
                            {
                                printf("%02X ", s->frame_buffer[i]); // In dữ liệu dưới dạng hex
                            }
                            printf("\n");

                            // s->tx_rx_mode = 1; // Switch to read mode after sending
                        }
                    }
                    else
                    {
                        printf("TCP Backend not connected.\n");
                    }

                    // Clear the buffer after sending
                    memset(s->frame_buffer, 0, sizeof(s->frame_buffer));
                    s->frame_index = 0;
                    s->expected_len = 0;
                }
                else
                {
                    printf("Warning: Frame length mismatch. ");
                    printf("Expected %d, got %d bytes.\n", total_size, s->frame_index);
                }
            }
            else
            {
                printf("Warning: Expected end marker 0x0A but got 0x%02X.\n", byte);
                printf("Not Sent Frame : ");
                for (int i = 0; i < s->frame_index; i++)
                {
                    printf("%02X ", s->frame_buffer[i]); // In dữ liệu dưới dạng hex
                }
                printf("\n");
            }

            /* Reset to IDLE for next frame no matter what. */
            s->frame_index = 0;
            // s->expected_len = 0;
            s->frame_state = FRAME_IDLE;
            break;

        default:
            /* Fallback: reset if we get confused. */
            s->frame_index = 0;
            s->expected_len = 0;
            s->frame_state = FRAME_IDLE;
            printf("SPI_Write_DR - DEFAULT .\n");
            break;
        }

        // Thực hiện truyền SPI
        stm32f2xx_spi_transfer(s);

        return;
    case STM_SPI_CRCPR:
        qemu_log_mask(LOG_UNIMP, "%s: CRC is not implemented\n", __func__);
        return;
    case STM_SPI_RXCRCR:
        qemu_log_mask(LOG_GUEST_ERROR, "%s: Read only register: "
                                       "0x%" HWADDR_PRIx "\n",
                      __func__, addr);
        return;
    case STM_SPI_TXCRCR:
        qemu_log_mask(LOG_GUEST_ERROR, "%s: Read only register: "
                                       "0x%" HWADDR_PRIx "\n",
                      __func__, addr);
        return;
    case STM_SPI_I2SCFGR:
        qemu_log_mask(LOG_UNIMP, "%s: "
                                 "I2S is not implemented\n",
                      __func__);
        return;
    case STM_SPI_I2SPR:
        qemu_log_mask(LOG_UNIMP, "%s: "
                                 "I2S is not implemented\n",
                      __func__);
        return;
    default:
        qemu_log_mask(LOG_GUEST_ERROR,
                      "%s: Bad offset 0x%" HWADDR_PRIx "\n", __func__, addr);
    }
    // }
    // else
    // {
    //     printf("SPI write is disabled, tx_rx_mode is not set to TX_MODE.\n");
    // }
}

static const MemoryRegionOps stm32f2xx_spi_ops = {
    .read = stm32f2xx_spi_read,
    .write = stm32f2xx_spi_write,
    .endianness = DEVICE_NATIVE_ENDIAN,
};

static const VMStateDescription vmstate_stm32f2xx_spi = {
    .name = TYPE_STM32F2XX_SPI,
    .version_id = 1,
    .minimum_version_id = 1,
    .fields = (const VMStateField[]){
        VMSTATE_UINT32(spi_cr1, STM32F2XXSPIState),
        VMSTATE_UINT32(spi_cr2, STM32F2XXSPIState),
        VMSTATE_UINT32(spi_sr, STM32F2XXSPIState),
        VMSTATE_UINT32(spi_dr, STM32F2XXSPIState),
        VMSTATE_UINT32(spi_crcpr, STM32F2XXSPIState),
        VMSTATE_UINT32(spi_rxcrcr, STM32F2XXSPIState),
        VMSTATE_UINT32(spi_txcrcr, STM32F2XXSPIState),
        VMSTATE_UINT32(spi_i2scfgr, STM32F2XXSPIState),
        VMSTATE_UINT32(spi_i2spr, STM32F2XXSPIState),
        VMSTATE_END_OF_LIST()}};

static void stm32f2xx_spi_init(Object *obj)
{
    STM32F2XXSPIState *s = STM32F2XX_SPI(obj);
    DeviceState *dev = DEVICE(obj);

    memory_region_init_io(&s->mmio, obj, &stm32f2xx_spi_ops, s,
                          TYPE_STM32F2XX_SPI, 0x400);
    sysbus_init_mmio(SYS_BUS_DEVICE(obj), &s->mmio);

    sysbus_init_irq(SYS_BUS_DEVICE(obj), &s->irq);

    s->ssi = ssi_create_bus(dev, "ssi");

    // Tìm chardev được truyền từ dòng lệnh
    Chardev *chr = qemu_chr_find("spi_uart_tcp");
    if (!chr)
    {
        fprintf(stderr, "Error: Failed to find chardev 'spi_uart_tcp'\n");
        return;
    }

    // Connect the SPI frontend with the found chardev backend
    if (qemu_chr_fe_init(&s->chr, chr, NULL) < 0)
    {
        fprintf(stderr, "Error: Failed to initialize UART frontend\n");
        return;
    }

    printf("SPI UART frontend successfully initialized via TCP!\n");
}

static void stm32f2xx_spi_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);

    device_class_set_legacy_reset(dc, stm32f2xx_spi_reset);
    dc->vmsd = &vmstate_stm32f2xx_spi;
}

static const TypeInfo stm32f2xx_spi_info = {
    .name = TYPE_STM32F2XX_SPI,
    .parent = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(STM32F2XXSPIState),
    .instance_init = stm32f2xx_spi_init,
    .class_init = stm32f2xx_spi_class_init,
};

static void stm32f2xx_spi_register_types(void)
{
    type_register_static(&stm32f2xx_spi_info);
}

type_init(stm32f2xx_spi_register_types)
