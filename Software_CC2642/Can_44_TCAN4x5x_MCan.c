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
 *  \file     Can_44_TCAN4x5x_Mcan.c
 *
 *  \brief    This file contains CAN MCAL driver implementation.
 *
 */
/*******************************************************************************
 *                      MISRA-C VIOLATIONS
 ******************************************************************************/
/**
 * \brief
 * JUSTIFICATION_1  : Dir-1.1(Required) To have a better throughput, keyword
 *                    inline has been used.
 *                    Dir-1.1(Required) Initialization element is required.
 * JUSTIFICATION_2  : Rule-10.8 (Required) A composite expression of
 *                    'essentially unsigned' type (unsigned long) is being cast
 *                    to a different type category, 'signed'. This is required
 *                    to calculate the count of leading zeros
 * JUSTIFICATION_3  : Rule 11.3 (Required)To make passing argument as a
 *                    Compatible pointer type
 * JUSTIFICATION_4  : Rule 11.6 (Required) To make passing argument as a
 *                    Compatible pointer type, Converted to uint32* during
 *                    address assignment.
*/


#ifdef __cplusplus
extern "C"{
#endif
/*******************************************************************************
 *                      INCLUDE FILES
 ******************************************************************************/
#include "stddef.h"
#include "string.h"
#include "Can_44_TCAN4x5x.h"
#include "Spi.h"
#if (CAN_44_TCAN4x5x_DEV_ERROR_DETECT == STD_ON)
#include "Det.h"
#endif
#include "Can_44_TCAN4x5x_MCan.h"
#include "SchM_Can_44_TCAN4x5x.h"
#include "EcuM.h"
#include "CanIf_Cbk.h"
#include "WdgMgr.h"
/*******************************************************************************
 *                      EXPORTED CONSTANTS and MACROS
 ******************************************************************************/
#define CAN_44_TCAN4x5x_VENDOR_ID_MCAN_C   44u
#define CAN_44_TCAN4x5x_MODULE_ID_MCAN_C   80u

#define CAN_44_TCAN4x5x_AR_RELEASE_MAJOR_VERSION_MCAN_C     4u
#define CAN_44_TCAN4x5x_AR_RELEASE_MINOR_VERSION_MCAN_C     2u
#define CAN_44_TCAN4x5x_AR_RELEASE_REVISION_VERSION_MCAN_C  1u

#define CAN_44_TCAN4x5x_SW_MAJOR_VERSION_MCAN_C     2u
#define CAN_44_TCAN4x5x_SW_MINOR_VERSION_MCAN_C     3u
#define CAN_44_TCAN4x5x_SW_PATCH_VERSION_MCAN_C     0u

/*******************************************************************************
 *                      LOCAL CONSTANTS and MACROS
 ******************************************************************************/
#define Can_44_TCAN4x5x_MCAN_REGS(id)       (Can_44_TCAN4x5x_MCAN_RODev[0].REGS)
#define Can_44_TCAN4x5x_DEVCFG_REGS(id)     (Can_44_TCAN4x5x_MCAN_RODev[0].DEVCFG)
#define Can_44_TCAN4x5x_MCAN_MAX_HW_ID      CAN_44_TCAN4x5x_NUM_CONTROLLER_MAX

#ifdef __TCAN4x5x_SPI_READ_BACK_ENABLED__
#define AppUtils_printf     printf
#endif

// '1' to include logging, '0' to remove it
#if 0
    #define DETLOG_CAN_44_TCAN4x5x( ... ) Det_RbLog(CAN_44_TCAN4x5x_MODULE_ID, ... )
#else
    #define DETLOG_CAN_44_TCAN4x5x( ... )
#endif

/*******************************************************************************
 *                      GLOBAL CONSTANTS
 ******************************************************************************/

/*******************************************************************************
 *                      GLOBAL VARIABLES
 ******************************************************************************/
#if (CAN_44_TCAN4x5x_PUBLIC_ICOM_SUPPORT == STD_ON)
#define CAN_44_TCAN4x5x_START_SEC_VAR_CLEARED_8
#include "Can_44_TCAN4x5x_MemMap.h"
/** Variable - Can_44_TCAN4x5x_IcomWakeup */
VAR(uint8, CAN_44_TCAN4x5x_VAR_CLEARED)
                Can_44_TCAN4x5x_IcomWakeup[CAN_44_TCAN4x5x_NUM_CONTROLLER_MAX];

#define CAN_44_TCAN4x5x_STOP_SEC_VAR_CLEARED_8
#include "Can_44_TCAN4x5x_MemMap.h"
#endif

#if (CAN_44_TCAN4x5x_PUBLIC_ICOM_SUPPORT == STD_ON)
#define CAN_44_TCAN4x5x_START_SEC_VAR_CLEARED_8
#include "Can_44_TCAN4x5x_MemMap.h"
/** Variable - Can_44_TCAN4x5x_GComConfigId */
VAR(IcomConfigIdType, CAN_44_TCAN4x5x_VAR_CLEARED)
            Can_44_TCAN4x5x_GComConfigId[CAN_44_TCAN4x5x_NUM_CONTROLLER_MAX];

#define CAN_44_TCAN4x5x_STOP_SEC_VAR_CLEARED_8
#include "Can_44_TCAN4x5x_MemMap.h"
#endif

#define CAN_44_TCAN4x5x_START_SEC_VAR_CLEARED_32
#include "Can_44_TCAN4x5x_MemMap.h"
/** Variable - Can_44_TCAN4x5x_SpiBuf */
static VAR(Can_44_TCAN4x5x_SpiBufType, CAN_44_TCAN4x5x_VAR_CLEARED)
                Can_44_TCAN4x5x_SpiBuf[CAN_44_TCAN4x5x_NUM_CONTROLLER_MAX];
#define CAN_44_TCAN4x5x_STOP_SEC_VAR_CLEARED_32
#include "Can_44_TCAN4x5x_MemMap.h"

#define CAN_44_TCAN4x5x_START_SEC_VAR_CLEARED_8
#include "Can_44_TCAN4x5x_MemMap.h"
/** Variable - Can_44_TCAN4x5x_ExCntxt */
volatile uint8 Can_44_TCAN4x5x_ExCntxt[CAN_44_TCAN4x5x_NUM_CONTROLLER_MAX];
#define CAN_44_TCAN4x5x_STOP_SEC_VAR_CLEARED_8
#include "Can_44_TCAN4x5x_MemMap.h"

#define CAN_44_TCAN4x5x_START_SEC_VAR_CLEARED_8
#include "Can_44_TCAN4x5x_MemMap.h"
/** Variable - Can_44_TCAN4x5x_ReadCmd */
static VAR(uint8, CAN_44_TCAN4x5x_VAR_CLEARED)
                Can_44_TCAN4x5x_ReadCmd[CAN_44_TCAN4x5x_NUM_CONTROLLER_MAX];

#define CAN_44_TCAN4x5x_STOP_SEC_VAR_CLEARED_8
#include "Can_44_TCAN4x5x_MemMap.h"

#define CAN_44_TCAN4x5x_START_SEC_VAR_CLEARED_UNSPECIFIED
#include "Can_44_TCAN4x5x_MemMap.h"
/** \brief Can_44_TCAN4x5x_DrvConfigPtr to store Driver Configuration*/
static P2CONST(Can_44_TCAN4x5x_ConfigType, CAN_44_TCAN4x5x_VAR_FAST_CLEARED,
                CAN_44_TCAN4x5x_APPL_DATA) Can_44_TCAN4x5x_DrvConfigPtr;

#define CAN_44_TCAN4x5x_STOP_SEC_VAR_CLEARED_UNSPECIFIED
#include "Can_44_TCAN4x5x_MemMap.h"

#if ((CAN_44_TCAN4x5x_PUBLIC_ICOM_SUPPORT == STD_ON) && \
                                 (CAN_44_TCAN4x5x_ICOM_COUNTER_VALUE == STD_ON))
#define CAN_44_TCAN4x5x_START_SEC_VAR_CLEARED_8
#include "Can_44_TCAN4x5x_MemMap.h"
/** Can_44_TCAN4x5x_IcomMsgCounter*/
VAR(uint8, CAN_44_TCAN4x5x_VAR_CLEARED) Can_44_TCAN4x5x_IcomMsgCounter
   [CAN_44_TCAN4x5x_NUM_CONTROLLER_MAX][CAN_44_TCAN4x5x_ICOM_RX_MESSAGE_TOTAL];

#define CAN_44_TCAN4x5x_STOP_SEC_VAR_CLEARED_8
#include "Can_44_TCAN4x5x_MemMap.h"
#endif

#define CAN_44_TCAN4x5x_START_SEC_VAR_CLEARED_8
#include "Can_44_TCAN4x5x_MemMap.h"
/**
 * Variable to hold CanTrcv Device Major Rev Id.
 */
VAR(uint8, CAN_44_TCAN4x5x_VAR_CLEARED) Can_44_TCAN4x5x_MajorRevId;

#define CAN_44_TCAN4x5x_STOP_SEC_VAR_CLEARED_8
#include "Can_44_TCAN4x5x_MemMap.h"

#define CAN_44_TCAN4x5x_START_SEC_VAR_CLEARED_16
#include "Can_44_TCAN4x5x_MemMap.h"
VAR(uint16, CAN_44_TCAN4x5x_VAR_CLEARED) Can_44_TCAN4x5x_CurrentTxTimeStamp[CAN_44_TCAN4x5x_NUM_CONTROLLER_MAX];
#define CAN_44_TCAN4x5x_STOP_SEC_VAR_CLEARED_16
#include "Can_44_TCAN4x5x_MemMap.h"

/*******************************************************************************
 *                      VERSION CHECK
 ******************************************************************************/
#if ((CAN_44_TCAN4x5x_SW_MAJOR_VERSION_MCAN_C != \
                                  CAN_44_TCAN4x5x_SW_MAJOR_VERSION_MCAN_H) || \
     (CAN_44_TCAN4x5x_SW_MINOR_VERSION_MCAN_C != \
                                  CAN_44_TCAN4x5x_SW_MINOR_VERSION_MCAN_H) || \
     (CAN_44_TCAN4x5x_SW_PATCH_VERSION_MCAN_C != \
                                  CAN_44_TCAN4x5x_SW_PATCH_VERSION_MCAN_H))
    #error "The SW version number of Can_44_TCAN4x5x_MCan.c and \
            Can_44_TCAN4x5x_MCan.h are different."
#endif

#if ((CAN_44_TCAN4x5x_SW_MAJOR_VERSION_MCAN_C != \
                                        CAN_44_TCAN4x5x_SW_MAJOR_VERSION) || \
     (CAN_44_TCAN4x5x_SW_MINOR_VERSION_MCAN_C != \
                                        CAN_44_TCAN4x5x_SW_MINOR_VERSION) || \
     (CAN_44_TCAN4x5x_SW_PATCH_VERSION_MCAN_C != \
                                        CAN_44_TCAN4x5x_SW_PATCH_VERSION))
    #error "The SW version number of Can_44_TCAN4x5x_MCan.c and \
            Can_44_TCAN4x5x.h are different."
#endif
/*******************************************************************************
 *                      LOCAL FUNCTION PROTOTYPES
 ******************************************************************************/


 #define CAN_44_TCAN4x5x_START_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"

static FUNC(Std_ReturnType, CAN_44_TCAN4x5x_CODE_FAST) Can_44_TCAN4x5x_VerifySpiRead
(uint8 aId, uint32* address);

#define CAN_44_TCAN4x5x_STOP_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"

#define CAN_44_TCAN4x5x_START_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"

static FUNC(void, CAN_44_TCAN4x5x_CODE_FAST) configFilterMasks
    (VAR(Can_44_TCAN4x5x_HwIDType, AUTOMATIC) aId,
    P2CONST(Can_44_TCAN4x5x_RxObjFilterConfigType, AUTOMATIC,
    CAN_44_TCAN4x5x_APPL_DATA) filterObjPtr,
    VAR(uint32, AUTOMATIC) filterConfig);

#define CAN_44_TCAN4x5x_STOP_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"

#if (CAN_44_TCAN4x5x_PUBLIC_ICOM_SUPPORT == STD_ON)
#define CAN_44_TCAN4x5x_START_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"

static FUNC(Std_ReturnType, CAN_44_TCAN4x5x_CODE_FAST)
Can_44_TCAN4x5x_ProcessIcomRx(VAR(Can_44_TCAN4x5x_HwIDType, AUTOMATIC) aId,
                 P2CONST(Can_44_TCAN4x5x_MsgInfoType, AUTOMATIC,
                                        CAN_44_TCAN4x5x_APPL_DATA) MsgInfo);

#define CAN_44_TCAN4x5x_STOP_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"
#endif

#define CAN_44_TCAN4x5x_START_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"
static FUNC(Std_ReturnType, CAN_44_TCAN4x5x_CODE_FAST) Can_44_TCAN4x5x_SpiRead
(uint8 aId, uint32* address, uint8 word_count);
#define CAN_44_TCAN4x5x_STOP_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"

#define CAN_44_TCAN4x5x_START_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"
static FUNC(Std_ReturnType, CAN_44_TCAN4x5x_CODE_FAST) Can_44_TCAN4x5x_SpiWrite
(uint8 aId, uint32* address, uint8 word_count);
#define CAN_44_TCAN4x5x_STOP_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"

#define CAN_44_TCAN4x5x_START_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"

static Std_ReturnType Can_44_TCAN4x5x_SpiRW(uint8 aId, uint32 address,
                                            uint8 word_count, uint8 cmd);

#define CAN_44_TCAN4x5x_STOP_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"

#define CAN_44_TCAN4x5x_START_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"
static FUNC(sint8, CAN_44_TCAN4x5x_CODE_FAST)GetCntLeadZero
                                            (VAR(uint32, AUTOMATIC) value);
#define CAN_44_TCAN4x5x_STOP_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"

#ifdef CAN_44_TCAN4x5x_ENABLE_GPIO_INTERRUPT_CONTROL
#define CAN_44_TCAN4x5x_START_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"
static FUNC(void, CAN_44_TCAN4x5x_CODE_FAST) Can_44_TCAN4x5x_Gpio_Disable_Intr();
#define CAN_44_TCAN4x5x_STOP_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"
#endif

#ifdef CAN_44_TCAN4x5x_ENABLE_GPIO_INTERRUPT_CONTROL
#define CAN_44_TCAN4x5x_START_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"
static FUNC(void, CAN_44_TCAN4x5x_CODE_FAST) Can_44_TCAN4x5x_Gpio_Enable_Intr();
#define CAN_44_TCAN4x5x_STOP_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"
#endif
/*******************************************************************************
 *                      LOCAL VARIABLES
 ******************************************************************************/
/** \brief Declarations */

#define CAN_44_TCAN4x5x_START_SEC_VAR_FAST_CLEARED_UNSPECIFIED
#include "Can_44_TCAN4x5x_MemMap.h"

/** All values are initialized to zero, so no need to initialize explicitly */
static VAR(Can_44_TCAN4x5x_MCAN_RWDevType, CAN_44_TCAN4x5x_VAR_FAST_CLEARED)
                     Can_44_TCAN4x5x_MCAN_RWDev[Can_44_TCAN4x5x_MCAN_MAX_HW_ID];

#define CAN_44_TCAN4x5x_STOP_SEC_VAR_FAST_CLEARED_UNSPECIFIED
#include "Can_44_TCAN4x5x_MemMap.h"

#define CAN_44_TCAN4x5x_START_SEC_VAR_FAST_CLEARED_UNSPECIFIED
#include "Can_44_TCAN4x5x_MemMap.h"

/** All values are initialized to zero, so no need to initialize explicitly */
static VAR(PduIdType, CAN_44_TCAN4x5x_VAR_FAST_CLEARED)
    Can_44_TCAN4x5x_Mcan_swPduHandle[CAN_44_TCAN4x5x_NUM_CONTROLLER_MAX][32u];

#define CAN_44_TCAN4x5x_STOP_SEC_VAR_FAST_CLEARED_UNSPECIFIED
#include "Can_44_TCAN4x5x_MemMap.h"

#define CAN_44_TCAN4x5x_START_SEC_VAR_FAST_CLEARED_UNSPECIFIED
#include "Can_44_TCAN4x5x_MemMap.h"

/** All values are initialized to zero, so no need to initialize explicitly */
static VAR(uint8, CAN_44_TCAN4x5x_VAR_FAST_CLEARED)
                                        Can_44_TCAN4x5x_Mcan_extFilterIndex;

#define CAN_44_TCAN4x5x_STOP_SEC_VAR_FAST_CLEARED_UNSPECIFIED
#include "Can_44_TCAN4x5x_MemMap.h"

#define CAN_44_TCAN4x5x_START_SEC_VAR_FAST_CLEARED_UNSPECIFIED
#include "Can_44_TCAN4x5x_MemMap.h"

/** All values are initialized to zero, so no need to initialize explicitly */
static VAR(uint8, CAN_44_TCAN4x5x_VAR_FAST_CLEARED)
                                        Can_44_TCAN4x5x_Mcan_stdFilterIndex;

#define CAN_44_TCAN4x5x_STOP_SEC_VAR_FAST_CLEARED_UNSPECIFIED
#include "Can_44_TCAN4x5x_MemMap.h"

#define CAN_44_TCAN4x5x_START_SEC_VAR_FAST_CLEARED_UNSPECIFIED
#include "Can_44_TCAN4x5x_MemMap.h"

static P2CONST(Can_44_TCAN4x5x_ControllerFdBaudrateConfigType,
CAN_44_TCAN4x5x_VAR_FAST_CLEARED, CAN_44_TCAN4x5x_APPL_DATA)
            Can_44_TCAN4x5x_Mcan_FdConfig[CAN_44_TCAN4x5x_NUM_CONTROLLER_MAX];

#define CAN_44_TCAN4x5x_STOP_SEC_VAR_FAST_CLEARED_UNSPECIFIED
#include "Can_44_TCAN4x5x_MemMap.h"

#define CAN_44_TCAN4x5x_START_SEC_CONST_UNSPECIFIED
#include "Can_44_TCAN4x5x_MemMap.h"
static CONST(Can_44_TCAN4x5x_MCAN_RODevType, CAN_44_TCAN4x5x_CONST)
Can_44_TCAN4x5x_MCAN_RODev[1] =
{
    /** MISRA DIR 1.1 VIOLATION: JUSTIFICATION_1 */
    /** MISRA RULE 11.6 VIOLATION: JUSTIFICATION_4 */
    [0] =
    {
        .REGS = (Can_44_TCAN4x5x_MCAN_RegsType *)Can_44_TCAN4x5x_MCAN_BASE,
        .MSGRAM = MSG_RAM_BASE_ADDR,
        .DEVCFG = (Can_44_TCAN4x5x_DevCfg_RegsType *)Can_44_TCAN4x5x_DEVCFG_BASE
    }
};

#define CAN_44_TCAN4x5x_STOP_SEC_CONST_UNSPECIFIED
#include "Can_44_TCAN4x5x_MemMap.h"

#define CAN_44_TCAN4x5x_START_SEC_CONST_32
#include "Can_44_TCAN4x5x_MemMap.h"
CONST(uint32, CAN_44_TCAN4x5x_CONST) Can_44_TCAN4x5x_MCAN_ROBufferSizeFD[8] =
{
    8u, 12u, 16u, 20u, 24u, 32u, 48u, 64u
};
#define CAN_44_TCAN4x5x_STOP_SEC_CONST_32
#include "Can_44_TCAN4x5x_MemMap.h"

#define TXBUFFER_CONFIG_DATA_SIZE(code) \
                                    (Can_44_TCAN4x5x_MCAN_ROBufferSizeFD[code])
#define RXBUFFER_CONFIG_DATA_SIZE(code) \
                                    (Can_44_TCAN4x5x_MCAN_ROBufferSizeFD[code])
/*******************************************************************************
 *                      FUNCTION DEFINITIONS
 ******************************************************************************/
/** \brief Memory map of the CAN driver code */

#define CAN_44_TCAN4x5x_START_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"


/** brief
 *
 * Returns bit position of msb which is set
 *
 * param[in]    val      Value to parse
 *
 * return       Bit position which is set
 */
static FUNC(sint8, CAN_44_TCAN4x5x_CODE_FAST)GetCntLeadZero
(VAR(uint32, AUTOMATIC) value)
{
    VAR(uint32, AUTOMATIC) temp1;
    VAR(uint32, AUTOMATIC) Cnt = (uint32)16;
    VAR(uint32, AUTOMATIC) Num = (uint32)32;
    do
    {
        temp1 = value >> Cnt;
        if (temp1 != (uint32)0)
        {
            Num = Num - Cnt;
            value = temp1;
        }
        Cnt = Cnt >> (uint32)1;
    } while (Cnt != (uint32)0);
    /** MISRA RULE 10.8 VIOLATION: JUSTIFICATION_2*/
    return ((sint8)(Num - value));
}
static FUNC(void, CAN_44_TCAN4x5x_CODE_FAST) getmsb (VAR(uint64, AUTOMATIC) val,
                   P2VAR(sint8, AUTOMATIC, CAN_44_TCAN4x5x_APPL_DATA) bitPosPtr)
{
    VAR(uint32, AUTOMATIC) temp;
    VAR(sint8, AUTOMATIC) bitpos;

    temp = (uint32)(val >> 32u);

    bitpos = (31 - (sint8)GetCntLeadZero(temp));

    if (0 > bitpos)
    {
        /** bit not set in upper word, check lower 32-bits */
        temp = (uint32)(val & 0xFFFFFFFFUL);
        bitpos = (31 - (sint8)GetCntLeadZero(temp));
    }
    else
    {   /** bit is set in upper 32-bit word, so add 32 for 32->63 range. */
        bitpos += 32;
    }

    *bitPosPtr = bitpos;
}
#define CAN_44_TCAN4x5x_STOP_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"

#define CAN_44_TCAN4x5x_START_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"
/** MISRA DIR 1.1 VIOLATION: JUSTIFICATION_1 */
LOCAL_INLINE FUNC(uint32, CAN_44_TCAN4x5x_CODE_FAST)Can_44_TCAN4x5x_MCAN_SetInit
                                  (VAR(Can_44_TCAN4x5x_HwIDType, AUTOMATIC) aId)
{
    VAR(uint32, AUTOMATIC) ret = E_OK;
    VAR(uint32, AUTOMATIC) timeout = CAN_44_TCAN4x5x_INIT_WAIT_TIMEOUT;
    VAR(uint32, AUTOMATIC) CCCRValue = 0x0u;
    P2VAR(uint32, AUTOMATIC, CAN_44_TCAN4x5x_APPL_DATA)
                            CCCR_ptr = &(Can_44_TCAN4x5x_MCAN_REGS(aId)->CCCR);
    P2VAR(uint32, AUTOMATIC, CAN_44_TCAN4x5x_APPL_DATA) spi_tx_buf_ptr;
    P2VAR(uint32, AUTOMATIC, CAN_44_TCAN4x5x_APPL_DATA) spi_rx_buf_ptr;

    spi_tx_buf_ptr =
        &Can_44_TCAN4x5x_SpiBuf[aId].SpiTxBuf[Can_44_TCAN4x5x_ExCntxt[aId]][1];
    spi_rx_buf_ptr =
        &Can_44_TCAN4x5x_SpiBuf[aId].SpiRxBuf[Can_44_TCAN4x5x_ExCntxt[aId]][1];
    /** Read, modify & write the register */
    (void)Can_44_TCAN4x5x_SpiRead((uint8)aId, CCCR_ptr, (uint8)1);
    CCCRValue = spi_rx_buf_ptr[0];
    if((CCCRValue & CCCR_INIT) == 0UL)
    {
        /** Enter Critical Section */
        SchM_Enter_Can_44_TCAN4x5x_Exclusive_Area_03();

        (void)Can_44_TCAN4x5x_SpiRead((uint8)aId, (uint32*)0x0800, (uint8)1);

        /** Clear MCAN_CONFIG bit at 0x0800 */
        /** MISRA RULE 11.6 VIOLATION: JUSTIFICATION_4 */
        #if( CAN_44_TCAN4x5x_TARGET_DEVICE == CAN_44_TCAN4x5x_TCAN1043)
            if(Can_44_TCAN4x5x_MajorRevId == (uint8)0x2){
                spi_tx_buf_ptr[0] = (spi_rx_buf_ptr[0]|( (uint32)1 << (uint32)6) );
                spi_tx_buf_ptr[0] &= ~( (uint32)1 << (uint32)7);
               (void)Can_44_TCAN4x5x_SpiWrite((uint8)aId, (uint32*)0x0800, (uint8)1);
        }
        #endif
        spi_tx_buf_ptr[0] = (spi_rx_buf_ptr[0] & (~((uint32)0x00000020)));

        #if( CAN_44_TCAN4x5x_TARGET_DEVICE == CAN_44_TCAN4x5x_TCAN1043)

             if(Can_44_TCAN4x5x_MajorRevId != (uint8)0x2) {
                /* BIT[7:6] =11 to Enter Trcv to Test Mode for Silicon Ver1.0 & Ver1.1*/
                spi_tx_buf_ptr[0] |= ((uint32)0x000000C0);
             }
             else{
                /* BIT[21] = 1 TEST_MODE_EN; Test mode enable. for Silicon Ver2.0 */
                spi_tx_buf_ptr[0] |= ((uint32)1 << (uint32)21);
             }

        #endif
        /** MISRA RULE 11.6 VIOLATION: JUSTIFICATION_4 */
        (void)Can_44_TCAN4x5x_SpiWrite((uint8)aId, (uint32*)0x0800, (uint8)1);
        /** Exit Critical Section */
        SchM_Exit_Can_44_TCAN4x5x_Exclusive_Area_03();

        /** Set INIT bit in CCR register */
        spi_tx_buf_ptr[0] = (CCCRValue | CCCR_INIT);
        (void)Can_44_TCAN4x5x_SpiWrite((uint8)aId, CCCR_ptr, (uint8)1);
        while (timeout > 0UL)
        {
            (void)Can_44_TCAN4x5x_SpiRead((uint8)aId, CCCR_ptr, (uint8)1);
            CCCRValue = spi_rx_buf_ptr[0];
            if ((CCCRValue & CCCR_INIT) != 0UL)
            {
                break;
            }
            timeout--;
            #if (CAN_44_TCAN4x5x_DEV_ERROR_DETECT == STD_ON)
            if (timeout == 0UL)
            {
                (void)Det_ReportError(CAN_44_TCAN4x5x_MODULE_ID,
                                    CAN_44_TCAN4x5x_INSTANCE_ID,
                                    Can_44_TCAN4x5x_ApiId,
                                    CAN_44_TCAN4x5x_E_INIT_FAILED);
                ret = (uint32)E_NOT_OK;
            }
            #endif // (CAN_44_TCAN4x5x_DEV_ERROR_DETECT == STD_ON)
        }
    }
    return ret;
}
#define CAN_44_TCAN4x5x_STOP_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"

#define CAN_44_TCAN4x5x_START_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"
/** MISRA DIR 1.1 VIOLATION: JUSTIFICATION_1 */
LOCAL_INLINE FUNC(void, CAN_44_TCAN4x5x_CODE_FAST)Can_44_TCAN4x5x_MCAN_ResetInit
                                 (VAR(Can_44_TCAN4x5x_HwIDType, AUTOMATIC) aId)
{
    VAR(uint32, AUTOMATIC) timeout = CAN_44_TCAN4x5x_INIT_WAIT_TIMEOUT;
    VAR(uint32, AUTOMATIC) ResetVal = (uint32)0x0;
    P2VAR(uint32, AUTOMATIC, CAN_44_TCAN4x5x_APPL_DATA)
                            CCCR_ptr = &(Can_44_TCAN4x5x_MCAN_REGS(aId)->CCCR);
    P2VAR(uint32, AUTOMATIC, CAN_44_TCAN4x5x_APPL_DATA) spi_tx_buf_ptr;
    P2VAR(uint32, AUTOMATIC, CAN_44_TCAN4x5x_APPL_DATA) spi_rx_buf_ptr;

    spi_tx_buf_ptr =
        &Can_44_TCAN4x5x_SpiBuf[aId].SpiTxBuf[Can_44_TCAN4x5x_ExCntxt[aId]][1];
    spi_rx_buf_ptr =
        &Can_44_TCAN4x5x_SpiBuf[aId].SpiRxBuf[Can_44_TCAN4x5x_ExCntxt[aId]][1];

    /** Clear INIT bit */
    (void)Can_44_TCAN4x5x_SpiRead((uint8)aId, CCCR_ptr, (uint8)1);
    spi_tx_buf_ptr[0] = spi_rx_buf_ptr[0] & (~CCCR_INIT);
    (void)Can_44_TCAN4x5x_SpiWrite((uint8)aId, CCCR_ptr, (uint8)1);
    while (timeout > (uint32)0)
    {
        (void)Can_44_TCAN4x5x_SpiRead((uint8)aId, CCCR_ptr, (uint8)1);
        ResetVal =
          Can_44_TCAN4x5x_SpiBuf[aId].SpiRxBuf[Can_44_TCAN4x5x_ExCntxt[aId]][1];
        if((ResetVal & CCCR_INIT) == 0UL)
        {
            break;
        }
        timeout--;
    }
    if (timeout == (uint32)0)
    {
        #if (CAN_44_TCAN4x5x_DEV_ERROR_DETECT == STD_ON)
        (void)Det_ReportError(CAN_44_TCAN4x5x_MODULE_ID,
                            CAN_44_TCAN4x5x_INSTANCE_ID,
                            Can_44_TCAN4x5x_ApiId,
                            CAN_44_TCAN4x5x_E_INIT_FAILED);
        #endif // (CAN_44_TCAN4x5x_DEV_ERROR_DETECT == STD_ON)
    }
    else
    {
        #if (!defined(__TCAN4x5x_MCAN_INT_LOOPBACK_ENABLED__) && \
        !defined(__TCAN4x5x_MCAN_EXT_LOOPBACK_ENABLED__) \
             && !defined(__TCAN4x5x_MCAN_FPGA_BOARD__))
        /** Enter Critical Section */
        SchM_Enter_Can_44_TCAN4x5x_Exclusive_Area_04();
        /** Set MCAN_CONFIG bit */

        /** MISRA RULE 11.6 VIOLATION: JUSTIFICATION_4 */
        (void)Can_44_TCAN4x5x_SpiRead((uint8)aId, (uint32*)0x0800, (uint8)1);
        spi_tx_buf_ptr[0] = (spi_rx_buf_ptr[0] | 0x00000020u);

        #if( CAN_44_TCAN4x5x_TARGET_DEVICE == CAN_44_TCAN4x5x_TCAN1043)
             if(Can_44_TCAN4x5x_MajorRevId != (uint8)0x2) {
                /* BIT[7:6] =11 to Enter Trcv to Test Mode for Silicon Ver1.0 & Ver1.1*/
                spi_tx_buf_ptr[0] |= ((uint32)0x000000C0);
             }
             else{
                /* BIT[21] = 1 TEST_MODE_EN; Test mode enable. for Silicon Ver2.0 */
                spi_tx_buf_ptr[0] |= ((uint32)1 << (uint32)21);
             }
        #endif
        /** MISRA RULE 11.6 VIOLATION: JUSTIFICATION_4 */
        (void)Can_44_TCAN4x5x_SpiWrite((uint8)aId, (uint32*)0x0800, (uint8)1);

        #if( CAN_44_TCAN4x5x_TARGET_DEVICE == CAN_44_TCAN4x5x_TCAN1043)
            if(Can_44_TCAN4x5x_MajorRevId == (uint8)0x2){
                spi_tx_buf_ptr[0] =  (spi_tx_buf_ptr[0] | ((uint32)1 << (uint32)7));
                spi_tx_buf_ptr[0] &= ~((uint32)1 << (uint32)6);
                (void)Can_44_TCAN4x5x_SpiWrite((uint8)aId, (uint32*)0x0800, (uint8)1);
            }
        #endif
        /** Exit Critical Section */
        SchM_Exit_Can_44_TCAN4x5x_Exclusive_Area_04();
        #endif
    }
}
#define CAN_44_TCAN4x5x_STOP_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"

#define CAN_44_TCAN4x5x_START_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"
 FUNC(void, CAN_44_TCAN4x5x_CODE_FAST) Can_44_TCAN4x5x_MCAN_SetCCE (VAR
                                    (Can_44_TCAN4x5x_HwIDType, AUTOMATIC) aId)
{
    if (Can_44_TCAN4x5x_MCAN_SetMode(aId, CAN_44_TCAN4x5x_STATE_IDLE) ==
                                                                (uint32)E_OK)
    {
        /** Read, modify & write the register */
        if (Can_44_TCAN4x5x_SpiRead((uint8)aId,
            &(Can_44_TCAN4x5x_MCAN_REGS(aId)->CCCR), (uint8)1) == E_OK)
        {
            Can_44_TCAN4x5x_SpiBuf[aId].
            SpiTxBuf[Can_44_TCAN4x5x_ExCntxt[aId]][1] =
            Can_44_TCAN4x5x_SpiBuf[aId].
            SpiRxBuf[Can_44_TCAN4x5x_ExCntxt[aId]][1] | CCCR_CCE;
            (void)Can_44_TCAN4x5x_SpiWrite((uint8)aId,
            &(Can_44_TCAN4x5x_MCAN_REGS(aId)->CCCR), (uint8)1);
        }
    }
    else
    {
        #if (CAN_44_TCAN4x5x_DEV_ERROR_DETECT == STD_ON)
        (void)Det_ReportError(CAN_44_TCAN4x5x_MODULE_ID,
                              CAN_44_TCAN4x5x_INSTANCE_ID,
                              Can_44_TCAN4x5x_ApiId,
                              CAN_44_TCAN4x5x_E_INIT_FAILED);
        #endif // (CAN_44_TCAN4x5x_DEV_ERROR_DETECT == STD_ON) */
    }
}
#define CAN_44_TCAN4x5x_STOP_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"

#define CAN_44_TCAN4x5x_START_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"
/** MISRA DIR 1.1 VIOLATION: JUSTIFICATION_1 */
LOCAL_INLINE FUNC(void, CAN_44_TCAN4x5x_CODE_FAST) Can_44_TCAN4x5x_MCAN_ResetCCE
                                  (VAR(Can_44_TCAN4x5x_HwIDType, AUTOMATIC) aId)
{
    VAR(uint32, AUTOMATIC) ret = E_OK;

    ret = Can_44_TCAN4x5x_MCAN_SetInit(aId);

    if (ret == (uint32)E_OK)
    {
        /** Read, modify & write the register */
        if (Can_44_TCAN4x5x_SpiRead((uint8)aId,
        &(Can_44_TCAN4x5x_MCAN_REGS(aId)->CCCR), (uint8)1) == E_OK)
        {
            Can_44_TCAN4x5x_SpiBuf[aId].
            SpiTxBuf[Can_44_TCAN4x5x_ExCntxt[aId]][1] =
            Can_44_TCAN4x5x_SpiBuf[aId].
            SpiRxBuf[Can_44_TCAN4x5x_ExCntxt[aId]][1] & (~CCCR_CCE);
            (void)Can_44_TCAN4x5x_SpiWrite((uint8)aId,
            &(Can_44_TCAN4x5x_MCAN_REGS(aId)->CCCR), (uint8)1);
            }
    }
}
#define CAN_44_TCAN4x5x_STOP_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"

#define CAN_44_TCAN4x5x_START_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"
FUNC(void, CAN_44_TCAN4x5x_CODE_FAST) Can_44_TCAN4x5x_MCAN_ClearTXBRP
                                  (VAR(Can_44_TCAN4x5x_HwIDType, AUTOMATIC) aId)
{
    /** Request transmit cancellation */
    if (Can_44_TCAN4x5x_SpiRead((uint8)aId,
    &(Can_44_TCAN4x5x_MCAN_REGS(aId)->TXBRP), (uint8)1) == E_OK)
    {
        VAR(uint32, AUTOMATIC) TXBRPValue;

        TXBRPValue = Can_44_TCAN4x5x_SpiBuf[aId].
        SpiRxBuf[Can_44_TCAN4x5x_ExCntxt[aId]][1];

        if(TXBRPValue != 0UL)
        {
            Can_44_TCAN4x5x_SpiBuf[aId].
            SpiTxBuf[Can_44_TCAN4x5x_ExCntxt[aId]][1] = TXBRPValue;
            (void)Can_44_TCAN4x5x_SpiWrite((uint8)aId,
            &(Can_44_TCAN4x5x_MCAN_REGS(aId)->TXBCR), (uint8)1);
        }
    }
}
#define CAN_44_TCAN4x5x_STOP_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"

#define CAN_44_TCAN4x5x_START_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"
FUNC(void, CAN_44_TCAN4x5x_CODE_FAST) Can_44_TCAN4x5x_MCAN_EnableAllInterrupts
                                  (VAR(Can_44_TCAN4x5x_HwIDType, AUTOMATIC) aId)
{
    VAR(uint32, AUTOMATIC) IE_value = 0x0u;
    P2VAR(uint32, AUTOMATIC, CAN_44_TCAN4x5x_APPL_DATA) spi_tx_buf_ptr;
    /** If RX/TX processing is true, then interrupt mode is supported */
    if(Can_44_TCAN4x5x_ControllerSetting_PC[aId].CanRxProcessing ==
                                                    CAN_44_TCAN4x5x_INTERRUPT)
    {
        IE_value = (IR_DRX | IR_RF0N | IR_RF1N | IR_RF0L | IR_RF1L);
    }
    if(Can_44_TCAN4x5x_ControllerSetting_PC[aId].CanTxProcessing ==
                                                    CAN_44_TCAN4x5x_INTERRUPT)
    {
        IE_value |= (IR_TEFN | IR_TEFL);
    }
    if(Can_44_TCAN4x5x_ControllerSetting_PC[aId].CanBusoffProcessing ==
                                                    CAN_44_TCAN4x5x_INTERRUPT)
    {
        IE_value |= (IR_BO);
    }
    spi_tx_buf_ptr = &Can_44_TCAN4x5x_SpiBuf[aId].
                     SpiTxBuf[Can_44_TCAN4x5x_ExCntxt[aId]][1];
    spi_tx_buf_ptr[0] = IE_value;

#if (RBD_PK_PCB == RBD_PK_PCB_F005VC0733_B1ANCHOR_SVW) || (RBD_PK_PCB == RBD_PK_PCB_F005VC0732_B5ANCHOR)
    /** ILS register value */
    spi_tx_buf_ptr[1] = 0x3FFFFFFFu;        //Sett all bits to 1 (except for reserved) to assign it to m_can_int1

    /** ILE register value */
    spi_tx_buf_ptr[2] = ILE_EINT1;
#else
#warning "lih1cl: FIXME rework this register to make it configurable"
    spi_tx_buf_ptr[1] = 0u;

    /** ILE register value */
    spi_tx_buf_ptr[2] = ILE_EINT0;
#endif
    /** Write all three register values */
    (void)Can_44_TCAN4x5x_SpiWrite((uint8)aId,
                            &(Can_44_TCAN4x5x_MCAN_REGS(aId)->IE), (uint8)3);
}
#define CAN_44_TCAN4x5x_STOP_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"

#define CAN_44_TCAN4x5x_START_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"
FUNC(void, CAN_44_TCAN4x5x_CODE_FAST) Can_44_TCAN4x5x_MCAN_DisableAllInterrupts
                                  (VAR(Can_44_TCAN4x5x_HwIDType, AUTOMATIC) aId)
{
    Can_44_TCAN4x5x_SpiBuf[aId].SpiTxBuf[Can_44_TCAN4x5x_ExCntxt[aId]][1] =
                                                        IE_DISABLE_ALL;
    (void)Can_44_TCAN4x5x_SpiWrite((uint8)aId,
                            &(Can_44_TCAN4x5x_MCAN_REGS(aId)->IE), (uint8)1);
}
#define CAN_44_TCAN4x5x_STOP_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"

#define CAN_44_TCAN4x5x_START_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"
/** brief
 *
 * This API is used to change the state of MCAN Driver / Controller
 *
 * param[in]    aId      Index of the MCAN Controller
 * param[in]    state    new state to set for CAN Driver / Controller
 *
 * return       void
 */

FUNC(uint32, CAN_44_TCAN4x5x_CODE_FAST) Can_44_TCAN4x5x_MCAN_SetMode
 (VAR(Can_44_TCAN4x5x_HwIDType, AUTOMATIC) aId,
                              VAR(CAN_44_TCAN4x5x_StateType, AUTOMATIC) aState)
{
    VAR(uint32, AUTOMATIC) retval = E_OK;
    VAR(uint32, AUTOMATIC) TXBRPValue = 0x0u;
    P2VAR(uint32, AUTOMATIC, CAN_44_TCAN4x5x_APPL_DATA) spi_tx_buf_ptr;
    P2VAR(uint32, AUTOMATIC, CAN_44_TCAN4x5x_APPL_DATA) spi_rx_buf_ptr;
    P2VAR(uint32, AUTOMATIC, CAN_44_TCAN4x5x_APPL_DATA) CCCR_ptr;
    (void)TXBRPValue; //unused if CAN_44_TCAN4x5x_TRANSMIT_CANCELLATION_ENABLED == 0

    if ( (aId >= Can_44_TCAN4x5x_MCAN_MAX_HW_ID) ) {
         retval = (uint32)E_NOT_OK;
    }
    else
    {
        CCCR_ptr = &(Can_44_TCAN4x5x_MCAN_REGS(aId)->CCCR);
        spi_tx_buf_ptr = &Can_44_TCAN4x5x_SpiBuf[aId].
                            SpiTxBuf[Can_44_TCAN4x5x_ExCntxt[aId]][1];
        spi_rx_buf_ptr = &Can_44_TCAN4x5x_SpiBuf[aId].
                            SpiRxBuf[Can_44_TCAN4x5x_ExCntxt[aId]][1];
        switch (aState)
        {
        case CAN_44_TCAN4x5x_STATE_IDLE:
            Can_44_TCAN4x5x_MCAN_RWDev[aId].state = CAN_44_TCAN4x5x_STATE_IDLE;
            /**  If Transmission in Progress
            Request Transmit Cancellation and
            Disable Interrupts                */
            #if (CAN_44_TCAN4x5x_TRANSMIT_CANCELLATION_ENABLED == 1)
            (void)Can_44_TCAN4x5x_SpiRead((uint8)aId,
                            &(Can_44_TCAN4x5x_MCAN_REGS(aId)->TXBRP), (uint8)1);
			(void)Can_44_TCAN4x5x_VerifySpiRead((uint8)aId, &(Can_44_TCAN4x5x_MCAN_REGS(aId)->TXBRP));
            TXBRPValue = spi_rx_buf_ptr[0];
            if(TXBRPValue != 0u)
            {
                spi_tx_buf_ptr[0] = TXBRPValue;
                (void)Can_44_TCAN4x5x_SpiWrite((uint8)aId,
                            &(Can_44_TCAN4x5x_MCAN_REGS(aId)->TXBCR), (uint8)1);
                retval = (uint32)CAN_BUSY;
            }
            else
            #endif
            {
                /** Set CCCR.INIT */
                retval = Can_44_TCAN4x5x_MCAN_SetInit(aId);
                /** Set CCE bit. Read, modify & write the register */
                if (Can_44_TCAN4x5x_SpiRead((uint8)aId, CCCR_ptr, (uint8)1) ==
                                                                          E_OK)
                {
                    spi_tx_buf_ptr[0] = spi_rx_buf_ptr[0] | (CCCR_CCE);
                    spi_tx_buf_ptr[0] &= ~(CCCR_CSR);
                    (void)Can_44_TCAN4x5x_SpiWrite((uint8)aId, CCCR_ptr,
                                                                    (uint8)1);
                }

            }
            break;

        case CAN_44_TCAN4x5x_STATE_RUNNING:
            Can_44_TCAN4x5x_MCAN_RWDev[aId].state =
                                                CAN_44_TCAN4x5x_STATE_RUNNING;
            Can_44_TCAN4x5x_MCAN_EnableAllInterrupts(aId);
            Can_44_TCAN4x5x_MCAN_ResetCCE(aId);         /** Reset CCCR.CCE */
            Can_44_TCAN4x5x_MCAN_ResetInit(aId);        /** Reset CCCR.INIT */
            break;

        case CAN_44_TCAN4x5x_STATE_SLEEP:
            Can_44_TCAN4x5x_MCAN_RWDev[aId].state = CAN_44_TCAN4x5x_STATE_SLEEP;

            /** Enable CSR */
            /** Read, modify & write the register */
            if (Can_44_TCAN4x5x_SpiRead((uint8)aId, CCCR_ptr, (uint8)1) == E_OK)
            {
                spi_tx_buf_ptr[0] = spi_rx_buf_ptr[0] | (CCCR_CSR);
                (void)Can_44_TCAN4x5x_SpiWrite((uint8)aId, CCCR_ptr, (uint8)1);
            }
            if(retval == E_OK)
            {
                Can_44_TCAN4x5x_MCAN_DisableAllInterrupts(aId);
            }
        break;

        case CAN_44_TCAN4x5x_STATE_WAKEUP:
            Can_44_TCAN4x5x_MCAN_RWDev[aId].state =
                                                CAN_44_TCAN4x5x_STATE_WAKEUP;
            /** Disable CSR */
            /** Read, modify & write the register */
            if (Can_44_TCAN4x5x_SpiRead((uint8)aId, CCCR_ptr, (uint8)1) == E_OK)
            {
                spi_tx_buf_ptr[0] = spi_rx_buf_ptr[0] & (~CCCR_CSR);
                (void)Can_44_TCAN4x5x_SpiWrite((uint8)aId, CCCR_ptr, (uint8)1);
            }
            break;

        case CAN_44_TCAN4x5x_STATE_UNINIT:
            Can_44_TCAN4x5x_MCAN_RWDev[aId].state =
                                                CAN_44_TCAN4x5x_STATE_UNINIT;
            Can_44_TCAN4x5x_DriverState = (uint8)0;
            break;

        default:
            retval = (uint32)E_NOT_OK;
            break;
        }
    }
    return retval;
}
#define CAN_44_TCAN4x5x_STOP_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"

#define CAN_44_TCAN4x5x_START_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"
/** brief
 *
 * Configures filter elements for RX messages
 *
 * param[in]    Id      Index of the MCAN Controller
 * param[in]    Config  Pointer to configuration structure
 *
 * return       void
 */

FUNC(void, CAN_44_TCAN4x5x_CODE_FAST)Can_44_TCAN4x5x_MCAN_ConfigureGlobalFilters
        (VAR(Can_44_TCAN4x5x_HwIDType, AUTOMATIC) aId,
        CONSTP2CONST(Can_44_TCAN4x5x_RxObjFilterConfigType, AUTOMATIC,
        CAN_44_TCAN4x5x_APPL_DATA) rxFilterObjPtr,
        VAR(uint8, AUTOMATIC) canFilterObjSize)
{
    VAR(uint32, AUTOMATIC) idx;
    /** Reject all non-matching frames */
    Can_44_TCAN4x5x_SpiBuf[aId].SpiTxBuf[Can_44_TCAN4x5x_ExCntxt[aId]][1] =
                                                              0x3Fu;
    (void)Can_44_TCAN4x5x_SpiWrite((uint8)aId,
                            &(Can_44_TCAN4x5x_MCAN_REGS(aId)->GFC), (uint8)1);
    Can_44_TCAN4x5x_Mcan_stdFilterIndex = 0U;
    Can_44_TCAN4x5x_Mcan_extFilterIndex = 0U;
    for(idx = 0UL; idx < canFilterObjSize;idx++)
    {
        /** Configure Filter masks */
        configFilterMasks(aId, &rxFilterObjPtr[idx],
                rxFilterObjPtr[idx].CanMessageBufferType);
    }
}

/** brief
 *
 * Convert message length to CAN DLC code.
 *
 * param[in]    canLength      Message length
 *
 * return       void
 */
#define CAN_44_TCAN4x5x_STOP_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"

#define CAN_44_TCAN4x5x_START_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"
FUNC(uint32, CAN_44_TCAN4x5x_CODE_FAST) ConvertToDLC
                                        (VAR(uint32, AUTOMATIC) msgLength)
{
    VAR(uint32, AUTOMATIC) MsgDLC;

    if (msgLength <= 8UL)
    {
        MsgDLC = msgLength;
    }
    else if (msgLength <= 12UL)
    {
        MsgDLC = 9UL;
    }
    else if (msgLength <= 16UL)
    {
        MsgDLC = 10UL;
    }
    else if (msgLength <= 20UL)
    {
        MsgDLC = 11UL;
    }
    else if (msgLength <= 24UL)
    {
        MsgDLC = 12UL;
    }
    else if (msgLength <= 32UL)
    {
        MsgDLC = 13UL;
    }
    else if (msgLength <= 48UL)
    {
        MsgDLC = 14UL;
    }
    else
    {
        MsgDLC = 15UL;
    }

    return MsgDLC;
}
#define CAN_44_TCAN4x5x_STOP_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"

#define CAN_44_TCAN4x5x_START_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"
/** brief
 *
 * Processing of Transmission Complete events in Event FIFO
 *
 * param[in]    Id      Index of the MCAN Controller
 *
 * return       void
 */

static FUNC(uint32, CAN_44_TCAN4x5x_CODE_FAST) ConvertCanDLC
                                                (VAR(uint32, AUTOMATIC) canDLC)
{
    VAR(uint32, AUTOMATIC) MsgLength;
    switch (canDLC)
    {
    case (uint8)9:
        MsgLength = (uint32)12;
        break;
    case (uint8)10:
        MsgLength = (uint32)16;
        break;
    case (uint8)11:
        MsgLength = (uint32)20;
        break;
    case (uint8)12:
        MsgLength = (uint32)24;
        break;
    case (uint8)13:
        MsgLength = (uint32)32;
           break;
    case (uint8)14:
        MsgLength = (uint32)48;
        break;
    case (uint8)15:
        MsgLength = (uint32)64;
        break;
    default:
        MsgLength = canDLC;
        break;
    }
    return MsgLength;
}
#define CAN_44_TCAN4x5x_STOP_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"

/** brief
 *
 * Sets Baud Rate parameters of MCAN Controller
 *
 * param[in]    Id      Index of the MCAN Controller
 * param[in]    Config  Pointer to configuration structure
 *
 * return      E_OK              Baudrate set successfully
 * return      ERR_INVALID_PARAMS  Invalid Controller index or
 *                                  baudrate parameters
 *
 */
#if(CAN_44_TCAN4x5x_SET_BAUDRATE_API == STD_ON)
#define CAN_44_TCAN4x5x_START_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"
FUNC(uint32, CAN_44_TCAN4x5x_CODE_FAST) Can_44_TCAN4x5x_MCAN_SetBaudrate
    (VAR(Can_44_TCAN4x5x_HwIDType, AUTOMATIC) aId,
    CONSTP2CONST(Can_44_TCAN4x5x_ControllerBaudrateConfigType, AUTOMATIC,
                                           CAN_44_TCAN4x5x_APPL_DATA) aConfig)
{
    VAR(uint32, AUTOMATIC) ntseg1;
    VAR(uint32, AUTOMATIC) ntseg2;
    VAR(uint32, AUTOMATIC) nsjw;
    VAR(uint32, AUTOMATIC) nbrp;
    VAR(uint32, AUTOMATIC) nbtp;
    VAR(uint32, AUTOMATIC) retval = E_OK;
    VAR(uint32, AUTOMATIC) dtseg1;
    VAR(uint32, AUTOMATIC) dtseg2;
    VAR(uint32, AUTOMATIC) dsjw;
    VAR(uint32, AUTOMATIC) dbrp;
    VAR(uint32, AUTOMATIC) dbtp = (uint32)0x0;
    VAR(uint32, AUTOMATIC) tdc;
    P2VAR(uint32, AUTOMATIC, CAN_44_TCAN4x5x_APPL_DATA) spi_tx_buf_ptr;
    P2VAR(uint32, AUTOMATIC, CAN_44_TCAN4x5x_APPL_DATA) spi_rx_buf_ptr;
    P2VAR(uint32, AUTOMATIC, CAN_44_TCAN4x5x_APPL_DATA)
                            CCCR_ptr = &(Can_44_TCAN4x5x_MCAN_REGS(aId)->CCCR);
    spi_tx_buf_ptr = &Can_44_TCAN4x5x_SpiBuf[aId].
                                     SpiTxBuf[Can_44_TCAN4x5x_ExCntxt[aId]][1];
    spi_rx_buf_ptr = &Can_44_TCAN4x5x_SpiBuf[aId].
                                    SpiRxBuf[Can_44_TCAN4x5x_ExCntxt[aId]][1];

    /** Calculate timequantas */
    ntseg1 = ((uint32)(aConfig->CanControllerPropSeg) +
                                        (uint32)(aConfig->CanControllerSeg1));
    ntseg2 = aConfig->CanControllerSeg2;
    nsjw = aConfig->CanControllerSyncJumpWidth;
    nbrp = aConfig->CanControllerBRP;

    #if (CAN_44_TCAN4x5x_DEV_ERROR_DETECT == STD_ON)
    if ( ((ntseg1 < TSEG1_MIN) || (ntseg1 > TSEG1_MAX)) ||
         (ntseg2 > TSEG2_MAX) ||
         (nsjw > SJW_MAX) ||
         (nbrp > NBRP_MAX))
    {
             (void)Det_ReportError(CAN_44_TCAN4x5x_MODULE_ID,
                                   CAN_44_TCAN4x5x_INSTANCE_ID,
                                   Can_44_TCAN4x5x_ApiId,
                            CAN_44_TCAN4x5x_E_INVALID_NOMINAL_BITTIMING_PARAM);
             retval = (uint32)E_NOT_OK;
    }
    else
    #endif // (CAN_44_TCAN4x5x_DEV_ERROR_DETECT == STD_ON)
    {
        if(Can_44_TCAN4x5x_ControllerSetting_PC[aId].CanFDBaudRateEnabled ==
                                                                    (uint8)1)
        {
            /** Save pointer to use in WRITE function later */
            Can_44_TCAN4x5x_Mcan_FdConfig[aId] =
                                        aConfig->CanControllerFdBaudrateConf;
            /** Calculate timequantas */
            dtseg1 = ((uint32)(Can_44_TCAN4x5x_Mcan_FdConfig[aId]->
                        CanControllerDataPropSeg) +
                        (uint32)(Can_44_TCAN4x5x_Mcan_FdConfig[aId]->
                        CanControllerDataSeg1));
            dtseg2 = Can_44_TCAN4x5x_Mcan_FdConfig[aId]->CanControllerDataSeg2;
            dsjw =
            Can_44_TCAN4x5x_Mcan_FdConfig[aId]->CanControllerDataSyncJumpWidth;
            dbrp = Can_44_TCAN4x5x_Mcan_FdConfig[aId]->CanFDControllerBRP;
            if (Can_44_TCAN4x5x_Mcan_FdConfig[aId]->
                        CanControllerDataTrcvDelayCompensationOffset != 0U)
            {
                tdc = 1U;
            }
            else
            {
                tdc = 0U;
            }
            #if (CAN_44_TCAN4x5x_DEV_ERROR_DETECT == STD_ON)
            if ( ((dtseg1 < TSEG1_MIN) || (dtseg1 > TSEG1_MAX)) ||
                (dtseg2 > TSEG2_MAX) ||
                (dsjw > SJW_MAX) ||
                (dbrp > NBRP_MAX))
            {
                    (void)Det_ReportError(CAN_44_TCAN4x5x_MODULE_ID,
                                        CAN_44_TCAN4x5x_INSTANCE_ID,
                                        Can_44_TCAN4x5x_ApiId,
                               CAN_44_TCAN4x5x_E_INVALID_DATA_BITTIMING_PARAM);
                    retval = (uint32)E_NOT_OK;
            }
            #endif // (CAN_44_TCAN4x5x_DEV_ERROR_DETECT == STD_ON)
            if (Can_44_TCAN4x5x_Mcan_FdConfig[aId]->
                                CanControllerDataTxBitRateSwitch == (uint8)1)
            {
                /** Set DBTP value for CAN FD configuration */
                dbtp = (((uint32)dsjw) | (dbrp << 16UL) |
                    (((uint32)dtseg1) << 8UL) | (((uint32)dtseg2) << 4UL));
                dbtp |= (tdc << 23UL);
            }
        }

        /** Reset timestamp, timeout registers */
        // if(Can_44_TCAN4x5x_ControllerSetting_PC[aId].CanFDBaudRateEnabled == (uint8)1)
        // {
        //     spi_tx_buf_ptr[0] = 0x00000002u; /** External time stamp generator */
        // }
        // else
        // {
        //     spi_tx_buf_ptr[0] = TSCC_RESET;
        // }
        spi_tx_buf_ptr[0] = CAN_44_TCAN4x5x_XTAL_FREQUENCY / 8 / CAN_44_TCAN4x5x_TIMESTAMP_FREQUENCY;
        (void)Can_44_TCAN4x5x_SpiWrite((uint8)aId, &(Can_44_TCAN4x5x_DEVCFG_REGS(aId)->Prescale), (uint8)1); // set Timestamp Prescalar (address = h0804)
        spi_tx_buf_ptr[0] = 0x00000002u; /** time stamp using above defined clock */
        (void)Can_44_TCAN4x5x_SpiWrite((uint8)aId,
                        &(Can_44_TCAN4x5x_MCAN_REGS(aId)->TSCC), (uint8)1);
        spi_tx_buf_ptr[0] = TOCC_RESET;
        (void)Can_44_TCAN4x5x_SpiWrite((uint8)aId,
                        &(Can_44_TCAN4x5x_MCAN_REGS(aId)->TOCC), (uint8)1);

        if(Can_44_TCAN4x5x_ControllerSetting_PC[aId].CanFDBaudRateEnabled ==
                                                                     (uint8)1)
        {
            if (Can_44_TCAN4x5x_SpiRead((uint8)aId, CCCR_ptr, (uint8)1) == E_OK)
            {
                spi_tx_buf_ptr[0] = spi_rx_buf_ptr[0] | (CCCR_FDOE);
                (void)Can_44_TCAN4x5x_SpiWrite((uint8)aId, CCCR_ptr, (uint8)1);
            }
            if (Can_44_TCAN4x5x_Mcan_FdConfig[aId]->
                                    CanControllerDataTxBitRateSwitch == STD_ON)
            {
                if (Can_44_TCAN4x5x_SpiRead((uint8)aId, CCCR_ptr, (uint8)1)
                                                                        == E_OK)
                {
                    spi_tx_buf_ptr[0] = spi_rx_buf_ptr[0] | (CCCR_BRSE);
                    (void)Can_44_TCAN4x5x_SpiWrite((uint8)aId, CCCR_ptr,
                                                                      (uint8)1);
                }
            }
        }
        #if (defined(__TCAN4x5x_MCAN_INT_LOOPBACK_ENABLED__) || \
                        defined(__TCAN4x5x_MCAN_EXT_LOOPBACK_ENABLED__))
        /** Set TEST bit */
        if (Can_44_TCAN4x5x_SpiRead((uint8)aId, CCCR_ptr, (uint8)1) == E_OK)
        {
            spi_tx_buf_ptr[0] = spi_rx_buf_ptr[0] | (CCCR_TEST);
            (void)Can_44_TCAN4x5x_SpiWrite((uint8)aId, CCCR_ptr, (uint8)1);
        }
        /** Enable loop back */
        spi_tx_buf_ptr[0] = TEST_LBCK;
        (void)Can_44_TCAN4x5x_SpiWrite((uint8)aId,
                        &(Can_44_TCAN4x5x_MCAN_REGS(aId)->TEST), (uint8)1);

        #ifdef __TCAN4x5x_MCAN_INT_LOOPBACK_ENABLED__
        if (Can_44_TCAN4x5x_SpiRead((uint8)aId, CCCR_ptr, (uint8)1) == E_OK)
        {
            spi_tx_buf_ptr[0] = spi_rx_buf_ptr[0] | (CCCR_MON);
            (void)Can_44_TCAN4x5x_SpiWrite((uint8)aId, CCCR_ptr, (uint8)1);
        }
        #endif
        #endif
        nbtp = (((uint32)nsjw) << 25UL) | (nbrp << 16UL)|
            (((uint32)ntseg1) << 8UL) | ((uint32)ntseg2);
        spi_tx_buf_ptr[0] = nbtp;
        (void)Can_44_TCAN4x5x_SpiWrite((uint8)aId,
                        &(Can_44_TCAN4x5x_MCAN_REGS(aId)->NBTP), (uint8)1);

        if(Can_44_TCAN4x5x_ControllerSetting_PC[aId].CanFDBaudRateEnabled ==
                                                                    (uint8)1)
        {
            if (Can_44_TCAN4x5x_Mcan_FdConfig[aId]->
                            CanControllerDataTrcvDelayCompensationOffset != 0U)
            {
                spi_tx_buf_ptr[0] =
                (((uint32)(Can_44_TCAN4x5x_Mcan_FdConfig[aId]->
                                  CanControllerDataTrcvDelayCompensationOffset)
                - 1UL) << 8UL);
                (void)Can_44_TCAN4x5x_SpiWrite((uint8)aId,
                            &(Can_44_TCAN4x5x_MCAN_REGS(aId)->TDCE), (uint8)1);
            }
            spi_tx_buf_ptr[0] = dbtp;
            (void)Can_44_TCAN4x5x_SpiWrite((uint8)aId,
                            &(Can_44_TCAN4x5x_MCAN_REGS(aId)->DBTP), (uint8)1);
        }
    }
    return retval;
}
#define CAN_44_TCAN4x5x_STOP_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"
#endif

/** brief
 *
 * Processes messages received in RX buffer
 *
 * param[in]    Id      Index of the MCAN Controller
 *
 * return       void
 */

#define CAN_44_TCAN4x5x_START_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"
static FUNC(void, CAN_44_TCAN4x5x_CODE_FAST)Can_44_TCAN4x5x_MCAN_ProcessRxBuffer
                                (VAR(Can_44_TCAN4x5x_HwIDType, AUTOMATIC) aId)
{
    VAR(uint64, AUTOMATIC) ndat_val;
    VAR(sint8, AUTOMATIC) buf_index;
    VAR(uint32, AUTOMATIC) idx;
    VAR(uint32, AUTOMATIC) dataIdx;
    VAR(uint32, AUTOMATIC) canDLC;
    VAR(uint32, AUTOMATIC) MsgLength;
    P2VAR(uint32, AUTOMATIC, CAN_44_TCAN4x5x_APPL_DATA) rxElem;
    P2VAR(uint32, AUTOMATIC, CAN_44_TCAN4x5x_APPL_DATA) rxElemAdrs;
    VAR(Can_44_TCAN4x5x_MsgInfoType, AUTOMATIC) RxMsg;
    VAR(uint8, AUTOMATIC) rxBuf[64];
    P2VAR(uint32, AUTOMATIC, CAN_44_TCAN4x5x_APPL_DATA) spi_tx_buf_ptr;
    P2VAR(uint32, AUTOMATIC, CAN_44_TCAN4x5x_APPL_DATA) spi_rx_buf_ptr;

    spi_tx_buf_ptr = &Can_44_TCAN4x5x_SpiBuf[aId].
                                    SpiTxBuf[Can_44_TCAN4x5x_ExCntxt[aId]][1];
    spi_rx_buf_ptr = &Can_44_TCAN4x5x_SpiBuf[aId].
                                    SpiRxBuf[Can_44_TCAN4x5x_ExCntxt[aId]][1];

    (void)Can_44_TCAN4x5x_SpiRead((uint8)aId,
                            &(Can_44_TCAN4x5x_MCAN_REGS(aId)->NDAT1), (uint8)2);
    (void)Can_44_TCAN4x5x_VerifySpiRead((uint8)aId, &(Can_44_TCAN4x5x_MCAN_REGS(aId)->NDAT1));
    ndat_val = (uint64)spi_rx_buf_ptr[1];
    ndat_val = ndat_val << 32UL;
    ndat_val = spi_rx_buf_ptr[0] | ndat_val;

    while (ndat_val != 0ULL)
    {
        /** New data exists and Callback is not pending for this packet
                                                        (no loop-around) */
        getmsb(ndat_val, &buf_index);    /** Check which buffer has new data */
        /** MISRA RULE 11.6 VIOLATION: JUSTIFICATION_4 */
        rxElemAdrs = (((uint32*)MSG_RAM_RX_BUFFER_ELEM(aId)) +
        ((2U + (RXBUFFER_CONFIG_DATA_SIZE
        (Can_44_TCAN4x5x_ControllerSetting_PC[aId].CanFdDataBufSize) >> 2U)) *
                                                        ((uint8)buf_index)));
        /** Read first two words from MCAN RX buffer */
        (void)Can_44_TCAN4x5x_SpiRead((uint8)aId, rxElemAdrs, 2u);
        rxElem = spi_rx_buf_ptr;

        canDLC = (rxElem[1UL] & RX_ELEMENT_R1_DLC) >> RX_ELEMENT_DLC_OFFSET;
        MsgLength = ConvertCanDLC(canDLC);

        RxMsg.canTimeStamp = rxElem[1UL];

        if ((rxElem[0UL] & RX_ELEMENT_R0_XTD) == 0UL)
        {
            /** Standard Identifier */
            RxMsg.canID =
            ((rxElem[0UL] & RX_ELEMENT_R0_ID) >> RX_ELEMENT_STDID_OFFSET);
            RxMsg.canMsgID = MSG_ID_STANDARD;
        }
        else
        {
            RxMsg.canID = (rxElem[0UL] & RX_ELEMENT_R0_ID);
            RxMsg.canMsgID = MSG_ID_EXTENDED;
        }

        RxMsg.canMessageFormat =
            (rxElem[1UL] & RX_ELEMENT_R1_FDF) >> RX_ELEMENT_FDF_OFFSET;

        RxMsg.canFilterIndex =
            (rxElem[1UL] & RX_ELEMENT_R1_FIDX) >> RX_ELEMENT_FIDX_OFFSET;

        RxMsg.canMsgLength = MsgLength;

        if (RxMsg.canMsgLength > 0UL)
        {
            RxMsg.canMsgDataPtr = rxBuf;
        }
        else
        {
            RxMsg.canMsgDataPtr = NULL_PTR;
        }

        /** Convert msg length (bytes) to word */
        if ((MsgLength & 0x03u) != 0u)
        {
            MsgLength = (MsgLength / 4u) + (uint8)1;
        }
        else { MsgLength /= 4u; }
        /** Read the data and store in buffer */
        (void)Can_44_TCAN4x5x_SpiRead((uint8)aId, (rxElemAdrs + (uint32)2),
                                                            (uint8)MsgLength);
        for(idx = 0U; idx < RxMsg.canMsgLength; idx++)
        {
            dataIdx = idx >> 2U;
            rxBuf[idx] =
                (uint8)((rxElem[dataIdx] >> ((idx%4U)*8U)) & 0xFFUL);
        }

        if ((uint8)buf_index < 32U)
        {
            /** Clear New Data bit from NDAT1 */
            spi_tx_buf_ptr[0] = (uint32)(0x1UL << ((uint8)buf_index));
            (void)Can_44_TCAN4x5x_SpiWrite((uint8)aId,
                    &(Can_44_TCAN4x5x_MCAN_REGS(aId)->NDAT1), (uint8)1);
        }
        else
        {
            /** Clear New Data bit from NDAT2 */
            spi_tx_buf_ptr[0] =
                            (uint32)(0x1UL << ((uint8)buf_index - (uint8)32U));
            (void)Can_44_TCAN4x5x_SpiWrite((uint8)aId,
                            &(Can_44_TCAN4x5x_MCAN_REGS(aId)->NDAT2), (uint8)1);
        }
        ndat_val &= (~((uint64)0x1U << (uint8)buf_index));

        /** Call notification function */
        Can_44_TCAN4x5x_RxNotification(aId, &RxMsg);
    }
}
#define CAN_44_TCAN4x5x_STOP_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"

#define CAN_44_TCAN4x5x_START_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"
/** brief
 *
 * Processes messages received in RX FIFO 0 or 1
 *
 * param[in]    Id      Index of the MCAN Controller
 *
 * return       void
 */

static FUNC(void, CAN_44_TCAN4x5x_CODE_FAST) Can_44_TCAN4x5x_MCAN_ProcessRxFIFO
(VAR(Can_44_TCAN4x5x_HwIDType, AUTOMATIC) aId, VAR(uint32, AUTOMATIC) rxFifoCfg)
{
    VAR(uint8, AUTOMATIC) fifoLevel;
    VAR(uint32, AUTOMATIC) index;
    VAR(uint32, AUTOMATIC) idx;
    VAR(uint32, AUTOMATIC) dataIdx;
    VAR(uint32, AUTOMATIC) canDLC;
    VAR(uint32, AUTOMATIC) MsgLength;
    P2VAR(uint32, AUTOMATIC, CAN_44_TCAN4x5x_APPL_DATA) rxElem;
    P2VAR(uint32, AUTOMATIC, CAN_44_TCAN4x5x_APPL_DATA) rxElemAdrs;
    VAR(Can_44_TCAN4x5x_MsgInfoType, AUTOMATIC) RxMsg;
    VAR(uint8, AUTOMATIC) rxBuf[64];
    P2VAR(uint32, AUTOMATIC, CAN_44_TCAN4x5x_APPL_DATA) spi_tx_buf_ptr;
    P2VAR(uint32, AUTOMATIC, CAN_44_TCAN4x5x_APPL_DATA) spi_rx_buf_ptr;

    spi_tx_buf_ptr = &Can_44_TCAN4x5x_SpiBuf[aId].
                                    SpiTxBuf[Can_44_TCAN4x5x_ExCntxt[aId]][1];
    spi_rx_buf_ptr = &Can_44_TCAN4x5x_SpiBuf[aId].
                                    SpiRxBuf[Can_44_TCAN4x5x_ExCntxt[aId]][1];

    if (rxFifoCfg == MSG_USE_RX_FIFO_1)
    {
        /** Read message from RX FIFO 1 */
        (void)Can_44_TCAN4x5x_SpiRead((uint8)aId,
                           &(Can_44_TCAN4x5x_MCAN_REGS(aId)->RXF1S), (uint8)1);
		(void)Can_44_TCAN4x5x_VerifySpiRead((uint8)aId, &(Can_44_TCAN4x5x_MCAN_REGS(aId)->RXF1S));
        fifoLevel = (uint8)(spi_rx_buf_ptr[0] & RXF1S_F1FL);
        while (fifoLevel != 0U)
        {
            index = (spi_rx_buf_ptr[0] & RXF1S_F1GI) >> 8UL;
            /** MISRA RULE 11.6 VIOLATION: JUSTIFICATION_4 */
            rxElemAdrs = (((uint32*)MSG_RAM_RX_FIFO1_ELEM(aId)) + ((2UL +
            (RXBUFFER_CONFIG_DATA_SIZE(Can_44_TCAN4x5x_ControllerSetting_PC[aId]
            .CanFdDataBufSize) >> 2U)) * index));
            /** Read first two words from MCAN RX FIFO buffer */
            (void)Can_44_TCAN4x5x_SpiRead((uint8)aId, rxElemAdrs, 2u);
            rxElem = spi_rx_buf_ptr;

            canDLC = (rxElem[1] & RX_ELEMENT_R1_DLC) >> RX_ELEMENT_DLC_OFFSET;
            MsgLength = ConvertCanDLC(canDLC);

            RxMsg.canTimeStamp = rxElem[1];

            if ((rxElem[0UL] & RX_ELEMENT_R0_XTD) == 0UL) {
                /** Standard Identifier */
                RxMsg.canID =
            ((rxElem[0UL] & RX_ELEMENT_R0_ID) >> RX_ELEMENT_STDID_OFFSET);
                RxMsg.canMsgID = MSG_ID_STANDARD;
            } else {
                RxMsg.canID = (rxElem[0UL] & RX_ELEMENT_R0_ID);
                RxMsg.canMsgID = MSG_ID_EXTENDED;
            }

            RxMsg.canMessageFormat
            =  (rxElem[1UL] & RX_ELEMENT_R1_FDF) >> RX_ELEMENT_FDF_OFFSET;

            RxMsg.canFilterIndex =
            (rxElem[1UL] & RX_ELEMENT_R1_FIDX) >> RX_ELEMENT_FIDX_OFFSET;

            RxMsg.canMsgLength = MsgLength;

            if (RxMsg.canMsgLength > 0UL)
            {
                RxMsg.canMsgDataPtr = rxBuf;
            }
            else
            {
                RxMsg.canMsgDataPtr = NULL_PTR;
            }

            /** Convert msg length (bytes) to word */
            if ((MsgLength & 0x03u) != 0u)
            {
                MsgLength = (MsgLength / 4u) + (uint8)1;
            }
            else { MsgLength /= 4u; }
            /** Read the data and store in buffer */
            (void)Can_44_TCAN4x5x_SpiRead((uint8)aId, (rxElemAdrs + (uint32)2),
                                                            (uint8)MsgLength);
            for(idx = 0U; idx < RxMsg.canMsgLength; idx++)
            {
                dataIdx = idx >> 2U;
                rxBuf[idx] =
                (uint8)((rxElem[dataIdx] >> ((idx%4U)*8U)) & 0xFFUL);
            }

            /** Updating index in FIFO0 */
            spi_tx_buf_ptr[0] = (uint32)index;
            (void)Can_44_TCAN4x5x_SpiWrite((uint8)aId,
                        &(Can_44_TCAN4x5x_MCAN_REGS(aId)->RXF1A), (uint8)1);

            /** Call notification function */
            Can_44_TCAN4x5x_RxNotification(aId, &RxMsg);
            fifoLevel--;
            if (fifoLevel != 0u)
            {
                (void)Can_44_TCAN4x5x_SpiRead((uint8)aId,
                        &(Can_44_TCAN4x5x_MCAN_REGS(aId)->RXF1S), (uint8)1);
                (void)Can_44_TCAN4x5x_VerifySpiRead((uint8)aId, &(Can_44_TCAN4x5x_MCAN_REGS(aId)->RXF1S));
            }
        }
    }
    else
    {
        /** Read message from RX FIFO 0 */
        (void)Can_44_TCAN4x5x_SpiRead((uint8)aId,
                        &(Can_44_TCAN4x5x_MCAN_REGS(aId)->RXF0S), (uint8)1);
		(void)Can_44_TCAN4x5x_VerifySpiRead((uint8)aId, &(Can_44_TCAN4x5x_MCAN_REGS(aId)->RXF0S));
        fifoLevel = (uint8)(spi_rx_buf_ptr[0] & RXF0S_F0FL);
        while (fifoLevel != 0U)
        {
            index = (spi_rx_buf_ptr[0] & RXF0S_F0GI) >> 8UL;
            /** MISRA RULE 11.6 VIOLATION: JUSTIFICATION_4 */
            rxElemAdrs = (((uint32*)MSG_RAM_RX_FIFO0_ELEM(aId)) + ((2UL +
            (RXBUFFER_CONFIG_DATA_SIZE(Can_44_TCAN4x5x_ControllerSetting_PC[aId]
            .CanFdDataBufSize) >> 2U)) * index));
            /** Read first two words from MCAN RX FIFO buffer */
            (void)Can_44_TCAN4x5x_SpiRead((uint8)aId, rxElemAdrs, 2u);
            rxElem = spi_rx_buf_ptr;

            canDLC = (rxElem[1] & RX_ELEMENT_R1_DLC) >> RX_ELEMENT_DLC_OFFSET;
            MsgLength = ConvertCanDLC(canDLC);

            RxMsg.canTimeStamp = rxElem[1];

            if ((rxElem[0] & RX_ELEMENT_R0_XTD) == 0UL) {
                /** Standard Identifier */
                RxMsg.canID =
            ((rxElem[0UL] & RX_ELEMENT_R0_ID) >> RX_ELEMENT_STDID_OFFSET);
            RxMsg.canMsgID = MSG_ID_STANDARD;
            } else {
                RxMsg.canID = (rxElem[0UL] & RX_ELEMENT_R0_ID);
                RxMsg.canMsgID = MSG_ID_EXTENDED;
            }

            RxMsg.canMessageFormat
            = (rxElem[1UL] & RX_ELEMENT_R1_FDF) >> RX_ELEMENT_FDF_OFFSET;

            RxMsg.canFilterIndex =
            (rxElem[1UL] & RX_ELEMENT_R1_FIDX) >> RX_ELEMENT_FIDX_OFFSET;

            RxMsg.canMsgLength = MsgLength;

            if (RxMsg.canMsgLength > 0UL) {
                RxMsg.canMsgDataPtr = rxBuf;
            }
            else
            {
                RxMsg.canMsgDataPtr = NULL_PTR;
            }

            /** Convert msg length (bytes) to word */
            if ((MsgLength & 0x03u) != 0u)
            {
                MsgLength = (MsgLength / 4u) + (uint8)1;
            }
            else { MsgLength /= 4u; }
            /** Read the data and store in buffer */
            (void)Can_44_TCAN4x5x_SpiRead((uint8)aId, (rxElemAdrs + (uint32)2),
                                                            (uint8)MsgLength);
            for(idx = 0U; idx < RxMsg.canMsgLength; idx++)
            {
                dataIdx = idx >> 2U;
                rxBuf[idx] =
                    (uint8)((rxElem[dataIdx] >> ((idx%4U)*8U)) & 0xFFUL);
            }

            /** Updating index in FIFO0 */
            spi_tx_buf_ptr[0] = (uint32)index;
            (void)Can_44_TCAN4x5x_SpiWrite((uint8)aId,
                            &(Can_44_TCAN4x5x_MCAN_REGS(aId)->RXF0A), (uint8)1);

            /** Call notification function */
            Can_44_TCAN4x5x_RxNotification(aId, &RxMsg);
            fifoLevel--;
            if (fifoLevel != 0u)
            {
                (void)Can_44_TCAN4x5x_SpiRead((uint8)aId,
                            &(Can_44_TCAN4x5x_MCAN_REGS(aId)->RXF0S), (uint8)1);
                (void)Can_44_TCAN4x5x_VerifySpiRead((uint8)aId, &(Can_44_TCAN4x5x_MCAN_REGS(aId)->RXF0S));
            }
        }
    }
}
#define CAN_44_TCAN4x5x_STOP_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"

#define CAN_44_TCAN4x5x_START_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"
/** brief
 *
 * Busoff event handling
 *
 * param[in]    Id      Index of the MCAN Controller
 *
 * return       void
 */
FUNC(void, CAN_44_TCAN4x5x_CODE_FAST) Can_44_TCAN4x5x_MCAN_BusoffProcess
                                (VAR(Can_44_TCAN4x5x_HwIDType, AUTOMATIC) aId)
{
    VAR(uint32, AUTOMATIC) intr;
    VAR(Std_ReturnType,AUTOMATIC) Read_Status = E_NOT_OK;
    Read_Status = Can_44_TCAN4x5x_SpiRead((uint8)aId,
                              &(Can_44_TCAN4x5x_MCAN_REGS(aId)->INTR), (uint8)1);
    (void)Can_44_TCAN4x5x_VerifySpiRead((uint8)aId, &(Can_44_TCAN4x5x_MCAN_REGS(aId)->INTR));
    if(Read_Status == E_OK)
    {
        intr = Can_44_TCAN4x5x_SpiBuf[aId].SpiRxBuf[Can_44_TCAN4x5x_ExCntxt[aId]][1]
                                                                            & IR_BO;
        /** Clear the interrupt */
        if (intr != 0u)
        {
            Can_44_TCAN4x5x_SpiBuf[aId].SpiTxBuf[Can_44_TCAN4x5x_ExCntxt[aId]][1] =
                                                                                intr;
            (void)Can_44_TCAN4x5x_SpiWrite((uint8)aId,
                                &(Can_44_TCAN4x5x_MCAN_REGS(aId)->INTR), (uint8)1);
        }
        if(intr > 0UL)
        {
            /** Notify Busoff only if it occured in STARTED state */
            if(CAN_T_START == Can_44_TCAN4x5x_ControllerMode[aId])
            {
                /** Set current controller state to STOPPED */
                Can_44_TCAN4x5x_ControllerMode[aId] = CAN_T_STOP;
                Can_44_TCAN4x5x_MCAN_RWDev[aId].state = CAN_44_TCAN4x5x_STATE_IDLE;

                /** Notify the bus-off event to upper layer */
                CanIf_ControllerBusOff((uint8)aId);
            }
        }
    }
    else
    {
        /* Do nothing */
    }
}
#define CAN_44_TCAN4x5x_STOP_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"

#define CAN_44_TCAN4x5x_START_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"
/** brief
 *
 * Processes message reception from Handler
 *
 * param[in]    Id      Index of the MCAN Controller
 * param[in]    Config  Configuration to use for message reception
 *                        RX Buffer, RX FIFO 0, or RX FIFO 1
 *
 * return       void
 */

FUNC(void, CAN_44_TCAN4x5x_CODE_FAST) Can_44_TCAN4x5x_MCAN_ProcessCanRx
                                (VAR(Can_44_TCAN4x5x_HwIDType, AUTOMATIC) aId)
{
    VAR(uint32, AUTOMATIC) intr;
    VAR(Std_ReturnType,AUTOMATIC) Read_Status = E_NOT_OK;
    #if (CAN_44_TCAN4x5x_DEV_ERROR_DETECT == STD_ON)
    VAR(uint32, AUTOMATIC) msgL0;
    VAR(uint32, AUTOMATIC) msgL1;
    #endif
    P2VAR(uint32, AUTOMATIC, CAN_44_TCAN4x5x_APPL_DATA) spi_tx_buf_ptr;
    P2VAR(uint32, AUTOMATIC, CAN_44_TCAN4x5x_APPL_DATA) spi_rx_buf_ptr;

    spi_rx_buf_ptr = &Can_44_TCAN4x5x_SpiBuf[aId].
                                    SpiRxBuf[Can_44_TCAN4x5x_ExCntxt[aId]][1];
    Read_Status = Can_44_TCAN4x5x_SpiRead((uint8)aId,
                              &(Can_44_TCAN4x5x_MCAN_REGS(aId)->INTR), (uint8)1);
   (void)Can_44_TCAN4x5x_VerifySpiRead((uint8)aId, &(Can_44_TCAN4x5x_MCAN_REGS(aId)->INTR));
    if(Read_Status == E_OK)
    {
        intr = spi_rx_buf_ptr[0] & (IR_RF0N | IR_RF1N | IR_DRX | IR_RF0L | IR_RF1L);

        /** Clear flags */
        if (intr != 0u)
        {
            spi_tx_buf_ptr = &Can_44_TCAN4x5x_SpiBuf[aId].
                                        SpiTxBuf[Can_44_TCAN4x5x_ExCntxt[aId]][1];
            spi_tx_buf_ptr[0] = intr;
            (void)Can_44_TCAN4x5x_SpiWrite((uint8)aId,
                                &(Can_44_TCAN4x5x_MCAN_REGS(aId)->INTR), (uint8)1);
        }
        if ((intr & IR_DRX) != 0UL)
        {
            /** Message Stored to Dedicated RX Buffer */
            Can_44_TCAN4x5x_MCAN_ProcessRxBuffer(aId);
        }
        if ((intr & IR_RF0N) != 0UL)
        {
            #if (CAN_44_TCAN4x5x_DEV_ERROR_DETECT == STD_ON)
            /** Message Stored to Dedicated RX FIFO 0 */
            if((intr & IR_RF0L) != 0UL)
            {
                (void)Can_44_TCAN4x5x_SpiRead((uint8)aId,
                                &(Can_44_TCAN4x5x_MCAN_REGS(aId)->RXF0S), (uint8)1);
			    (void)Can_44_TCAN4x5x_VerifySpiRead((uint8)aId, &(Can_44_TCAN4x5x_MCAN_REGS(aId)->RXF0S));
                msgL0 = spi_rx_buf_ptr[0] & RXF0S_RF0L;
                /** RX FIFO 0 Message Lost */
                if((msgL0 & RXF0S_RF0L) != 0UL)
                {
                    /** Rx Event FIFO Element Lost */
                    (void)Det_ReportError(CAN_44_TCAN4x5x_MODULE_ID,
                                        CAN_44_TCAN4x5x_INSTANCE_ID,
                                        Can_44_TCAN4x5x_ApiId,
                                        CAN_44_TCAN4x5x_E_DATALOST);
                }
            }
            #endif // (CAN_44_TCAN4x5x_DEV_ERROR_DETECT == STD_ON)
            Can_44_TCAN4x5x_MCAN_ProcessRxFIFO(aId, MSG_USE_RX_FIFO_0);
        }
        if ((intr & IR_RF1N) != 0UL)
        {
            #if (CAN_44_TCAN4x5x_DEV_ERROR_DETECT == STD_ON)
            /** Message Stored to Dedicated RX FIFO 1 */
            if ((intr & IR_RF1L) != 0UL)
            {
                (void)Can_44_TCAN4x5x_SpiRead((uint8)aId,
                                &(Can_44_TCAN4x5x_MCAN_REGS(aId)->RXF1S), (uint8)1);
			 	(void)Can_44_TCAN4x5x_VerifySpiRead((uint8)aId, &(Can_44_TCAN4x5x_MCAN_REGS(aId)->RXF1S));
                msgL1 = spi_rx_buf_ptr[0] & RXF1S_RF1L;
                /** RX FIFO 1 Message Lost */
                if((msgL1 & RXF1S_RF1L) != 0UL)
                {
                    /** Rx Event FIFO Element Lost */
                    (void)Det_ReportError(CAN_44_TCAN4x5x_MODULE_ID,
                                        CAN_44_TCAN4x5x_INSTANCE_ID,
                                        Can_44_TCAN4x5x_ApiId,
                                        CAN_44_TCAN4x5x_E_DATALOST);
                }
            }
            #endif // (CAN_44_TCAN4x5x_DEV_ERROR_DETECT == STD_ON)
            Can_44_TCAN4x5x_MCAN_ProcessRxFIFO(aId, MSG_USE_RX_FIFO_1);
        }
    }
    else
    {
        /*Do Nothing for MISRA compliance*/
    }
}
#define CAN_44_TCAN4x5x_STOP_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"

#define CAN_44_TCAN4x5x_START_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"
/** brief
 *
 * Initializes Message RAM Elements used by MCAN Controller
 *
 * param[in]    Id      Index of the MCAN Controller
 *
 * return       void
 */

FUNC(void, CAN_44_TCAN4x5x_CODE_FAST) Can_44_TCAN4x5x_MCAN_MessageRAM_Init
       (VAR(Can_44_TCAN4x5x_HwIDType, AUTOMATIC) aId,
       P2CONST(Can_44_TCAN4x5x_MsgRamConfigType, AUTOMATIC,
       CAN_44_TCAN4x5x_APPL_DATA) msg_ram_config)
{
    VAR(uint32, AUTOMATIC) next_adrs = 0u;
    P2VAR(uint32, AUTOMATIC, CAN_44_TCAN4x5x_APPL_DATA) spi_tx_buf_ptr;
    spi_tx_buf_ptr = &Can_44_TCAN4x5x_SpiBuf[aId].
                                    SpiTxBuf[Can_44_TCAN4x5x_ExCntxt[aId]][1];

    /** Standard ID Filter Configuration */
    spi_tx_buf_ptr[0] = (uint32)(
    (((uint32)msg_ram_config->CanStdandardIDFiltersCount) << SIDFC_LSS_OFFSET) |
        (STD_ID_FILTER_CONFIG_FLS_START_ADDR));
    next_adrs += (uint32)msg_ram_config->CanStdandardIDFiltersCount * 4u;

    /** Extended ID Filter Configuration */
    Can_44_TCAN4x5x_MCAN_RWDev[aId].EXTND_ID_FILTER_START_ADDR = next_adrs;
    spi_tx_buf_ptr[1] = (uint32)(
    (((uint32)msg_ram_config->CanExtendedIDFiltersCount) << XIDFC_LSS_OFFSET) |
      (next_adrs));
    next_adrs += ((uint32)msg_ram_config->CanExtendedIDFiltersCount) * 8u;

    /** Write Standard & Extended filter configuration to TCAN device */
    (void)Can_44_TCAN4x5x_SpiWrite((uint8)aId,
                            &(Can_44_TCAN4x5x_MCAN_REGS(aId)->SIDFC), 2u);

    /** RX FIFO 0 Configuration */
    Can_44_TCAN4x5x_MCAN_RWDev[aId].RX_FIFO0_START_ADDR = next_adrs;
    spi_tx_buf_ptr[0] = (uint32)((RXFIFO0_CONFIG_OPERATION_MODE <<
    RXF0C_OPERATION_MODE_OFFSET) |
            (RXFIFO0_CONFIG_WATERMARK << RXF0C_WATERMARK_OFFSET) |
    (((uint32)msg_ram_config->CanFifo0BufferCount) << RXF0C_SIZE_OFFSET) |
            (next_adrs));
    next_adrs += ((uint32)msg_ram_config->CanFifo0BufferCount) *
    (RXBUFFER_CONFIG_DATA_SIZE(Can_44_TCAN4x5x_ControllerSetting_PC[aId].
                                                    CanFdDataBufSize) + 8u);
    /** Write to TCAN device */
    (void)Can_44_TCAN4x5x_SpiWrite((uint8)aId,
                            &(Can_44_TCAN4x5x_MCAN_REGS(aId)->RXF0C), (uint8)1);

    /** RX FIFO 1 Configuration */
    Can_44_TCAN4x5x_MCAN_RWDev[aId].RX_FIFO1_START_ADDR = next_adrs;
    spi_tx_buf_ptr[0] = (uint32)((RXFIFO1_CONFIG_OPERATION_MODE <<
        RXF1C_OPERATION_MODE_OFFSET) |
            (RXFIFO1_CONFIG_WATERMARK << RXF1C_WATERMARK_OFFSET) |
    (((uint32)msg_ram_config->CanFifo1BufferCount) << RXF1C_SIZE_OFFSET) |
            (next_adrs));
    next_adrs += ((uint32)msg_ram_config->CanFifo1BufferCount) *
   (RXBUFFER_CONFIG_DATA_SIZE(Can_44_TCAN4x5x_ControllerSetting_PC[aId].
                                                        CanFdDataBufSize) + 8u);
    /** Write to TCAN device */
    (void)Can_44_TCAN4x5x_SpiWrite((uint8)aId,
                        &(Can_44_TCAN4x5x_MCAN_REGS(aId)->RXF1C), (uint8)1);

    /** RX Buffers Configuration */
    Can_44_TCAN4x5x_MCAN_RWDev[aId].RX_BUFFER_START_ADDR = next_adrs;
    spi_tx_buf_ptr[0] = next_adrs;
    next_adrs += ((uint32)msg_ram_config->CanDedicatedRxBufferCount) *
    (RXBUFFER_CONFIG_DATA_SIZE(Can_44_TCAN4x5x_ControllerSetting_PC[aId].
                                                        CanFdDataBufSize) + 8u);
    /** Write to TCAN device */
    (void)Can_44_TCAN4x5x_SpiWrite((uint8)aId,
                            &(Can_44_TCAN4x5x_MCAN_REGS(aId)->RXBC), (uint8)1);

    /** TX Event FIFO Configuration */
    Can_44_TCAN4x5x_MCAN_RWDev[aId].TX_EVENT_FIFO_START_ADDR = next_adrs;
    spi_tx_buf_ptr[0] = (uint32)((TXEVENTFIFO_CONFIG_WATERMARK <<
        TXEFC_WATERMARK_OFFSET) |
            (((uint32)msg_ram_config->CanTxBufferCount) << TXEFC_SIZE_OFFSET) |
            (next_adrs));
    next_adrs += ((uint32)msg_ram_config->CanTxBufferCount) * 8u;
    /** Write to TCAN device */
    (void)Can_44_TCAN4x5x_SpiWrite((uint8)aId,
                            &(Can_44_TCAN4x5x_MCAN_REGS(aId)->TXEFC), (uint8)1);

    /** TX Buffers Configuration */
    Can_44_TCAN4x5x_MCAN_RWDev[aId].TX_BUFFER_START_ADDR = next_adrs;
    spi_tx_buf_ptr[0] =
            (uint32)((CAN_44_TCAN4x5x_TX_FIFO_CONFIG << TXBC_TFQM_OFFSET) |
            (CAN_44_TCAN4x5x_TX_NUM_FIFO_QUEUE_BUFFERS << TXBC_SIZE_OFFSET) |
            (((uint32)msg_ram_config->CanTxBufferCount) << TXBC_NDTB_OFFSET) |
            (next_adrs));
    /** Write to TCAN device */
    (void)Can_44_TCAN4x5x_SpiWrite((uint8)aId,
                            &(Can_44_TCAN4x5x_MCAN_REGS(aId)->TXBC), (uint8)1);

    /** Up to 64 bytes for CAN FD */
    spi_tx_buf_ptr[0] =(uint32)(((uint32)CAN_RX_BUFFER_ELEMENT_DATA_SIZE(aId) <<
      RXESC_BUFFER_SIZE_OFFSET) |
      ((uint32)CAN_RX_FIFO1_ELEMENT_DATA_SIZE(aId) << RXESC_FIFO1_SIZE_OFFSET) |
            ((uint32)CAN_RX_FIFO0_ELEMENT_DATA_SIZE(aId)));
    /** Write to TCAN device */
    (void)Can_44_TCAN4x5x_SpiWrite((uint8)aId,
                        &(Can_44_TCAN4x5x_MCAN_REGS(aId)->RXESC), (uint8)1);

    spi_tx_buf_ptr[0] = (uint32)CAN_TX_BUFFER_ELEMENT_DATA_SIZE(aId);
    /** Write to TCAN device */
    (void)Can_44_TCAN4x5x_SpiWrite((uint8)aId,
                        &(Can_44_TCAN4x5x_MCAN_REGS(aId)->TXESC), (uint8)1);
}
#define CAN_44_TCAN4x5x_STOP_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"

#define CAN_44_TCAN4x5x_START_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"
static FUNC(void, CAN_44_TCAN4x5x_CODE_FAST) Can_44_TCAN4x5x_MCAN_ProcessTx
                                (VAR(Can_44_TCAN4x5x_HwIDType, AUTOMATIC) aId)
{
    VAR(uint8, AUTOMATIC) fifoLevel;
    VAR(uint32, AUTOMATIC) index;
    VAR(uint32, AUTOMATIC) canMM;
    P2VAR(uint32, AUTOMATIC, CAN_44_TCAN4x5x_APPL_DATA) txEventElem;
    P2VAR(uint32, AUTOMATIC, CAN_44_TCAN4x5x_APPL_DATA) spi_tx_buf_ptr;
    P2VAR(uint32, AUTOMATIC, CAN_44_TCAN4x5x_APPL_DATA) spi_rx_buf_ptr;

    spi_tx_buf_ptr = &Can_44_TCAN4x5x_SpiBuf[aId].
                                    SpiTxBuf[Can_44_TCAN4x5x_ExCntxt[aId]][1];
    spi_rx_buf_ptr = &Can_44_TCAN4x5x_SpiBuf[aId].
                                    SpiRxBuf[Can_44_TCAN4x5x_ExCntxt[aId]][1];

    /** Read message from TX Event FIFO */
    (void)Can_44_TCAN4x5x_SpiRead((uint8)aId,
                            &(Can_44_TCAN4x5x_MCAN_REGS(aId)->TXEFS), (uint8)1);
    (void)Can_44_TCAN4x5x_VerifySpiRead((uint8)aId,
                                    &(Can_44_TCAN4x5x_MCAN_REGS(aId)->TXEFS));
    fifoLevel = (uint8)(spi_rx_buf_ptr[0] & TXEFS_EFFL);
    while (fifoLevel != 0U)
    {
        index = (spi_rx_buf_ptr[0] & TXEFS_REFGI) >> 8UL;
        /** MISRA RULE 11.6 VIOLATION: JUSTIFICATION_4 */
        txEventElem =((uint32*)MSG_RAM_TX_EVENT_FIFO_ELEM(aId)) + (2UL * index);
        /** Read one word */
        (void)Can_44_TCAN4x5x_SpiRead((uint8)aId, (txEventElem + 1u), 1u);
        canMM = ((spi_rx_buf_ptr[0UL] & TX_EVENT_FIFO_ELEMENT_R1_MM) >> 24UL);
        Can_44_TCAN4x5x_CurrentTxTimeStamp[aId] = spi_rx_buf_ptr[0UL];

        /** Updating index in TX Event FIFO */
        spi_tx_buf_ptr[0] = (uint32)index;
        (void)Can_44_TCAN4x5x_SpiWrite((uint8)aId,
                            &(Can_44_TCAN4x5x_MCAN_REGS(aId)->TXEFA), (uint8)1);
        Can_44_TCAN4x5x_TxNotification
                            (Can_44_TCAN4x5x_Mcan_swPduHandle[aId][canMM]);
        fifoLevel--;
        if (fifoLevel != 0U)
        {
            (void)Can_44_TCAN4x5x_SpiRead((uint8)aId,
                            &(Can_44_TCAN4x5x_MCAN_REGS(aId)->TXEFS), (uint8)1);
            (void)Can_44_TCAN4x5x_VerifySpiRead((uint8)aId,
                                    &(Can_44_TCAN4x5x_MCAN_REGS(aId)->TXEFS));
        }
    };
}
#define CAN_44_TCAN4x5x_STOP_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"

#define CAN_44_TCAN4x5x_START_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"
/** brief
 *
 * Handles Transmission Complete events
 *
 * param[in]    Id      Index of the MCAN Controller
 *
 * return       void
 */

FUNC(void, CAN_44_TCAN4x5x_CODE_FAST) Can_44_TCAN4x5x_MCAN_ProcessCanTx
                                (VAR(Can_44_TCAN4x5x_HwIDType, AUTOMATIC) aId)
{
    VAR(uint32, AUTOMATIC) intr;
    VAR(Std_ReturnType,AUTOMATIC) Read_Status = E_NOT_OK;

    /** Read flags and clear flags */
    Read_Status = Can_44_TCAN4x5x_SpiRead((uint8)aId,
                        &(Can_44_TCAN4x5x_MCAN_REGS(aId)->INTR), (uint8)1);
   (void)Can_44_TCAN4x5x_VerifySpiRead((uint8)aId, &(Can_44_TCAN4x5x_MCAN_REGS(aId)->INTR));
    if(Read_Status == E_OK)
    {
        intr = Can_44_TCAN4x5x_SpiBuf[aId].
                    SpiRxBuf[Can_44_TCAN4x5x_ExCntxt[aId]][1] & (IR_TEFN | IR_TEFL);
        /** Clear flags */
        if (intr != 0u)
        {
            Can_44_TCAN4x5x_SpiBuf[aId].SpiTxBuf[Can_44_TCAN4x5x_ExCntxt[aId]][1] =
                                                                            intr;
            (void)Can_44_TCAN4x5x_SpiWrite((uint8)aId,
                                &(Can_44_TCAN4x5x_MCAN_REGS(aId)->INTR), (uint8)1);
        }
        /** To be Supported, Transmission Cancellation (intr & IR_TCF) */
        if ((intr & IR_TEFN) != 0UL)
        {
            if ((intr & IR_TEFL) != 0UL)
            {
                #if (CAN_44_TCAN4x5x_DEV_ERROR_DETECT == STD_ON)
                /** TX Event FIFO Element Lost */
                (void)Det_ReportError(CAN_44_TCAN4x5x_MODULE_ID,
                                    CAN_44_TCAN4x5x_INSTANCE_ID,
                                    Can_44_TCAN4x5x_ApiId,
                                    CAN_44_TCAN4x5x_E_TX_EVENT_FIFO_ELEMENT_LOST);
                #endif // (CAN_44_TCAN4x5x_DEV_ERROR_DETECT == STD_ON)
            }
            Can_44_TCAN4x5x_MCAN_ProcessTx(aId);
        }
    }
    else
    {
        /*Do Nothing for MISRA compliance*/
    }
}

#define CAN_44_TCAN4x5x_STOP_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"

#define CAN_44_TCAN4x5x_START_SEC_CODE_SLOW
#include "Can_44_TCAN4x5x_MemMap.h"

/** brief  Initialize MCAN Controller
 *
 * This API is used to initialize the MCAN Controller
 *
 * param[in]    Id      Index of the MCAN Controller
 * param[in]    Config  Pointer to configuration structure
 *
 * return       void
 */

FUNC(void, CAN_44_TCAN4x5x_CODE_SLOW) Can_44_TCAN4x5x_MCAN_Init
        (VAR(Can_44_TCAN4x5x_HwIDType, AUTOMATIC) aId,
         CONSTP2CONST(Can_44_TCAN4x5x_ConfigType, AUTOMATIC,
                                    CAN_44_TCAN4x5x_APPL_DATA) aConfig)
{
    VAR(uint32,AUTOMATIC) ReadData = 0u;
    VAR(uint32, AUTOMATIC) ret = E_OK;
    VAR(uint8, AUTOMATIC) filter_size;
    VAR(uint32, AUTOMATIC) indx;
    VAR(uint8, AUTOMATIC) default_baud_config;
    VAR(uint32, AUTOMATIC) LCanXtalClock;
    /** Declare local pointer for CanController config structure */
    P2CONST(Can_44_TCAN4x5x_ControllerConfigType_PC, AUTOMATIC,
                           CAN_44_TCAN4x5x_APPL_CONST)LpCanControllerconfig;
    P2VAR(uint32, AUTOMATIC, CAN_44_TCAN4x5x_APPL_DATA) spi_tx_buf_ptr;
    /**Initialization of local pointer of first CanController config structure*/
    LpCanControllerconfig = &Can_44_TCAN4x5x_ControllerSetting_PC[aId];
    LCanXtalClock = LpCanControllerconfig->CanXtalClock;
    spi_tx_buf_ptr = &Can_44_TCAN4x5x_SpiBuf[aId].
                                    SpiTxBuf[Can_44_TCAN4x5x_ExCntxt[aId]][1];
    if((aConfig->CanControllerConfiguration[aId].CanSpiBaudrateRef >
                                                              (uint32)12400000)
        && (LCanXtalClock <= (uint32)20000000))
    {
        Can_44_TCAN4x5x_ReadCmd[aId] = CAN_44_TCAN4x5x_READ_WAIT;
        /** UPDATE TCAN REGISTER FOR 4-DUMMY CYCLES */
        spi_tx_buf_ptr[0] = 0x1F1F0413u;
        /** MISRA RULE 11.6 VIOLATION: JUSTIFICATION_4 */
        (void)Can_44_TCAN4x5x_SpiWrite((uint8)aId, (uint32*)0x0014, (uint8)1);
    }
    else
    {
        Can_44_TCAN4x5x_ReadCmd[aId] = CAN_44_TCAN4x5x_READ;
    }

    #if( CAN_44_TCAN4x5x_TARGET_DEVICE == CAN_44_TCAN4x5x_TCAN1043)

         spi_tx_buf_ptr[0] = 0u;
        /** MISRA RULE 11.6 VIOLATION: JUSTIFICATION_4 */
        (void)Can_44_TCAN4x5x_SpiWrite((uint8)aId, (uint32*)0x0830, (uint8)1);

    #endif

    (void)Can_44_TCAN4x5x_SpiRead((uint8)aId, (uint32*)0x0008, (uint8)1);

    ReadData = Can_44_TCAN4x5x_SpiBuf[aId].
                                    SpiRxBuf[Can_44_TCAN4x5x_ExCntxt[aId]][1];

    Can_44_TCAN4x5x_MajorRevId = (uint8)((ReadData >> (uint32)8) & (uint32)0x000000FF);

    if(Can_44_TCAN4x5x_MajorRevId != (uint8)0x01)
    {
        if (aId < Can_44_TCAN4x5x_MCAN_MAX_HW_ID)
        {
            Can_44_TCAN4x5x_MCAN_SetCCE(aId);        /** Set CCCR.INI and CCCR.CCE.
                                                This sets Driver state to Idle */
            /** Initialize and clear Message RAM (2K bytes) */
            for (indx=0u; indx<16u; indx++) { spi_tx_buf_ptr[indx] = 0u; }

            //In CC26x2, this function takes 29ms to complete:
            //- from start to here takes 3ms
            //- the following loop takes 17ms
            //- from the following loop to the end takes 9ms
            for (indx=0u; indx<32u; indx++)
            {
                (void)Can_44_TCAN4x5x_SpiWrite((uint8)aId,
                     (uint32*)(MSG_RAM_BASE_ADDR+(indx * 16UL * 4UL)), (uint8)16);

                if (indx==16)
                {
                    //kick the watchdog
                    WdgMgr_Kick( WDGMGRkeN_ALL );
                }
            }
            Can_44_TCAN4x5x_MCAN_MessageRAM_Init(aId,
                        &aConfig->CanControllerConfiguration[aId].CanMsgRamConfig);
            /** Set bit timing and filter parameters */
            default_baud_config =
              aConfig->CanControllerConfiguration[aId].CanControllerDefaultBaudrate;
            ret = Can_44_TCAN4x5x_MCAN_SetBaudrate(aId,
                     &aConfig->CanControllerConfiguration[aId].
                               CanControllerBaudrateConfigPtr[default_baud_config]);

            filter_size = aConfig->CanControllerConfiguration[aId].CanMsgRamConfig.
             CanStdandardIDFiltersCount + aConfig->CanControllerConfiguration[aId].
                        CanMsgRamConfig.CanExtendedIDFiltersCount;
            Can_44_TCAN4x5x_MCAN_ConfigureGlobalFilters(aId,
                                   aConfig->CanControllerConfiguration[aId].
                                            CanRxObjFilterConfigPtr,filter_size);

            /* TCAN1043 Code*/
            #if( CAN_44_TCAN4x5x_TARGET_DEVICE == CAN_44_TCAN4x5x_TCAN1043)

            SchM_Enter_Can_44_TCAN4x5x_Exclusive_Area_05();

            (void)Can_44_TCAN4x5x_SpiRead((uint8)aId, (uint32*)0x0800, (uint8)1);

            ReadData = Can_44_TCAN4x5x_SpiBuf[aId].
                                        SpiRxBuf[Can_44_TCAN4x5x_ExCntxt[aId]][1];

             /****************************  STEP 1 ******************************/

            /* Write 0x0800[15:14] = 2b10 to set RX/GPIO1 to an input for
                                            Silicon Ver1.0,Ver1.1 and Ver2.0*/
            ReadData &= ~((uint32)1 << (uint32)14);
            ReadData |= ((uint32)1 << (uint32)15);
            Can_44_TCAN4x5x_SpiBuf[aId].
                          SpiTxBuf[Can_44_TCAN4x5x_ExCntxt[aId]][1] = ReadData;
                /** MISRA RULE 11.6 VIOLATION: JUSTIFICATION_4 */
            (void)Can_44_TCAN4x5x_SpiWrite((uint8)aId, (uint32*)0x0800, (uint8)1);

            /****************************  STEP 2 ****************************/
           if(Can_44_TCAN4x5x_MajorRevId != (uint8)0x2) {
                /* BIT[1:0] =01 CAN test with TXD/RXD_INT_CAN mapped to external pins*/
                ReadData &= ~((uint32)1 << (uint32)1);
                ReadData |= ((uint32)1 << (uint32)0);
            }
            else{
                /* BIT[0] = 1 CAN test with TXD/RXD_INT_CAN mapped to external
                pins for Silicon Ver 2.0 & BIT[1] is default '0' i.e SWE_DIS:
                Sleep Wake Error Enabled*/
                ReadData |= ((uint32)1 << (uint32)0);
            }

            Can_44_TCAN4x5x_SpiBuf[aId].
                  SpiTxBuf[Can_44_TCAN4x5x_ExCntxt[aId]][1] = ReadData;
            /** MISRA RULE 11.6 VIOLATION: JUSTIFICATION_4 */
            (void)Can_44_TCAN4x5x_SpiWrite((uint8)aId, (uint32*)0x0800, (uint8)1);

            /***************************** STEP 3 *****************************/
            if(Can_44_TCAN4x5x_MajorRevId != (uint8)0x2) {
                /* BIT[7:6] =11 to Enter Trcv to Test Mode for Silicon Ver1.0 & Ver1.1*/
                ReadData |= ((uint32)1 << (uint32)6);
                ReadData |= ((uint32)1 << (uint32)7);
             }
             else{
                /* BIT[21] = 1 TEST_MODE_EN; Test mode enable. for Silicon Ver2.0 */
                ReadData |= ((uint32)1 << (uint32)21);
             }
            Can_44_TCAN4x5x_SpiBuf[aId].
                    SpiTxBuf[Can_44_TCAN4x5x_ExCntxt[aId]][1] = ReadData;
            /** MISRA RULE 11.6 VIOLATION: JUSTIFICATION_4 */
            (void)Can_44_TCAN4x5x_SpiWrite((uint8)aId, (uint32*)0x0800, (uint8)1);

            SchM_Exit_Can_44_TCAN4x5x_Exclusive_Area_05();
            #endif

            #if (CAN_44_TCAN4x5x_DEV_ERROR_DETECT == STD_ON)
            if (ret != (uint32)E_OK)
            {
                (void)Det_ReportError(CAN_44_TCAN4x5x_MODULE_ID,
                                      CAN_44_TCAN4x5x_INSTANCE_ID,
                                      Can_44_TCAN4x5x_ApiId,
                                      CAN_44_TCAN4x5x_E_SETBAUDRATE_FAILED);
            }
            /** After Init the, CAN HW should be in STOPPED state */
            if(Can_44_TCAN4x5x_MCAN_SetMode(aId, CAN_44_TCAN4x5x_STATE_IDLE) !=
                                                                      (uint32)E_OK)
            {
                (void)Det_ReportError(CAN_44_TCAN4x5x_MODULE_ID,
                                      CAN_44_TCAN4x5x_INSTANCE_ID,
                                      Can_44_TCAN4x5x_ApiId,
                                      CAN_44_TCAN4x5x_E_SETMODE_FAILED);
            }
            #else
             if(ret == (uint32)E_OK)
             {
                /** Do Nothing */
             }
            #endif // (CAN_44_TCAN4x5x_DEV_ERROR_DETECT == STD_ON)
        }

        #if (CAN_44_TCAN4x5x_DEV_ERROR_DETECT == STD_ON)
        else
        {
            (void)Det_ReportError(CAN_44_TCAN4x5x_MODULE_ID,
                                  CAN_44_TCAN4x5x_INSTANCE_ID,
                                  Can_44_TCAN4x5x_ApiId,
                                  CAN_44_TCAN4x5x_E_INVALID_ARGUMENTS);
        }
        #endif // (CAN_44_TCAN4x5x_DEV_ERROR_DETECT == STD_ON)
        Can_44_TCAN4x5x_DrvConfigPtr = aConfig;
     }
     else{
      /* Do nothing:Can Driver remains in Uninit state */
     }

    return;
}

#define CAN_44_TCAN4x5x_STOP_SEC_CODE_SLOW
#include "Can_44_TCAN4x5x_MemMap.h"

#define CAN_44_TCAN4x5x_START_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"

/** brief
 *
 * Configures individual filter masks
 *
 * param[in]    Id              Index of the MCAN Controller
 * param[in]    filterObjPtr      Pointer to configured filter object
 * param[in]    filterConfig      Filter Element Configuration
 *                                 Values:
 *                                  1: Store in Rx FIFO 0 if Filter matches
 *                                  2: Store in Rx FIFO 1 if Filter matches
 *                                   7: Store in Rx Buffer
 *
 * return       void
 */

static FUNC(void, CAN_44_TCAN4x5x_CODE_FAST)configFilterMasks
(VAR(Can_44_TCAN4x5x_HwIDType, AUTOMATIC) aId,
    P2CONST(Can_44_TCAN4x5x_RxObjFilterConfigType, AUTOMATIC,
                            CAN_44_TCAN4x5x_APPL_DATA) filterObjPtr,
    VAR(uint32, AUTOMATIC) filterConfig)
{
    P2VAR(uint32, AUTOMATIC, CAN_44_TCAN4x5x_APPL_DATA) filterElem;
    P2VAR(uint32, AUTOMATIC, CAN_44_TCAN4x5x_APPL_DATA) spi_tx_buf_ptr;
    spi_tx_buf_ptr = &Can_44_TCAN4x5x_SpiBuf[aId].
                                    SpiTxBuf[Can_44_TCAN4x5x_ExCntxt[aId]][1];

    if (filterObjPtr->CanIdType == MSG_ID_STANDARD) {
        /** MISRA RULE 11.6 VIOLATION: JUSTIFICATION_4 */
        filterElem = (uint32*)MSG_RAM_STD_MSG_ID_FILTER_ELEM(aId);
    spi_tx_buf_ptr[0] =
                    ( (((uint32)filterObjPtr->CanFilterType) << 30UL) |
                              (filterConfig << 27UL) |
                              (filterObjPtr->CanHwFilterCode << 16UL) |
                              (filterObjPtr->CanHwFilterMask) );
    (void)Can_44_TCAN4x5x_SpiWrite((uint8)aId,
                &filterElem[Can_44_TCAN4x5x_Mcan_stdFilterIndex], (uint8)1);
                                        Can_44_TCAN4x5x_Mcan_stdFilterIndex++;
    }
    else
    {
        /** MISRA RULE 11.6 VIOLATION: JUSTIFICATION_4 */
        filterElem = (uint32*)MSG_RAM_EXT_MSG_ID_FILTER_ELEM(aId);
    spi_tx_buf_ptr[0] =
                  ( (filterConfig << 29UL) | (filterObjPtr->CanHwFilterCode) );
    spi_tx_buf_ptr[1] =
                ( (((uint32)filterObjPtr->CanFilterType) << 30UL) |
                                       (filterObjPtr->CanHwFilterMask) );
    (void)Can_44_TCAN4x5x_SpiWrite((uint8)aId,
                        &filterElem[Can_44_TCAN4x5x_Mcan_extFilterIndex], 2u);
            Can_44_TCAN4x5x_Mcan_extFilterIndex += (uint8)2;
    }
}
#define CAN_44_TCAN4x5x_STOP_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"

#define CAN_44_TCAN4x5x_START_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"
/** brief  MCAN IRQ Handler for Line 0
 *
 * This API is called when MCAN interrupt is asserted for Interrupt Line 0
 *                                                          (used for Rx and Tx)
 *
 * param[in]    Id  Index of the MCAN port. Passed by osil.
 *
 * return       void
 */

FUNC(void, CAN_44_TCAN4x5x_CODE_FAST) Can_44_TCAN4x5x_MCAN_ProcessCanIRQLine0
                               (VAR(Can_44_TCAN4x5x_HwIDType, AUTOMATIC) aId)
{
    uint8 exec_context_backup;
    if ((aId >= Can_44_TCAN4x5x_MCAN_MAX_HW_ID))
    {
        #if (CAN_44_TCAN4x5x_DEV_ERROR_DETECT == STD_ON)
        (void)Det_ReportError(CAN_44_TCAN4x5x_MODULE_ID,
                              CAN_44_TCAN4x5x_INSTANCE_ID,
                              Can_44_TCAN4x5x_ApiId,
                              CAN_44_TCAN4x5x_E_PARAM_CONTROLLER);
        #endif // (CAN_44_TCAN4x5x_DEV_ERROR_DETECT == STD_ON)
    }
    else
    {
        if (CAN_44_TCAN4x5x_STATE_RUNNING == Can_44_TCAN4x5x_MCAN_RWDev[aId].state)
        {
            exec_context_backup = Can_44_TCAN4x5x_ExCntxt[aId];
            Can_44_TCAN4x5x_ExCntxt[aId] = ISR_CONTEXT;
            if(Can_44_TCAN4x5x_ControllerSetting_PC[aId].CanTxProcessing ==
                                                        CAN_44_TCAN4x5x_INTERRUPT)
            {
                Can_44_TCAN4x5x_MCAN_ProcessCanTx(aId);
            }

            if(Can_44_TCAN4x5x_ControllerSetting_PC[aId].CanRxProcessing ==
                                                        CAN_44_TCAN4x5x_INTERRUPT)
            {
                Can_44_TCAN4x5x_MCAN_ProcessCanRx(aId);
            }

            if(Can_44_TCAN4x5x_ControllerSetting_PC[aId].CanBusoffProcessing ==
                                                        CAN_44_TCAN4x5x_INTERRUPT)
            {
                Can_44_TCAN4x5x_MCAN_BusoffProcess(aId);
            }
            Can_44_TCAN4x5x_ExCntxt[aId] = exec_context_backup;
        }
    }
}
#define CAN_44_TCAN4x5x_STOP_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"


#define CAN_44_TCAN4x5x_START_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"

/** brief
 *
 * This API is used to write a message to CAN TX Buffer or FIFO / Queue
 *
 * param[in]    Id    Index of the MCAN Controller
 * param[in]    TxObj Pointer to Transmit Object containing message information
 *
 * return       uint8    Error Code
 */

FUNC(uint32, CAN_44_TCAN4x5x_CODE_FAST) Can_44_TCAN4x5x_MCAN_Write
(VAR(Can_44_TCAN4x5x_HwIDType, AUTOMATIC) aId,
P2CONST(Can_PduType, AUTOMATIC, CAN_44_TCAN4x5x_APPL_DATA) pduInfo,
P2CONST(Can_44_TCAN4x5x_TxObjConfigType, AUTOMATIC,
                                        CAN_44_TCAN4x5x_APPL_DATA) TxObj)
{
    VAR(Std_ReturnType, AUTOMATIC) Ret_Val;
    VAR(uint8, AUTOMATIC) idx;
    VAR(uint32, AUTOMATIC) index;
    VAR(uint8, AUTOMATIC) tx_buf_idx;
    VAR(uint32, AUTOMATIC) msgMarkerIndex = 0UL;
    VAR(uint32, AUTOMATIC) fdf_config;
    VAR(uint32, AUTOMATIC) brs_config;
    VAR(uint32, AUTOMATIC) efc_config = 0x1UL;  /** Set to store TX Events */
    VAR(uint32, AUTOMATIC) id_type = 0UL;
    VAR(uint32, AUTOMATIC) dataIdx;
    VAR(uint32, AUTOMATIC) retval = E_OK;
    VAR(uint32, AUTOMATIC) msgLength;
    VAR(uint32, AUTOMATIC) dlcCode;
    P2VAR(uint32, AUTOMATIC, CAN_44_TCAN4x5x_APPL_DATA) txElem;
    P2VAR(void, AUTOMATIC, CAN_44_TCAN4x5x_APPL_DATA) dataBufPtr;
    VAR(uint8, AUTOMATIC) data;
    VAR(PduInfoType, AUTOMATIC) pduInformation;
    #if(CAN_44_TCAN4x5x_TRIGGER_TRANSMIT_SUPPORTED_IN_CANIF == STD_ON)
    VAR(uint8, AUTOMATIC) data_buffer[64u];
    #endif
    P2VAR(uint32, AUTOMATIC, CAN_44_TCAN4x5x_APPL_DATA) spi_tx_buf_ptr;
    VAR(uint32, AUTOMATIC) TXBRPValue = 0x0u;

    /** Store message into Dedicated TX buffer */
    tx_buf_idx = TxObj->CanTxBufferIndex;
    (void)Can_44_TCAN4x5x_SpiRead((uint8)aId,
                        &(Can_44_TCAN4x5x_MCAN_REGS(aId)->TXBRP), (uint8)1);
	(void)Can_44_TCAN4x5x_VerifySpiRead((uint8)aId, &(Can_44_TCAN4x5x_MCAN_REGS(aId)->TXBRP));
    TXBRPValue = Can_44_TCAN4x5x_SpiBuf[aId].
                                    SpiRxBuf[Can_44_TCAN4x5x_ExCntxt[aId]][1];
    for (idx = 0u; idx < (TxObj->CanHwObjectCount); idx++)
    {
        if ((TXBRPValue & (0x01UL << tx_buf_idx)) == 0U)
        {
            /** Buffer is available */
            retval = E_OK;
            /** copy PDU information */
            pduInformation.SduLength = pduInfo->length;
            pduInformation.SduDataPtr = pduInfo->sdu;

            #if(CAN_44_TCAN4x5x_TRIGGER_TRANSMIT_SUPPORTED_IN_CANIF == \
                                                                        STD_ON)
            if ((pduInfo->sdu == NULL_PTR) &&
                (TxObj->CanTriggerTransmitEnable == STD_ON))
            {

                pduInformation.SduDataPtr = data_buffer;

                retval = CanIf_TriggerTransmit(
                            ((PduIdType)(pduInfo->swPduHandle)),
                                            &pduInformation);
            }
            #endif

            if (retval == (uint32)E_OK)
            {
                /** TX Request not pending for this buffer, add message to
                                                            this buffer... */
                spi_tx_buf_ptr = &Can_44_TCAN4x5x_SpiBuf[aId].
                                    SpiTxBuf[Can_44_TCAN4x5x_ExCntxt[aId]][1];
                txElem = spi_tx_buf_ptr;

                id_type = pduInfo->id & CAN_44_TCAN4x5x_ID_TYPE_MASK;

                if (id_type == MSG_ID_STANDARD)
                {
                    txElem[0UL] = ((0UL << TX_ELEMENT_XTD_OFFSET) |
                        (pduInfo->id << TX_ELEMENT_STDID_OFFSET));
                }
                else
                {
                    txElem[0UL] = ((1UL << TX_ELEMENT_XTD_OFFSET) |
                                                    (pduInfo->id));
                }

                /** Use buffer index as MM and use that MM to get swPduHandle
                    after successfull transmission */
                msgMarkerIndex = tx_buf_idx;
                Can_44_TCAN4x5x_Mcan_swPduHandle[aId][msgMarkerIndex] =
                                                          pduInfo->swPduHandle;

                if(Can_44_TCAN4x5x_ControllerSetting_PC[aId].
                                            CanFDBaudRateEnabled == (uint8)1)
                {
                    fdf_config = pduInfo->id & CAN_44_TCAN4x5x_FRAME_TYPE_MASK;
                    if (fdf_config != MSG_STANDARD_FORMAT)
                    {
                        fdf_config = MSG_CANFD_FORMAT;
                    }
                    brs_config = Can_44_TCAN4x5x_Mcan_FdConfig[aId]->
                                            CanControllerDataTxBitRateSwitch;
                }
                else
                {
                    fdf_config = MSG_STANDARD_FORMAT;
                    brs_config = 0u;
                }
                if(Can_44_TCAN4x5x_ControllerSetting_PC[aId].
                                            CanFDBaudRateEnabled == (uint8)1)
                {
                    dlcCode = ConvertToDLC(pduInformation.SduLength);
                    msgLength = ConvertCanDLC(dlcCode);
                }
                else
                {
                    msgLength = pduInformation.SduLength;
                    dlcCode = msgLength;
                }
                txElem[1UL] = ((msgMarkerIndex << TX_ELEMENT_MARKER_OFFSET) |
                            (efc_config << TX_ELEMENT_EFC_OFFSET) |
                            (fdf_config << TX_ELEMENT_FDF_OFFSET) |
                            (brs_config << TX_ELEMENT_BRS_OFFSET) |
                            (dlcCode << TX_ELEMENT_DLC_OFFSET));

                dataBufPtr = &txElem[2UL];
                /**clear data buffer memory */
                (void)memset(dataBufPtr, (sint32)0U, msgLength);

                /** Store data bytes into TX Element */
                for(index = 0U; index < msgLength; index++)
                {
                    dataIdx = index >> 2U;
                    if((Can_44_TCAN4x5x_ControllerSetting_PC[aId].
                            CanFDBaudRateEnabled == (uint8)1) &&
                                        (index >= pduInformation.SduLength))
                    {
                        /** Fill with user configured padding byte value */
                        data = TxObj->CanFdPaddingValue;
                    }
                    else
                    {
                        data = pduInformation.SduDataPtr[index];
                    }
                    txElem[2U + dataIdx] |= (((uint32)data) <<
                                                        ((index%4U) * 8U));
                }
                /** Write MCAN TX buffer element */
                /** MISRA RULE 11.6 VIOLATION: JUSTIFICATION_4 */
                txElem = (((uint32*)MSG_RAM_TX_BUFFER_ELEM(aId)) + ((2U +
                    (TXBUFFER_CONFIG_DATA_SIZE
                    (Can_44_TCAN4x5x_ControllerSetting_PC[aId].CanFdDataBufSize)
                    >> 2U)) * tx_buf_idx));
                /** Convert msg length (bytes) to word */
                if ((msgLength & 0x03u) != 0u)
                {
                    msgLength = (msgLength / 4u) + (uint8)1;
                }
                else { msgLength /= 4u; }
                (void)Can_44_TCAN4x5x_SpiWrite((uint8)aId, txElem,
                                                    ((uint8)msgLength + 2u));

                /** Set appropriate bit in TXBAR to request transmission */
                *spi_tx_buf_ptr = (0x01UL << tx_buf_idx);
                Ret_Val = Can_44_TCAN4x5x_SpiWrite((uint8)aId,
                        &(Can_44_TCAN4x5x_MCAN_REGS(aId)->TXBAR), (uint8)1);
                if(Ret_Val == E_OK)
                {
                    retval = (uint32)E_OK;
                }
            }
            else
            {
                    retval = (uint32)E_NOT_OK;
            }
            /** Exit for loop */
            break;
        }
        else
        {
            retval = (uint32)CAN_BUSY;
        }
        tx_buf_idx++;
    }

    return retval;
}

#define CAN_44_TCAN4x5x_STOP_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"

#ifdef __TCAN4x5x_DRIVERS_TEST_ENVIRONMENT__
#define CAN_44_TCAN4x5x_START_SEC_CODE_SLOW
#include "Can_44_TCAN4x5x_MemMap.h"

/** brief
 *
 * This API is used to de-initialize the MCAN Controller
 *
 * param[in]    Id  Index of the MCAN Controller
 *
 * return       void
 */

FUNC(void, CAN_44_TCAN4x5x_CODE_SLOW) Can_44_TCAN4x5x_MCAN_DeInit
                                (VAR(Can_44_TCAN4x5x_HwIDType, AUTOMATIC) aId)
{
    VAR(uint32, AUTOMATIC) ret;

    if (Can_44_TCAN4x5x_MCAN_RWDev[aId].state == CAN_44_TCAN4x5x_STATE_UNINIT)
    {
        return;
    }

    if (Can_44_TCAN4x5x_MCAN_RWDev[aId].state == CAN_44_TCAN4x5x_STATE_RUNNING)
    {
        /** To be supported, Abort TX? */
    }

    Can_44_TCAN4x5x_MCAN_DisableAllInterrupts(aId);
    ret = Can_44_TCAN4x5x_MCAN_SetMode(aId, CAN_44_TCAN4x5x_STATE_UNINIT);
    Can_44_TCAN4x5x_ControllerMode[aId] = (Can_StateTransitionType)0;

    if(ret == (uint32)E_OK)
    {
        /** Do Nothing */
    }
}

#define CAN_44_TCAN4x5x_STOP_SEC_CODE_SLOW
#include "Can_44_TCAN4x5x_MemMap.h"
#endif

#define CAN_44_TCAN4x5x_START_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"
FUNC(void, CAN_44_TCAN4x5x_CODE_FAST) Can_44_TCAN4x5x_RxNotification
    (VAR(Can_44_TCAN4x5x_HwIDType, AUTOMATIC) aId,
     P2VAR(Can_44_TCAN4x5x_MsgInfoType, AUTOMATIC,
                                        CAN_44_TCAN4x5x_APPL_DATA) MsgInfo)
{
    VAR(Can_HwType, AUTOMATIC) Mailbox;
    VAR(PduInfoType, AUTOMATIC) PduInfo;

  #if (CAN_44_TCAN4x5x_PUBLIC_ICOM_SUPPORT == STD_ON)
  /** If ICOM is not active OR valid ICOM rx wakeup message received... */
  if ((Can_44_TCAN4x5x_GComConfigId[aId] == 0U) ||
                          (Can_44_TCAN4x5x_ProcessIcomRx(aId,MsgInfo) == E_OK))
  #endif
  {
    /** Fill the mail box data */
    Mailbox.ControllerId = (uint8)aId;

    if (MsgInfo->canMsgID == MSG_ID_STANDARD)
    {
        #if (CAN_44_TCAN4x5x_PUBLIC_ICOM_SUPPORT == STD_ON)
        if(Can_44_TCAN4x5x_GComConfigId[aId] != 0UL) {
            Mailbox.Hoh = (uint8)0;
        }
        else
        #endif
        {
         Mailbox.Hoh =
         Can_44_TCAN4x5x_DrvConfigPtr->CanControllerConfiguration[aId].
                        CanFilterIdxToRxObjStandard[MsgInfo->canFilterIndex];
        }

        Mailbox.CanId = (MsgInfo->canID | MSG_ID_STANDARD);
    }
    else
    {
        Mailbox.Hoh =
        Can_44_TCAN4x5x_DrvConfigPtr->CanControllerConfiguration[aId].
                        CanFilterIdxToRxObjExtended[MsgInfo->canFilterIndex];
        Mailbox.CanId = (MsgInfo->canID | CAN_44_TCAN4x5x_EXTD_ID_MASK);
    }

    if(Can_44_TCAN4x5x_ControllerSetting_PC[aId].CanFDBaudRateEnabled ==
                                                                      (uint8)1)
    {
        if (MsgInfo->canMessageFormat == MSG_CANFD_FORMAT)
        {
            Mailbox.CanId |= CAN_44_TCAN4x5x_FD_FRAME_MASK;
        }
    }

    /** Fill the SDU length and data pointer */
    PduInfo.SduDataPtr = MsgInfo->canMsgDataPtr;
    PduInfo.SduLength = MsgInfo->canMsgLength;
    PduInfo.SduTimeStamp = MsgInfo->canTimeStamp;
    #ifdef CAN_44_TCAN4x5x_LPDU_RECEIVE_CALLOUT_FUNCTION
    if(CAN_44_TCAN4x5x_LPDU_RECEIVE_CALLOUT_FUNCTION(Mailbox.Hoh, Mailbox.CanId,
                              PduInfo.SduLength, PduInfo.SduDataPtr) == TRUE)
    #endif
    {
        /** Call notification */
        CanIf_RxIndication(&Mailbox, &PduInfo);
        #if (CAN_44_TCAN4x5x_PUBLIC_ICOM_SUPPORT == STD_ON)
        if((Can_44_TCAN4x5x_GComConfigId[aId] != 0UL) &&
                                 (Can_44_TCAN4x5x_IcomWakeup[aId] == (uint8)1))
        {
            EcuM_CheckWakeup(Can_44_TCAN4x5x_ControllerSetting_PC[aId].
                                                                CanWakeupRef);
        }
        #endif
    }
  }
}
#define CAN_44_TCAN4x5x_STOP_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"


#define CAN_44_TCAN4x5x_START_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"
FUNC(void, CAN_44_TCAN4x5x_CODE_FAST) Can_44_TCAN4x5x_TxNotification
                                        (VAR(PduIdType, AUTOMATIC) swPduHandle)
{
    CanIf_TxConfirmation(swPduHandle);
}
#define CAN_44_TCAN4x5x_STOP_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"

#if (CAN_44_TCAN4x5x_PUBLIC_ICOM_SUPPORT == STD_ON)
#define CAN_44_TCAN4x5x_START_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"
static FUNC(Std_ReturnType, CAN_44_TCAN4x5x_CODE_FAST)
 Can_44_TCAN4x5x_ProcessIcomRx(VAR(Can_44_TCAN4x5x_HwIDType, AUTOMATIC) aId,
                    P2CONST(Can_44_TCAN4x5x_MsgInfoType, AUTOMATIC,
                                        CAN_44_TCAN4x5x_APPL_DATA) MsgInfo)
{

    /** Declare local pointer for ICOM config structure */
    P2CONST(Can_44_TCAN4x5x_IcomConfigType, AUTOMATIC,
                                        CAN_44_TCAN4x5x_APPL_CONST)LpIcomconfig;

    /** Local pointer to the config structure for wakeup causes of
       received messages */
    P2CONST(Can_44_TCAN4x5x_IcomRxMessageType, AUTOMATIC,
                                CAN_44_TCAN4x5x_APPL_CONST)LpIcomRxmessagePtr;

    P2CONST(Can_44_TCAN4x5x_RxObjFilterConfigType, AUTOMATIC,
    CAN_44_TCAN4x5x_APPL_CONST)LpCanRxObjFilterIComConfig;
    VAR(uint32, AUTOMATIC) LucCount;
    VAR(uint16, AUTOMATIC) LCanIcomCounterValue;
    VAR(uint32, AUTOMATIC) LCanIcomMessageId;
    VAR(uint8, AUTOMATIC) LcanIcomRxMessages_size;
    VAR(uint8, AUTOMATIC) CALL_Notification = FALSE;
    VAR(Std_ReturnType, AUTOMATIC) ret_val = E_NOT_OK;
    #if (CAN_44_TCAN4x5x_ICOM_RX_MESSAGE_SINAL_CONFIGURATION == STD_ON)
    VAR(uint64, AUTOMATIC) LCanIcomSignalMask;
    VAR(uint64, AUTOMATIC) LCanIcomSignalValue;
    P2CONST(Can_44_TCAN4x5x_IcomRxMessageSignalConfigType, AUTOMATIC,
    CAN_44_TCAN4x5x_APPL_CONST)LpCanIcomRxMessageSignalConfigPtr;
    VAR(uint64, AUTOMATIC) data = 0u;
    VAR(uint32, AUTOMATIC) LucCount1;
    #endif

   /** Initialization of local pointer of first ICOM config structure */
    LpIcomconfig =
      &Can_44_TCAN4x5x_Icomconfig[Can_44_TCAN4x5x_GComConfigId[aId] - (uint8)1];

    LcanIcomRxMessages_size = LpIcomconfig->canIcomRxMessages_size;

    /** Get the value of ICOM RX message structure pointer in the
                                                    local pointer variable */
    LpIcomRxmessagePtr = LpIcomconfig->CanIcomRxMessagePtr;

    /** Loop to parse matching received message ICOM configuration
        structures */
    for (LucCount = 0UL; LucCount < LcanIcomRxMessages_size;
                                                        LucCount++)
    {
        LpCanRxObjFilterIComConfig =
                     &LpIcomRxmessagePtr->CanRxObjFilterIComConfigPtr[LucCount];

        /** Local variable for CanIcomMessageId */
        LCanIcomMessageId = LpCanRxObjFilterIComConfig->CanHwFilterCode;

        LCanIcomCounterValue = LpIcomRxmessagePtr->CanIcomCounterValue;

        /** Condition check for payload error  */
        if((LpIcomRxmessagePtr->CanIcomPayloadLengthError == (boolean)TRUE) &&
           (MsgInfo->canMsgLength != 8UL))
        {
            Can_44_TCAN4x5x_IcomWakeup[aId] = (uint8)1;
            EcuM_CheckWakeup(Can_44_TCAN4x5x_ControllerSetting_PC[aId].
                                                                CanWakeupRef);
        }
        else
        {
            /** MCU wake up if the message with the ID is received
                    "LCanIcomCounterValue" times on the communication channel */
            #if(CAN_44_TCAN4x5x_ICOM_COUNTER_VALUE == STD_ON)
            if(MsgInfo->canID == LCanIcomMessageId)
            {
                Can_44_TCAN4x5x_IcomMsgCounter[aId][LucCount]++;
                if(Can_44_TCAN4x5x_IcomMsgCounter[aId][LucCount] >=
                                                           LCanIcomCounterValue)
                {
                    Can_44_TCAN4x5x_IcomMsgCounter[aId][LucCount] = 0u;
                    Can_44_TCAN4x5x_IcomWakeup[aId] = (uint8)1;
                    EcuM_CheckWakeup(Can_44_TCAN4x5x_ControllerSetting_PC[aId].
                                                                CanWakeupRef);
                }
            }
            #endif

            #if (CAN_44_TCAN4x5x_ICOM_RX_MESSAGE_SINAL_CONFIGURATION == STD_ON)
            /** Local pointer to the config structure for wakeup causes of
            received messages signal */
            LpCanIcomRxMessageSignalConfigPtr =
                            LpIcomRxmessagePtr->CanIcomRxMessageSignalConfigPtr;

            LCanIcomSignalMask =
                        LpCanIcomRxMessageSignalConfigPtr->CanIcomSignalMask;

            LCanIcomSignalValue =
                        LpCanIcomRxMessageSignalConfigPtr->CanIcomSignalValue;

            /** move LSB to MSB and vice-versa */
            for (LucCount1 = 0u; LucCount1<MsgInfo->canMsgLength; LucCount1++)
            {
                data = data << 8;
                data |= ((uint64)(*(MsgInfo->canMsgDataPtr + LucCount1)));
            }

            data = data & LCanIcomSignalMask;
            for (LucCount1 = 0UL; LucCount1 <
           (LpIcomRxmessagePtr->CanIcomRxMessageSignalConfig_size); LucCount1++)
            {
               switch(LpCanIcomRxMessageSignalConfigPtr->CanIcomSignalOperation)
                {
                    case CAN_44_TCAN4x5x_ICOM_SIG_OP_AND:
                    {
                        /** Condition check if received signal masked with
                           CanIcomsignalmask has atleast one bit set in common
                                                    with CanIcomsignalvalue */
                        if((data & LCanIcomSignalValue) != 0UL)
                        {
                            CALL_Notification = TRUE;
                        }
                    }
                    break;
                    case CAN_44_TCAN4x5x_ICOM_SIG_OP_EQUAL:
                    {
                        /** Condition check if received signal masked with
                           CanIcomsignalmask is equal to with
                           CanIcomsignalvalue */
                        if(data == LCanIcomSignalValue )
                        {
                            CALL_Notification = TRUE;
                        }
                    }
                    break;
                    case CAN_44_TCAN4x5x_ICOM_SIG_OP_GREATER:
                    {
                        /** Condition check if received signal masked  with
                        CanIcomsignalmask is greater than CanIcomsignalvalue */
                        if(data > LCanIcomSignalValue)
                        {
                            CALL_Notification = TRUE;
                        }
                    }
                    break;
                    case CAN_44_TCAN4x5x_ICOM_SIG_OP_SMALLER:
                    {
                        /** Condition check if received signal masked with
                        CanIcomsignalmask is smaller than CanIcomsignalvalue */
                        if(data < LCanIcomSignalValue)
                        {
                            CALL_Notification = TRUE;
                        }
                    }
                    break;
                    case CAN_44_TCAN4x5x_ICOM_SIG_OP_XOR:
                    {
                        /** Condition check if received signal masked with
                        CanIcomSignalMask then XORed to CanIcomSignalValue is
                            not null */
                        if((data ^ LCanIcomSignalValue) != 0UL)
                        {
                            CALL_Notification = TRUE;
                        }
                    }
                    break;
                    default:
                    break;
                }
                LpCanIcomRxMessageSignalConfigPtr++;
                if(CALL_Notification == TRUE)
                {
                    break;
                }
            }
            #endif
            /** Increment pointer to point to next RX message structure */
            LpIcomRxmessagePtr++;
            if(CALL_Notification == TRUE)
            {
                break;
            }
        }
    }
    if(CALL_Notification == TRUE)
    {
        ret_val = E_OK;
        /** Call EcuM_CheckWakeup */
        Can_44_TCAN4x5x_IcomWakeup[aId] = (uint8)1;
    }
    return ret_val;
}
#define CAN_44_TCAN4x5x_STOP_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"
#endif

#define CAN_44_TCAN4x5x_START_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"
static FUNC(Std_ReturnType, CAN_44_TCAN4x5x_CODE_FAST) Can_44_TCAN4x5x_SpiWrite
(uint8 aId, uint32* address, uint8 word_count)
{
#if CAN_44_TCAN4x5x_SPI_MSG_DET_LOG == STD_ON
    VAR(Std_ReturnType, AUTOMATIC) RetValue;
    VAR(uint8, AUTOMATIC) ii;

    DETLOG_CAN_44_TCAN4x5x(
              "CAN SPI WRITE  reg:0x%04X, len:%02d, val:",
              (uint32)address,
              word_count);
    for (ii = 1; ii <= word_count; ii++)
    {
        DETLOG_CAN_44_TCAN4x5x(
                  " 0x%08X",
                  Can_44_TCAN4x5x_SpiBuf[aId].SpiTxBuf[Can_44_TCAN4x5x_ExCntxt[aId]][ii]);
    }
    DETLOG_CAN_44_TCAN4x5x( "\r\n");

    RetValue = Can_44_TCAN4x5x_SpiRW(aId, (uint32)address, word_count, CAN_44_TCAN4x5x_WRITE);
    for (ii = 1; ii <= word_count; ii++)
    {
        Can_44_TCAN4x5x_SpiBuf[aId].SpiRxBuf[Can_44_TCAN4x5x_ExCntxt[aId]][ii] = 0;
    }
    (void)Can_44_TCAN4x5x_SpiRW(aId, (uint32)address, word_count,
                                            Can_44_TCAN4x5x_ReadCmd[aId]);
    DETLOG_CAN_44_TCAN4x5x(
              "CAN SPI WREAD  reg:0x%04X, len:%02d, val:",
              (uint32)address,
              word_count);
    for (ii = 1; ii <= word_count; ii++)
    {
        DETLOG_CAN_44_TCAN4x5x(
                  " 0x%08X",
                  Can_44_TCAN4x5x_SpiBuf[aId].SpiRxBuf[Can_44_TCAN4x5x_ExCntxt[aId]][ii]);
    }
    DETLOG_CAN_44_TCAN4x5x( "\r\n");

    return RetValue;
#else
    /** MISRA RULE 11.6 VIOLATION: JUSTIFICATION_4 */
    return (Can_44_TCAN4x5x_SpiRW(aId, (uint32)address, word_count,
                                                    CAN_44_TCAN4x5x_WRITE));
#endif
}
#define CAN_44_TCAN4x5x_STOP_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"

#define CAN_44_TCAN4x5x_START_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"
static FUNC(Std_ReturnType, CAN_44_TCAN4x5x_CODE_FAST) Can_44_TCAN4x5x_SpiRead
(uint8 aId, uint32* address, uint8 word_count)
{
#if CAN_44_TCAN4x5x_SPI_MSG_DET_LOG == STD_ON
    VAR(Std_ReturnType, AUTOMATIC) RetValue;
    VAR(uint8, AUTOMATIC) ii;

    RetValue = Can_44_TCAN4x5x_SpiRW(aId, (uint32)address, word_count,
                                                Can_44_TCAN4x5x_ReadCmd[aId]);
    DETLOG_CAN_44_TCAN4x5x(
              "CAN SPI READ   reg:0x%04X, len:%02d, val:",
              (uint32)address,
              word_count);
    for (ii = 1; ii <= word_count; ii++)
    {
        DETLOG_CAN_44_TCAN4x5x(
                  " 0x%08X",
                  Can_44_TCAN4x5x_SpiBuf[aId].SpiRxBuf[Can_44_TCAN4x5x_ExCntxt[aId]][ii]);
    }
    DETLOG_CAN_44_TCAN4x5x( "\r\n");

    return RetValue;
#else
    /** MISRA RULE 11.6 VIOLATION: JUSTIFICATION_4 */
    return (Can_44_TCAN4x5x_SpiRW(aId, (uint32)address, word_count,
                                  Can_44_TCAN4x5x_ReadCmd[aId]));
#endif
}
#define CAN_44_TCAN4x5x_STOP_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"

#define CAN_44_TCAN4x5x_START_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"
static Std_ReturnType Can_44_TCAN4x5x_SpiRW(uint8 aId, uint32 address,
                                            uint8 word_count, uint8 cmd)
{
    VAR(Std_ReturnType, AUTOMATIC) Return_Val;
    VAR(uint8, AUTOMATIC) NoOfSpiCommRetry = (uint8)0x0;
    volatile VAR(uint32, AUTOMATIC) iterx;
    uint32 *tx_buf_ptr;
    uint32 *rx_buf_ptr;
    uint8 spi_ch_id;
    Spi_SequenceType spi_seq_id;

    tx_buf_ptr = &Can_44_TCAN4x5x_SpiBuf[aId].
                                SpiTxBuf[Can_44_TCAN4x5x_ExCntxt[aId]][0];
    tx_buf_ptr[0] = ((uint32)cmd << 24u) | ((uint32)address << 8u) | word_count;

    // Endianness conversion if required
    for(iterx = 0u; iterx <= (uint32)word_count; iterx++)
    {
        tx_buf_ptr[iterx] = SWAP32(tx_buf_ptr[iterx]);
    }

    rx_buf_ptr = &Can_44_TCAN4x5x_SpiBuf[aId].SpiRxBuf[Can_44_TCAN4x5x_ExCntxt[aId]][0];
    spi_ch_id = Can_44_TCAN4x5x_ControllerSetting_PC[aId].CanSpiChannel[Can_44_TCAN4x5x_ExCntxt[aId]];

    if (cmd == CAN_44_TCAN4x5x_READ_WAIT) { rx_buf_ptr--; word_count++; }
    /** MISRA RULE 11.3 VIOLATION: JUSTIFICATION_3 */
    Return_Val = Spi_SetupEB((Spi_ChannelType)spi_ch_id,
                             (Spi_DataBufferType*)tx_buf_ptr,
                             (Spi_DataBufferType*)rx_buf_ptr,
                             (uint16)((word_count + 1) << 2));

    if (Return_Val == E_OK)
    {
        spi_seq_id = Can_44_TCAN4x5x_ControllerSetting_PC[aId].
                        CanSpiSequenceRef[Can_44_TCAN4x5x_ExCntxt[aId]];
        do
        {
            #ifdef CAN_44_TCAN4x5x_ENABLE_GPIO_INTERRUPT_CONTROL
            /* Call the GpioCall back to disable the interrupts*/
            Can_44_TCAN4x5x_Gpio_Disable_Intr();
            Return_Val = Spi_SyncTransmit(spi_seq_id);
            Can_44_TCAN4x5x_Gpio_Enable_Intr();
            #else
            if((Can_44_TCAN4x5x_ControllerSetting_PC[aId].CanRxProcessing ==
                                                 CAN_44_TCAN4x5x_INTERRUPT) ||
               (Can_44_TCAN4x5x_ControllerSetting_PC[aId].CanTxProcessing ==
                                                 CAN_44_TCAN4x5x_INTERRUPT) ||
               (Can_44_TCAN4x5x_ControllerSetting_PC[aId].CanBusoffProcessing ==
                                                 CAN_44_TCAN4x5x_INTERRUPT))
            {
                SchM_Enter_Can_44_TCAN4x5x_Exclusive_Area_06();
                Return_Val = Spi_SyncTransmit(spi_seq_id);
                SchM_Exit_Can_44_TCAN4x5x_Exclusive_Area_06();
            }
            else
            {
                Return_Val = Spi_SyncTransmit(spi_seq_id);
            }
            #endif
            if(Return_Val != E_OK)
            {
                for(iterx = 0u; iterx < Can_44_TCAN4x5x_ConfigPtr->
                                                CanSpiCommRetryDelay; iterx++)
                {
                    ;
                }
            }
            else
            {
                // Endianness conversion if required
                for(iterx = 0u; iterx <= (uint32)word_count; iterx++)
                {
                    rx_buf_ptr[iterx] = SWAP32(rx_buf_ptr[iterx]);
                }
            }
            NoOfSpiCommRetry++;
        }while((Return_Val == E_NOT_OK) && (NoOfSpiCommRetry <=
                                Can_44_TCAN4x5x_ConfigPtr->CanSpiCommRetries));
    }
    return(Return_Val);
}
#define CAN_44_TCAN4x5x_STOP_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"

#define CAN_44_TCAN4x5x_START_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"
static FUNC(Std_ReturnType, CAN_44_TCAN4x5x_CODE_FAST) Can_44_TCAN4x5x_VerifySpiRead
(uint8 aId, uint32* address) {
    Std_ReturnType ret_val = E_OK;
        if(Can_44_TCAN4x5x_MajorRevId == (uint8)0x3){
            if (Can_44_TCAN4x5x_SpiBuf[aId].SpiRxBuf[Can_44_TCAN4x5x_ExCntxt[aId]][1] == (uint32)0)
            {
                if((&Can_44_TCAN4x5x_MCAN_REGS(aId)->NDAT1) == address)
                {
                    if(Can_44_TCAN4x5x_SpiBuf[aId].SpiRxBuf[Can_44_TCAN4x5x_ExCntxt[aId]][2] == (uint32)0)
                    {
                        (void)Can_44_TCAN4x5x_SpiRead((uint8)aId, address, 2);
                    }
                }
                else
                {
                    (void)Can_44_TCAN4x5x_SpiRead((uint8)aId, address, 1);
                }
            }
        }

    return ret_val;
}
#define CAN_44_TCAN4x5x_STOP_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"


#ifdef CAN_44_TCAN4x5x_ENABLE_GPIO_INTERRUPT_CONTROL
#define CAN_44_TCAN4x5x_START_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"
STATIC FUNC(void, CAN_44_TCAN4x5x_CODE_FAST) Can_44_TCAN4x5x_Gpio_Disable_Intr()
{
    if((Can_44_TCAN4x5x_ControllerSetting_PC[0u].CanRxProcessing ==
                                                 CAN_44_TCAN4x5x_INTERRUPT) ||
      (Can_44_TCAN4x5x_ControllerSetting_PC[0u].CanTxProcessing ==
                                                 CAN_44_TCAN4x5x_INTERRUPT) ||
      (Can_44_TCAN4x5x_ControllerSetting_PC[0u].CanBusoffProcessing ==
                                                 CAN_44_TCAN4x5x_INTERRUPT))
    {
        /* Disable the interrupt on pin [11] in GPIO7_IRQSTATUS_CLR_0*/
        *((uint32*)0x4805103C) |= 0x00000800u;
    }
}
#define CAN_44_TCAN4x5x_STOP_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"
#endif

#ifdef CAN_44_TCAN4x5x_ENABLE_GPIO_INTERRUPT_CONTROL
#define CAN_44_TCAN4x5x_START_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"
STATIC FUNC(void, CAN_44_TCAN4x5x_CODE_FAST) Can_44_TCAN4x5x_Gpio_Enable_Intr()
{
    if((Can_44_TCAN4x5x_ControllerSetting_PC[0u].CanRxProcessing ==
                                                 CAN_44_TCAN4x5x_INTERRUPT) ||
      (Can_44_TCAN4x5x_ControllerSetting_PC[0u].CanTxProcessing ==
                                                 CAN_44_TCAN4x5x_INTERRUPT) ||
      (Can_44_TCAN4x5x_ControllerSetting_PC[0u].CanBusoffProcessing ==
                                                 CAN_44_TCAN4x5x_INTERRUPT))
    {
        /* Enable the interrupt on pin [11] in GPIO7_IRQSTATUS_SET_0*/
        *((uint32*)0x48051034) |= 0x00000800u;
    }
}
#define CAN_44_TCAN4x5x_STOP_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"
#endif

#define CAN_44_TCAN4x5x_START_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"
FUNC(uint16, CAN_44_TCAN4x5x_CODE_FAST) Can_44_TCAN4x5x_GetCurrentTimeStampCounterValue(VAR(uint8, AUTOMATIC) aId)
{
    P2VAR(uint32, AUTOMATIC, CAN_44_TCAN4x5x_APPL_DATA) TSCV_ptr = &(Can_44_TCAN4x5x_MCAN_REGS(aId)->TSCV);
    P2VAR(uint32, AUTOMATIC, CAN_44_TCAN4x5x_APPL_DATA) spi_rx_buf_ptr;

    spi_rx_buf_ptr = &Can_44_TCAN4x5x_SpiBuf[aId].SpiRxBuf[Can_44_TCAN4x5x_ExCntxt[aId]][1];
    /** Read the register */
    (void)Can_44_TCAN4x5x_SpiRead(aId, TSCV_ptr, (uint8)1);
    return spi_rx_buf_ptr[0];
}
#define CAN_44_TCAN4x5x_STOP_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"

#define CAN_44_TCAN4x5x_START_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"
FUNC(uint16, CAN_44_TCAN4x5x_CODE_FAST) Can_44_TCAN4x5x_GetCurrentTxTimeStamp(VAR(uint8, AUTOMATIC) aId)
{
    return Can_44_TCAN4x5x_CurrentTxTimeStamp[aId];
}
#define CAN_44_TCAN4x5x_STOP_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"

/*******************************************************************************
*                       VERSION CHECK
*******************************************************************************/
/** \brief SW version check of *.h/c of the own module */
#if ((CAN_44_TCAN4x5x_SW_MAJOR_VERSION_MCAN_C != \
        CAN_44_TCAN4x5x_SW_MAJOR_VERSION) || \
        (CAN_44_TCAN4x5x_SW_MINOR_VERSION_MCAN_C != \
        CAN_44_TCAN4x5x_SW_MINOR_VERSION) ||\
        (CAN_44_TCAN4x5x_SW_PATCH_VERSION_MCAN_C != \
        CAN_44_TCAN4x5x_SW_PATCH_VERSION))
        #error "The SW version number of Can_44_TCAN4x5x_MCan.c and \
                Can_44_TCAN4x5x.h are different."
#endif

#if ((CAN_44_TCAN4x5x_SW_MAJOR_VERSION_MCAN_C != \
        CAN_44_TCAN4x5x_SW_MAJOR_VERSION_MCAN_H) || \
        (CAN_44_TCAN4x5x_SW_MINOR_VERSION_MCAN_C != \
        CAN_44_TCAN4x5x_SW_MINOR_VERSION_MCAN_H) ||\
        (CAN_44_TCAN4x5x_SW_PATCH_VERSION_MCAN_C != \
        CAN_44_TCAN4x5x_SW_PATCH_VERSION_MCAN_H))
    #error "The SW version number of Can_44_TCAN4x5x_MCan.c and \
            Can_44_TCAN4x5x_MCan.h are different."
#endif

#ifdef __cplusplus
}
#endif
/*******************************************************************************
    Version     Date            Author           Description
    -------     ----            ------           -----------
    1.0.0    17-05-2017       Lakshmi H        Initial Version
	1.0.1    07-08-2017       Lakshmi H        Performed QAC Analysis and
											   justification is given for Misra
											   Rule voilations
    1.0.2    28-08-2017       Lakshmi H        Critical section is added for
                                               Register 0x0800 and
                                               Commenting style is changed to
											   Doxygen style
    1.0.3    17-11-2017       Lakshmi H       TCAN1043 code added and Software
                                               version changed
 ******************************************************************************/
/*******************************************************************************
 *  End of File: Can_44_TCAN4x5x_MCan.c
 ******************************************************************************/
