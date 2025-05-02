/*==============================================================================
 *   Copyright (C) 2016-2017 Texas Instruments Incorporated
 *
 *   All rights reserved. Property of Texas Instruments Incorporated.
 *   Restricted rights to use, duplicate or disclose this code are
 *   granted through contract.
 *
 *   The program may not be used without the written permission
 *   of Texas Instruments Incorporated or against the terms and conditions
 *   stipulated in the agreement under which this program has been
 *   supplied.
==============================================================================*/

/**
 * file Can_44_TCAN4x5x_MCan.h
 *
 * brief MCAN interface header
 */

/*******************************************************************************
 *                      MISRA-C VIOLATIONS
 ******************************************************************************/

#ifndef CAN_44_TCAN4x5x_MCAN_H
#define CAN_44_TCAN4x5x_MCAN_H

/*******************************************************************************
 *                      INCLUDE FILES
 ******************************************************************************/
#include "Std_Types.h"
#include "Can_44_TCAN4x5x.h"
/*******************************************************************************
 *                      EXPORTED CONSTANTS and MACROS
 ******************************************************************************/
#define CAN_44_TCAN4x5x_VENDOR_ID_MCAN_H   44u
#define CAN_44_TCAN4x5x_MODULE_ID_MCAN_H   80u

#define CAN_44_TCAN4x5x_AR_RELEASE_MAJOR_VERSION_MCAN_H     4u
#define CAN_44_TCAN4x5x_AR_RELEASE_MINOR_VERSION_MCAN_H     2u
#define CAN_44_TCAN4x5x_AR_RELEASE_REVISION_VERSION_MCAN_H  1u

#define CAN_44_TCAN4x5x_SW_MAJOR_VERSION_MCAN_H     2u
#define CAN_44_TCAN4x5x_SW_MINOR_VERSION_MCAN_H     3u
#define CAN_44_TCAN4x5x_SW_PATCH_VERSION_MCAN_H     0u

#define Can_44_TCAN4x5x_DEVCFG_BASE     (0x00000800UL)
#define Can_44_TCAN4x5x_MCAN_BASE       (0x00001000UL)
#define MSG_RAM_BASE_ADDR               (0x00008000UL)
#define MSG_RAM_SIZE                    (0x00002000UL)
#define MSG_RAM_ERASE_VALUE             (0x00000000UL)

/*!<  Definitions for Nominal Bit Timing */
#define TSEG1_MIN   (1UL)
#define TSEG1_MAX   (255UL)
#define TSEG2_MIN   (0UL)
#define TSEG2_MAX   (127UL)
#define SJW_MIN     (0UL)
#define SJW_MAX     (127UL)
#define NBRP_MIN    (0UL)
#define NBRP_MAX    (511UL)

/*!<  Definitions for Data (CAN FD) Bit Timing */
#define DTSEG1_MIN  (0UL)
#define DTSEG1_MAX  (31UL)
#define DTSEG2_MIN  (0UL)
#define DTSEG2_MAX  (15UL)
#define DSJW_MIN    (0UL)
#define DSJW_MAX    (15UL)
#define DBRP_MIN    (0UL)
#define DBRP_MAX    (31UL)

#define Can_44_TCAN4x5x_MCAN_BAUDRATE_MAX       (5000UL)

#define MSG_RAM_STD_MSG_ID_FILTER_ELEM(aId)     \
            (Can_44_TCAN4x5x_MCAN_RODev[0].MSGRAM + \
            STD_ID_FILTER_CONFIG_FLS_START_ADDR)
#define MSG_RAM_EXT_MSG_ID_FILTER_ELEM(aId)     \
            (Can_44_TCAN4x5x_MCAN_RODev[0].MSGRAM + \
            Can_44_TCAN4x5x_MCAN_RWDev[aId].EXTND_ID_FILTER_START_ADDR)
#define MSG_RAM_RX_BUFFER_ELEM(aId)             \
            (Can_44_TCAN4x5x_MCAN_RODev[0].MSGRAM + \
            Can_44_TCAN4x5x_MCAN_RWDev[aId].RX_BUFFER_START_ADDR)
#define MSG_RAM_RX_FIFO0_ELEM(aId)              \
            (Can_44_TCAN4x5x_MCAN_RODev[0].MSGRAM + \
            Can_44_TCAN4x5x_MCAN_RWDev[aId].RX_FIFO0_START_ADDR)
#define MSG_RAM_RX_FIFO1_ELEM(aId)              \
            (Can_44_TCAN4x5x_MCAN_RODev[0].MSGRAM + \
            Can_44_TCAN4x5x_MCAN_RWDev[aId].RX_FIFO1_START_ADDR)
#define MSG_RAM_TX_EVENT_FIFO_ELEM(aid)         \
            (Can_44_TCAN4x5x_MCAN_RODev[0].MSGRAM + \
            Can_44_TCAN4x5x_MCAN_RWDev[aId].TX_EVENT_FIFO_START_ADDR)
#define MSG_RAM_TX_BUFFER_ELEM(aId)             \
            (Can_44_TCAN4x5x_MCAN_RODev[0].MSGRAM + \
            Can_44_TCAN4x5x_MCAN_RWDev[aId].TX_BUFFER_START_ADDR)

#define RX_ELEMENT_R0_XTD                       0x40000000UL
#define RX_ELEMENT_R0_ID                        0x1FFFFFFFUL
#define RX_ELEMENT_R1_FDF                       0x00200000UL
#define RX_ELEMENT_R1_BRS                       0x00100000UL
#define RX_ELEMENT_R1_DLC                       0x000F0000UL
#define RX_ELEMENT_R1_FIDX                      0x7F000000UL

#define RX_ELEMENT_DLC_OFFSET                   (16UL)
#define RX_ELEMENT_STDID_OFFSET                 (18UL)
#define RX_ELEMENT_FDF_OFFSET                   (21UL)
#define RX_ELEMENT_FIDX_OFFSET                  (24UL)

#define TX_ELEMENT_DLC_OFFSET                   (16UL)
#define TX_ELEMENT_STDID_OFFSET                 (18UL)
#define TX_ELEMENT_XTD_OFFSET                   (30UL)
#define TX_ELEMENT_MARKER_OFFSET                (24UL)
#define TX_ELEMENT_EFC_OFFSET                   (23UL)
#define TX_ELEMENT_FDF_OFFSET                   (21UL)
#define TX_ELEMENT_BRS_OFFSET                   (20UL)
#define TX_ELEMENT_DLC_OFFSET                   (16UL)

#define TX_EVENT_FIFO_ELEMENT_R0_XTD            0x40000000UL
#define TX_EVENT_FIFO_ELEMENT_R0_ID             0x1FFFFFFFUL
#define TX_EVENT_FIFO_ELEMENT_R1_DLC            0x000F0000UL
#define TX_EVENT_FIFO_ELEMENT_R1_FDF            0x00200000UL
#define TX_EVENT_FIFO_ELEMENT_R1_BRS            0x00100000UL
#define TX_EVENT_FIFO_ELEMENT_R1_MM             0xFF000000UL

/** \brief
 * Buffer Data Field Size Configuration for CAN FD Messages.
 * RXESC and TXESC registers use one of following values,
 * 000= 8 byte data field
 * 001= 12 byte data field
 * 010= 16 byte data field
 * 011= 20 byte data field
 * 100= 24 byte data field
 * 101= 32 byte data field
 * 110= 48 byte data field
 * 111= 64 byte data field
 */

extern CONST(uint32, CAN_44_TCAN4x5x_CONST)
                                    Can_44_TCAN4x5x_MCAN_ROBufferSizeFD[8];

#define CAN_44_TCAN4x5x_FD_MAX_DATA_SIZE(hw_id) \
(Can_44_TCAN4x5x_MCAN_ROBufferSizeFD[Can_44_TCAN4x5x_ControllerSetting_PC\
[hw_id].CanFdDataBufSize])

/*!<  Definitions for Callbacks */
#define ACTION_CB_DISABLED  (0UL)
#define ACTION_CB_ENABLED   (1UL)

/*!<  Pre-compile time Configuration Parameter definitions */
#define CAN_44_TCAN4x5x_TRANSMIT_CANCELLATION_ENABLED   1


/** \brief
 * Note, only valid if CAN_44_TCAN4x5x_TX_MODE_CONFIG is configured
 * for CAN_44_TCAN4x5x_TX_MODE_FIFO_QUEUE or CAN_44_TCAN4x5x_TX_MODE_MIXED
 */

/** Message RAM Configuration */
/** Note, change the following only if absolutely necessary for application
                                                                requirements */
/** Please refer to MCAN User's Manual for description on how to configure
                                                                Message RAM */
#define STD_ID_FILTER_CONFIG_FLS_START_ADDR             0x0000UL
#define RXFIFO0_CONFIG_OPERATION_MODE                   0x0UL
#define RXFIFO0_CONFIG_WATERMARK                        0x00UL
#define RXFIFO1_CONFIG_OPERATION_MODE                   0x0UL
#define RXFIFO1_CONFIG_WATERMARK                        0x00UL
#define TXEVENTFIFO_CONFIG_WATERMARK                    0x00UL


/** \brief
 * Note, only valid if CAN_44_TCAN4x5x_TX_MODE_CONFIG is configured
 * for CAN_44_TCAN4x5x_TX_MODE_FIFO_QUEUE or CAN_44_TCAN4x5x_TX_MODE_MIXED
 */
#define CAN_44_TCAN4x5x_TX_USE_QUEUE                (1UL)
#define CAN_44_TCAN4x5x_TX_FIFO_CONFIG              CAN_44_TCAN4x5x_TX_USE_QUEUE

/**
 * Use 32 Dedicated Tx Buffers.
 * This value is used to configure TXBC.NDTB
 */
#define CAN_44_TCAN4x5x_TX_NUM_DEDICATED_BUFFERS    32UL

/**
 * Use 32 buffers for TX Queue. This value is used to
 * configure TXBC.TFQS (only when CAN_44_TCAN4x5x_TX_MODE_FIFO_QUEUE
 * or CAN_44_TCAN4x5x_TX_MODE_MIXED are used)
 */
#define CAN_44_TCAN4x5x_TX_NUM_FIFO_QUEUE_BUFFERS   0UL

/**  Rx Buffer / FIFO Element Size Configuration */
#define CAN_RX_BUFFER_ELEMENT_DATA_SIZE(hw_id)        \
                (Can_44_TCAN4x5x_ControllerSetting_PC[hw_id].CanFdDataBufSize)
#define CAN_RX_FIFO0_ELEMENT_DATA_SIZE(hw_id)         \
                (Can_44_TCAN4x5x_ControllerSetting_PC[hw_id].CanFdDataBufSize)
#define CAN_RX_FIFO1_ELEMENT_DATA_SIZE(hw_id)         \
                (Can_44_TCAN4x5x_ControllerSetting_PC[hw_id].CanFdDataBufSize)

/**  Tx Buffer Element Size Configuration */
#define CAN_TX_BUFFER_ELEMENT_DATA_SIZE(hw_id)        \
                (Can_44_TCAN4x5x_ControllerSetting_PC[hw_id].CanFdDataBufSize)

#define MSG_ID_STANDARD                                  (0UL)
#define MSG_ID_EXTENDED                                  (1UL)

/**  Format of CAN Message */
#define MSG_STANDARD_FORMAT                              (0UL)
#define MSG_CANFD_FORMAT                                 (1UL)

#define CAN_44_TCAN4x5x_WRITE                            (uint8)0x61
#define CAN_44_TCAN4x5x_READ                             (uint8)0x41
#define CAN_44_TCAN4x5x_READ_WAIT                        (uint8)0x51

#define MSG_USE_RX_FIFO_0               (1UL)
#define MSG_USE_RX_FIFO_1               (2UL)

/**  Driver APIs execution context */
#define TASK_CONTEXT           0u
#define ISR_CONTEXT            1u
#define MAIN_FUNCTION_CONTEXT  2u

/*******************************************************************************
 *                      EXPORTED STRUCTURES, ENUMS, TYPEDEFS
 ******************************************************************************/

 #define CAN_44_TCAN4x5x_START_SEC_VAR_CLEARED_8
#include "Can_44_TCAN4x5x_MemMap.h"
/**
 *  Variable to hold CanTrcv Device Major Rev Id
 */
extern VAR(uint8, CAN_44_TCAN4x5x_VAR_CLEARED) Can_44_TCAN4x5x_MajorRevId;

#define CAN_44_TCAN4x5x_STOP_SEC_VAR_CLEARED_8
#include "Can_44_TCAN4x5x_MemMap.h"


/** Type of Spi Buffer */
 typedef struct
{
    /** 3 execution context, 20 words of max data */
    uint32 SpiTxBuf[3][20];
    uint32 SpiRxBuf[3][20];
}Can_44_TCAN4x5x_SpiBufType;

typedef uint8 Can_44_TCAN4x5x_MCAN_IRQType;

/**  Type of CAN Message */
typedef uint32 CanMsgIDType;

typedef struct
{
   uint32 ModePinsCfg;     // 0x0800 : Modes of Operation and Pin Configurations 0xC8000468 R/W/U
   uint32 Prescale;        // 0x0804 : Timestamp Prescalar 0x00000002 R/W
   uint32 TestReg;         // 0x0808 : Read and Write Test Registers 0x00000000 R/W
   uint32 EccTdr;          // 0x080C : ECC and TDR Registers 0x00000000 R/W/U
   uint32 RESERVED_00[5];
   uint32 IrqFlags;        // 0x0820 : Interrupt Flags 0x00000000 R (ref nTCAN4550tuIrqFlags)
   uint32 MCANIrqFlags;    // 0x0824 : MCAN Interrupt Flags 0x00000000 R
   uint32 RESERVED_01[2];
   uint32 IrqEnable;       // 0x0830 : Interrupt Enable 0xFFFFFFFF R/W
}Can_44_TCAN4x5x_DevCfg_RegsType;

/**  volatile structure to map the device registers to structure elements */
typedef struct {
    uint32 CREL;
    uint32 ENDN;
    uint32 CUST;
    uint32 DBTP;
    uint32 TEST;
/**<  Loopback mode */
#define TEST_LBCK               (0x00000010UL)
    uint32 RWD;
    uint32 CCCR;
/**<  Initialization */
#define CCCR_INIT               (0x00000001UL)
/**  Configuration Change Enable*/
#define CCCR_CCE                (0x00000002UL)
/**<  Restricted Operation Mode*/
#define CCCR_ASM                (0x00000004UL)
/**<  Clock Stop Request*/
#define CCCR_CSR                (0x00000010UL)
/**<  Bus Monitor Mode */
#define CCCR_MON                (0x00000020UL)
/**<  Test Mode Enable */
#define CCCR_TEST               (0x00000080UL)
/**<  FD Operation Enable */
#define CCCR_FDOE               (0x00000100UL)
/**<  Bit Rate Switch Enable */
#define CCCR_BRSE               (0x00000200UL)
/**<  Transmit Pause */
#define CCCR_TXP                (0x00004000UL)
    uint32 NBTP;
    uint32 TSCC;
#define TSCC_RESET              (0x00000000UL)
    uint32 TSCV;
    uint32 TOCC;
#define TOCC_RESET              (0x00000000UL)
    uint32 TOCV;
    uint32 RSVD_1;
    uint32 RSVD_2;
    uint32 RSVD_3;
    uint32 RSVD_4;
    uint32 ECR;
    uint32 PSR;
    uint32 TDCE;
    uint32 RSVD_5;
    uint32 INTR; /* NOTE: refactor to INTR from IR that is already defined as MACRO in DA14680xx.h */
    /**<  Rx FIFO 0 New Message */
#define IR_RF0N                 (0x00000001UL)
/**<  Rx FIFO 0 Message Lost*/
#define IR_RF0L                 (0x00000008UL)
 /**<  Rx FIFO 1 New Message */
#define IR_RF1N                 (0x00000010UL)
/**<  Rx FIFO 1 Message Lost */
#define IR_RF1L                 (0x00000080UL)
/**<  Tx Completed */
#define IR_TC                   (0x00000200UL)
/**<  Tx Cancellation Finished */
#define IR_TCF                  (0x00000400UL)
/**<  Message stored to Dedicate Rx Buffer */
#define IR_DRX                  (0x00080000UL)
/**<  Bus Off Status */
#define IR_BO                   (0x02000000UL)
/**<  Protocol Error in Arbitration Phase */
#define IR_PEA                  (0x08000000UL)
/**<  Protocol Error in Data Phase */
#define IR_PED                  (0x10000000UL)
/**<  Tx Event FIFO New Entry */
#define IR_TEFN                 (0x00001000UL)
/**<  Tx Event FIFO Element Lost*/
#define IR_TEFL                 (0x00008000UL)
    uint32 IE;
/**<  Enable ALL INterrupts */
#define IE_ENABLE_ALL           (0xFFFFFFFFUL)
/**<  Disable ALL INterrupts */
#define IE_DISABLE_ALL          (0x00000000UL)
    uint32 ILS;
/**<  Rx FIFO 0 New Message Interrupt Line */
#define ILS_RF0NL               (0x00000001UL)
/**<  Rx FIFO 0 Watermark Reached Interrupt Line */
#define ILS_RF0WL    (0x00000002UL)
/**<  Rx FIFO 0 Full Interrupt Line */
#define ILS_RF0FL    (0x00000004UL)
/**<  Rx FIFO 0 Message Lost Interrupt Line*/
#define ILS_RF0LL    (0x00000008UL)
/**<  Rx FIFO 1 New Message Interrupt Line*/
#define ILS_RF1NL    (0x00000010UL)
/**<  Rx FIFO 1 Watermark Reached Interrupt Line */
#define ILS_RF1WL    (0x00000020UL)
/**<  Rx FIFO 1 Full Interrupt Line */
#define ILS_RF1FL    (0x00000040UL)
/**<  Rx FIFO 1 Message Lost Interrupt Line*/
#define ILS_RF1LL    (0x00000080UL)
/**<  High Priority Message Interrupt Line*/
#define ILS_HPML     (0x00000100UL)
/**<  Transmission Completed Interrupt Line*/
#define ILS_TCL      (0x00000200UL)
/**<   Transmission Cancellation Finished Interrupt Line */
#define ILS_TCFL     (0x00000400UL)
/**<  Tx FIFO Empty Interrupt Line */
#define ILS_TFEL     (0x00000800UL)
/**<  Tx Event FIFO New Entry Interrupt Line*/
#define ILS_TEFNL    (0x00001000UL)
 /**<  Tx Event FIFO Watermark Reached Interrupt Line */
#define ILS_TEFWL    (0x00002000UL)
 /**<  Tx Event FIFO Event Full Interrupt Line */
#define ILS_TEFFL    (0x00004000UL)
 /**<  Tx Event FIFO Event Lost Interrupt Line*/
#define ILS_TEFLL    (0x00008000UL)
/**<  Timestamp Wraparound Interrupt Line */
#define ILS_TSWL     (0x00010000UL)
 /**<  Message RAM Access Failure Interrupt Line*/
#define ILS_MRAFL    (0x00020000UL)
/**<  Timeout Occurred Interrput Line */
#define ILS_TOOL     (0x00040000UL)
 /**<  Message stored to Dedicated Rx Buffer Interrupt Line */
#define ILS_DRXL     (0x00080000UL)
/**<  Bit Error Corrected Interrupt Line */
#define ILS_BECL     (0x00100000UL)
/**<  Bit Error Uncorrected Interrupt Line*/
#define ILS_BEUL     (0x00200000UL)
/**<  Error Logging Overflow Interrupt Line*/
#define ILS_ELOL     (0x00400000UL)
 /**<  Error Passive Interrupt Line */
#define ILS_EPL      (0x00800000UL)
/**<  Access to Reserved Address Line */
#define ILS_EWL      (0x01000000UL)
 /**<  Bus_Off Status Interrupt Line */
#define ILS_BOL      (0x02000000UL)
 /**<  Watchdog Interrupt Line */
#define ILS_WDIL     (0x04000000UL)
/**<  Protocol Error in Arbitration Phase Line */
#define ILS_PEAL     (0x08000000UL)
/**<  Protocol Error in Data Phase Line */
#define ILS_PEDL     (0x10000000UL)
 /**<  Access to Reserved Address Line */
#define ILS_ARAL     (0x20000000UL)
    uint32 ILE;
/**<  Enable Interrupt Line 0 */
#define ILE_EINT0               (0x00000001UL)
/**<  Enable Interrupt Line 1 */
#define ILE_EINT1               (0x00000002UL)
    uint32 RSVD_6;
    uint32 RSVD_7;
    uint32 RSVD_8;
    uint32 RSVD_9;
    uint32 RSVD_10;
    uint32 RSVD_11;
    uint32 RSVD_12;
    uint32 RSVD_13;
    uint32 GFC;
    uint32 SIDFC;
/**<  Offset for List Size Standard */
#define SIDFC_LSS_OFFSET        (16UL)
    uint32 XIDFC;
/**<  Offset for List Size Standard */
#define XIDFC_LSS_OFFSET        (16UL)
    uint32 RSVD_14;
    uint32 XIDAM;
    uint32 HPMS;
    uint32 NDAT1;
    uint32 NDAT2;
    uint32 RXF0C;
/**<  Offset for RxFIFO Operation Mode */
#define RXF0C_OPERATION_MODE_OFFSET   (31UL)
/**<  Offset for RxFIFO Watermark*/
#define RXF0C_WATERMARK_OFFSET        (24UL)
/**<  Offset for RxFIFO Size */
#define RXF0C_SIZE_OFFSET             (16UL)
    uint32 RXF0S;
/**<  Rx FIFO 0 Message Lost */
#define RXF0S_RF0L              (0x02000000UL)
/**<  Rx FIFO 0 Full */
#define RXF0S_F0F               (0x01000000UL)
/**<  Rx FIFO 0 Put Index */
#define RXF0S_F0PI              (0x003F0000UL)
/**<  Rx FIFO 0 Get Index */
#define RXF0S_F0GI              (0x00003F00UL)
/**<  Rx FIFO 0 FIFO Level */
#define RXF0S_F0FL              (0x0000007FUL)
    uint32 RXF0A;
/**<  Rx FIFO 0 Acknowledge Index*/
#define RXF0A_F0AI              (0x0000003FUL)
    uint32 RXBC;
    uint32 RXF1C;
 /**<  Offset for RxFIFO Operation Mode */
#define RXF1C_OPERATION_MODE_OFFSET   (31UL)
/**<  Offset for RxFIFO Watermark*/
#define RXF1C_WATERMARK_OFFSET        (24UL)
/**<  Offset for RxFIFO Size */
#define RXF1C_SIZE_OFFSET             (16UL)
    uint32 RXF1S;
/**<  Rx FIFO 1 Message Lost */
#define RXF1S_RF1L              (0x02000000UL)
/**<  Rx FIFO 1 Full */
#define RXF1S_F1F               (0x01000000UL)
 /**<  Rx FIFO 1 Put Index */
#define RXF1S_F1PI              (0x003F0000UL)
/**<  Rx FIFO 1 Get Index */
#define RXF1S_F1GI              (0x00003F00UL)
/**<  Rx FIFO 1 FIFO Level */
#define RXF1S_F1FL              (0x0000007FUL)
    uint32 RXF1A;
    uint32 RXESC;
 /**<  Offset for Rx Buffer Data Field Size */
#define RXESC_BUFFER_SIZE_OFFSET      (uint32)8
/**<  Offset for Rx FIFO1 Data Field Size */
#define RXESC_FIFO1_SIZE_OFFSET       (uint32)4
    uint32 TXBC;
 /**<  Offset for TX Buffer Config Fifo/Queue Mode */
#define TXBC_TFQM_OFFSET        (30UL)
 /**<  Offset for Tx Fifo/Queue Size */
#define TXBC_SIZE_OFFSET        (24UL)
 /**<  Offset for Number of dedicated Tx Buffers */
#define TXBC_NDTB_OFFSET        (16UL)
    uint32 TXFQS;
    uint32 TXESC;
    uint32 TXBRP;
/**<  All Dedicated TX Buffers Pending */
#define TXBRP_ALL_PENDING       (0xFFFFFFFFUL)
    uint32 TXBAR;
    uint32 TXBCR;
/**<  Cancel ALL */
#define TXBCR_CANCEL_ALL        (0xFFFFFFFFUL)
    uint32 TXBTO;
    uint32 TXBCF;
    uint32 TXBTIE;
    uint32 TXBCIE;
/**<  Enable ALL INterrupts */
#define TXBCIE_ENABLE_ALL       (0xFFFFFFFFUL)
    uint32 RSVD_15;
    uint32 RSVD_16;
    uint32 TXEFC;
 /**<  Offset for Tx Event FIFO Watermark */
#define TXEFC_WATERMARK_OFFSET  (24UL)
/**<  Offset for Tx Event FIFO Size */
#define TXEFC_SIZE_OFFSET       (16UL)
    uint32 TXEFS;
/**<  Event Fifo Get Index */
#define TXEFS_REFGI             (0x00001F00UL)
/**<  Event Fifo Fill Level */
#define TXEFS_EFFL              (0x0000003FUL)
    uint32 TXEFA;
} Can_44_TCAN4x5x_MCAN_RegsType;

typedef uint32 CAN_44_TCAN4x5x_StateType;

#define CAN_44_TCAN4x5x_STATE_UNINIT                (0UL)
#define CAN_44_TCAN4x5x_STATE_IDLE                  (1UL)
#define CAN_44_TCAN4x5x_STATE_RUNNING               (2UL)
#define CAN_44_TCAN4x5x_STATE_SLEEP                 (3UL)
#define CAN_44_TCAN4x5x_STATE_BUSOFF                (4UL)
#define CAN_44_TCAN4x5x_STATE_WAKEUP                (5UL)
/**
 * Driver private data (one instance of this structure for each CAN controller)
 */
typedef uint32 Can_44_TCAN4x5x_HwIDType;

/**  Structure Can_44_TCAN4x5x_MCAN_RWDevType*/
typedef struct
{
    CAN_44_TCAN4x5x_StateType state;
    uint32 EXTND_ID_FILTER_START_ADDR;
    uint32 RX_BUFFER_START_ADDR;
    uint32 RX_FIFO0_START_ADDR;
    uint32 RX_FIFO1_START_ADDR;
    uint32 TX_EVENT_FIFO_START_ADDR;
    uint32 TX_BUFFER_START_ADDR;
} Can_44_TCAN4x5x_MCAN_RWDevType;

/**  Structure containing CAN Message Information and pointer to data */
typedef struct {
    Can_44_TCAN4x5x_MCAN_RegsType *REGS;
    uint32 MSGRAM;
    Can_44_TCAN4x5x_DevCfg_RegsType *DEVCFG;
} Can_44_TCAN4x5x_MCAN_RODevType;

/**  Structure containing CAN Message Information and pointer to data */
typedef struct {
     uint32       canID;            /**< ID of the message */
     CanMsgIDType canMsgID;         /**< Msg ID type */
     uint32       canMsgLength;     /**< Data Length */
     uint8*       canMsgDataPtr;    /**< Pointer to data */
     uint32       canMessageFormat;
     uint32       canFilterIndex;
     uint16       canTimeStamp;
} Can_44_TCAN4x5x_MsgInfoType;

#if (CAN_44_TCAN4x5x_PUBLIC_ICOM_SUPPORT == STD_ON)
#define CAN_44_TCAN4x5x_START_SEC_VAR_CLEARED_8
#include "Can_44_TCAN4x5x_MemMap.h"

extern VAR(IcomConfigIdType, CAN_44_TCAN4x5x_VAR_CLEARED)
              Can_44_TCAN4x5x_GComConfigId[CAN_44_TCAN4x5x_NUM_CONTROLLER_MAX];

#define CAN_44_TCAN4x5x_STOP_SEC_VAR_CLEARED_8
#include "Can_44_TCAN4x5x_MemMap.h"
#endif

#if ((CAN_44_TCAN4x5x_PUBLIC_ICOM_SUPPORT == STD_ON) && \
                                 (CAN_44_TCAN4x5x_ICOM_COUNTER_VALUE == STD_ON))
#define CAN_44_TCAN4x5x_START_SEC_VAR_CLEARED_8
#include "Can_44_TCAN4x5x_MemMap.h"

extern VAR(uint8, CAN_44_TCAN4x5x_VAR_CLEARED) Can_44_TCAN4x5x_IcomMsgCounter
    [CAN_44_TCAN4x5x_NUM_CONTROLLER_MAX][CAN_44_TCAN4x5x_ICOM_RX_MESSAGE_TOTAL];

#define CAN_44_TCAN4x5x_STOP_SEC_VAR_CLEARED_8
#include "Can_44_TCAN4x5x_MemMap.h"
#endif

/*******************************************************************************
 *                      EXPORTED FUNCTIONS
 ******************************************************************************/
/** \brief Function Prototypes */
#define CAN_44_TCAN4x5x_START_SEC_CODE_SLOW
#include "Can_44_TCAN4x5x_MemMap.h"

extern FUNC(void, CAN_44_TCAN4x5x_CODE_SLOW) Can_44_TCAN4x5x_MCAN_Init
                    (VAR(Can_44_TCAN4x5x_HwIDType, AUTOMATIC) aId,
                       CONSTP2CONST(Can_44_TCAN4x5x_ConfigType, AUTOMATIC,
                                            CAN_44_TCAN4x5x_APPL_DATA) aConfig);
#define CAN_44_TCAN4x5x_STOP_SEC_CODE_SLOW
#include "Can_44_TCAN4x5x_MemMap.h"

#define CAN_44_TCAN4x5x_START_SEC_CODE_SLOW
#include "Can_44_TCAN4x5x_MemMap.h"
extern FUNC(void, CAN_44_TCAN4x5x_CODE_SLOW) Can_44_TCAN4x5x_MCAN_DeInit
                                (VAR(Can_44_TCAN4x5x_HwIDType, AUTOMATIC) aId);

#define CAN_44_TCAN4x5x_STOP_SEC_CODE_SLOW
#include "Can_44_TCAN4x5x_MemMap.h"


#if(CAN_44_TCAN4x5x_SET_BAUDRATE_API == STD_ON)
#define CAN_44_TCAN4x5x_START_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"
extern FUNC(uint32, CAN_44_TCAN4x5x_CODE_FAST) Can_44_TCAN4x5x_MCAN_SetBaudrate
    (VAR(Can_44_TCAN4x5x_HwIDType, AUTOMATIC) aId,
    CONSTP2CONST(Can_44_TCAN4x5x_ControllerBaudrateConfigType, AUTOMATIC,
                                           CAN_44_TCAN4x5x_APPL_DATA) aConfig);
#define CAN_44_TCAN4x5x_STOP_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"
#endif

#define CAN_44_TCAN4x5x_START_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"
extern FUNC(uint32, CAN_44_TCAN4x5x_CODE_FAST) Can_44_TCAN4x5x_MCAN_Write
                        (VAR(Can_44_TCAN4x5x_HwIDType, AUTOMATIC) aId,
                         P2CONST(Can_PduType, AUTOMATIC,
                                            CAN_44_TCAN4x5x_APPL_DATA) pduInfo,
                         P2CONST(Can_44_TCAN4x5x_TxObjConfigType, AUTOMATIC,
                                            CAN_44_TCAN4x5x_APPL_DATA) TxObj);
#define CAN_44_TCAN4x5x_STOP_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"

#define CAN_44_TCAN4x5x_START_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"
extern FUNC(uint32, CAN_44_TCAN4x5x_CODE_FAST) Can_44_TCAN4x5x_MCAN_SetMode
                        (VAR(Can_44_TCAN4x5x_HwIDType, AUTOMATIC) aId,
                            VAR(CAN_44_TCAN4x5x_StateType, AUTOMATIC) aState);
#define CAN_44_TCAN4x5x_STOP_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"

#define CAN_44_TCAN4x5x_START_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"
extern FUNC(void, CAN_44_TCAN4x5x_CODE_FAST) Can_44_TCAN4x5x_MCAN_ClearTXBRP
                                 (VAR(Can_44_TCAN4x5x_HwIDType, AUTOMATIC) aId);
#define CAN_44_TCAN4x5x_STOP_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"

#define CAN_44_TCAN4x5x_START_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"
extern FUNC(void, CAN_44_TCAN4x5x_CODE_FAST)
        Can_44_TCAN4x5x_MCAN_EnableAllInterrupts
                                 (VAR(Can_44_TCAN4x5x_HwIDType, AUTOMATIC) aId);
#define CAN_44_TCAN4x5x_STOP_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"

#define CAN_44_TCAN4x5x_START_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"
extern FUNC(void, CAN_44_TCAN4x5x_CODE_FAST)
            Can_44_TCAN4x5x_MCAN_DisableAllInterrupts
                                 (VAR(Can_44_TCAN4x5x_HwIDType, AUTOMATIC) aId);
#define CAN_44_TCAN4x5x_STOP_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"

#define CAN_44_TCAN4x5x_START_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"
extern FUNC(void, CAN_44_TCAN4x5x_CODE_FAST)
            Can_44_TCAN4x5x_MCAN_ProcessCanIRQLine0
                                 (VAR(Can_44_TCAN4x5x_HwIDType, AUTOMATIC) aId);
#define CAN_44_TCAN4x5x_STOP_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"

#define CAN_44_TCAN4x5x_START_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"
extern FUNC(void, CAN_44_TCAN4x5x_CODE_FAST)
            Can_44_TCAN4x5x_MCAN_ProcessCanIRQLine1
                                 (VAR(Can_44_TCAN4x5x_HwIDType, AUTOMATIC) aId);
#define CAN_44_TCAN4x5x_STOP_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"

#define CAN_44_TCAN4x5x_START_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"
extern FUNC(void, CAN_44_TCAN4x5x_CODE_FAST) Can_44_TCAN4x5x_MCAN_ProcessCanRx
                                 (VAR(Can_44_TCAN4x5x_HwIDType, AUTOMATIC) aId);
#define CAN_44_TCAN4x5x_STOP_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"

#define CAN_44_TCAN4x5x_START_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"
extern FUNC(void, CAN_44_TCAN4x5x_CODE_FAST) Can_44_TCAN4x5x_MCAN_ProcessCanTx
                                (VAR(Can_44_TCAN4x5x_HwIDType, AUTOMATIC) aId);
#define CAN_44_TCAN4x5x_STOP_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"

#define CAN_44_TCAN4x5x_START_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"
extern FUNC(void, CAN_44_TCAN4x5x_CODE_FAST) Can_44_TCAN4x5x_MCAN_SetCCE
                                (VAR(Can_44_TCAN4x5x_HwIDType, AUTOMATIC) aId);
#define CAN_44_TCAN4x5x_STOP_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"

#define CAN_44_TCAN4x5x_START_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"
extern FUNC(void, CAN_44_TCAN4x5x_CODE_FAST)
            Can_44_TCAN4x5x_MCAN_MessageRAM_Init
                        (VAR(Can_44_TCAN4x5x_HwIDType, AUTOMATIC) aId,
                        P2CONST(Can_44_TCAN4x5x_MsgRamConfigType, AUTOMATIC,
                        CAN_44_TCAN4x5x_APPL_DATA) msg_ram_config);
#define CAN_44_TCAN4x5x_STOP_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"

#define CAN_44_TCAN4x5x_START_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"
extern FUNC(void, CAN_44_TCAN4x5x_CODE_FAST)
            Can_44_TCAN4x5x_MCAN_ConfigureGlobalFilters
                  (VAR(Can_44_TCAN4x5x_HwIDType, AUTOMATIC) aId,
                  CONSTP2CONST(Can_44_TCAN4x5x_RxObjFilterConfigType, AUTOMATIC,
                                      CAN_44_TCAN4x5x_APPL_DATA) rxFilterObjPtr,
                  VAR(uint8, AUTOMATIC) canFilterObjSize);
#define CAN_44_TCAN4x5x_STOP_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"

#define CAN_44_TCAN4x5x_START_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"
extern FUNC(void, CAN_44_TCAN4x5x_CODE_FAST) Can_44_TCAN4x5x_MCAN_BusoffProcess
                                (VAR(Can_44_TCAN4x5x_HwIDType, AUTOMATIC) aId);
#define CAN_44_TCAN4x5x_STOP_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"

#define CAN_44_TCAN4x5x_START_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"

extern FUNC(uint32, CAN_44_TCAN4x5x_CODE_FAST) ConvertToDLC
                                            (VAR(uint32, AUTOMATIC) msgLength);
#define CAN_44_TCAN4x5x_STOP_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"


#define CAN_44_TCAN4x5x_START_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"

extern FUNC(void, CAN_44_TCAN4x5x_CODE_FAST) Can_44_TCAN4x5x_RxNotification
                (VAR(Can_44_TCAN4x5x_HwIDType, AUTOMATIC) aId,
                P2VAR(Can_44_TCAN4x5x_MsgInfoType, AUTOMATIC,
                                        CAN_44_TCAN4x5x_APPL_DATA) MsgInfo);

#define CAN_44_TCAN4x5x_STOP_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"

#define CAN_44_TCAN4x5x_START_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"

extern FUNC(void, CAN_44_TCAN4x5x_CODE_FAST) Can_44_TCAN4x5x_TxNotification
                                    (VAR(PduIdType, AUTOMATIC) swPduHandle);

#define CAN_44_TCAN4x5x_STOP_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"

#endif /** Can_44_TCAN4x5x_MCAN_H */
/******************************************************************************
    Version     Date            Author           Description
    -------     ----            ------           -----------
    1.0.0    11-05-2017       Lakshmi H        Initial Version
	1.0.1    07-08-2017       Lakshmi H        Performed QAC Analysis and
											   justification is given for Misra
											   Rule voilations
    1.0.2    26-09-2017       Lakshmi H        Commenting style is changed to
											   Doxygen style
    1.0.3    17-11-2017       Lakshmi H       TCAN1043 code added and Software
                                               version changed
******************************************************************************/
/*******************************************************************************
 *  End of File: Can_44_TCAN4x5x_MCan.h
 ******************************************************************************/
