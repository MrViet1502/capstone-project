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
 *  \file     Can_44_TCAN4x5x.h
 *
 *  \brief    This file contains interface header for CAN MCAL driver
 *
 */

#ifndef CAN_44_TCAN4x5x_H

#define CAN_44_TCAN4x5x_H

#ifdef __cplusplus
extern "C"{
#endif
/*******************************************************************************
 *                      INCLUDE FILES
 ******************************************************************************/

/** \brief Includes all types and constants that are shared among the
*   AUTOSAR CAN modules Can and CanIf
*/
#include "Can_GeneralTypes.h"
#include "Can_44_TCAN4x5x_Cfg.h"
#if (CAN_44_TCAN4x5x_PUBLIC_ICOM_SUPPORT == STD_ON)
#include "EcuM_Types.h"
#endif
#include "Spi.h"

/*******************************************************************************
 *                      EXPORTED CONSTANTS and MACROS
 ******************************************************************************/
/** [SWS_BSW_00059] Published information elements. */
#define CAN_44_TCAN4x5x_VENDOR_ID   44u
#define CAN_44_TCAN4x5x_MODULE_ID   80u

#define CAN_44_TCAN4x5x_AR_RELEASE_MAJOR_VERSION     4u
#define CAN_44_TCAN4x5x_AR_RELEASE_MINOR_VERSION     2u
#define CAN_44_TCAN4x5x_AR_RELEASE_REVISION_VERSION  1u

#define CAN_44_TCAN4x5x_SW_MAJOR_VERSION     2u
#define CAN_44_TCAN4x5x_SW_MINOR_VERSION     3u
#define CAN_44_TCAN4x5x_SW_PATCH_VERSION     0u

#define CAN_44_TCAN4x5x_VENDOR_ID_C   44u
#define CAN_44_TCAN4x5x_MODULE_ID_C   80u
#define CAN_44_TCAN4x5x_INSTANCE_ID   CAN_44_TCAN4x5x_INDEX

/**  Type of Hardware Object */
#define CAN_44_TCAN4x5x_HW_OBJ_TYPE_BASIC (0U)
#define CAN_44_TCAN4x5x_HW_OBJ_TYPE_FULL (1U)

#define CAN_44_TCAN4x5x_POLLING    0u
#define CAN_44_TCAN4x5x_INTERRUPT  1u

/** Object Type of Hardware Object */
#define CAN_44_TCAN4x5x_RECEIVE (0U)
#define CAN_44_TCAN4x5x_TRANSMIT (1U)

/** CanId Type of Hardware Object */
#define CAN_44_TCAN4x5x_ID_TYPE_EXTENDED (1U)
#define CAN_44_TCAN4x5x_ID_TYPE_MIXED    (1U)
#define CAN_44_TCAN4x5x_ID_TYPE_STANDARD (0U)
#define CAN_44_TCAN4x5x_ID_TYPE_MASK     (0x80000000UL)
#define CAN_44_TCAN4x5x_FRAME_TYPE_MASK  (0x40000000UL)

#define CAN_44_TCAN4x5x_EXTD_ID_MASK     (0x80000000UL)
#define CAN_44_TCAN4x5x_FD_FRAME_MASK    (0x40000000UL)

/** Values for CANFD */
#define CAN_44_TCAN4x5x_FD_DATA_BUFFER_SIZE_8        (0U)
#define CAN_44_TCAN4x5x_FD_DATA_BUFFER_SIZE_12       (1U)
#define CAN_44_TCAN4x5x_FD_DATA_BUFFER_SIZE_16       (2U)
#define CAN_44_TCAN4x5x_FD_DATA_BUFFER_SIZE_20       (3U)
#define CAN_44_TCAN4x5x_FD_DATA_BUFFER_SIZE_24       (4U)
#define CAN_44_TCAN4x5x_FD_DATA_BUFFER_SIZE_32       (5U)
#define CAN_44_TCAN4x5x_FD_DATA_BUFFER_SIZE_48       (6U)
#define CAN_44_TCAN4x5x_FD_DATA_BUFFER_SIZE_64       (7U)

/** Values for CAN Driver states. */
/** CAN Driver is NOT initialized. */
#define  CAN_44_TCAN4x5x_UNINIT                  ((uint8)0)
/** CAN Driver is initialized. */
#define  CAN_44_TCAN4x5x_READY                   ((uint8)1)

/**  Service IDs. Used while reporting development errors */
#define CAN_44_TCAN4x5x_SID_INIT                        ((uint8)0x00)
#define CAN_44_TCAN4x5x_SID_MAINFUNCTION_WRITE          ((uint8)0x01)
#define CAN_44_TCAN4x5x_SID_SETCONTROLLERMODE           ((uint8)0x03)
#define CAN_44_TCAN4x5x_SID_DISABLECONTROLLERINTERRUPTS ((uint8)0x04)
#define CAN_44_TCAN4x5x_SID_ENABLECONTROLLERINTERRUPTS  ((uint8)0x05)
#define CAN_44_TCAN4x5x_SID_WRITE                       ((uint8)0x06)
#define CAN_44_TCAN4x5x_SID_GETVERSIONINFO              ((uint8)0x07)
#define CAN_44_TCAN4x5x_SID_MAINFUNCTION_READ           ((uint8)0x08)
#define CAN_44_TCAN4x5x_SID_MAINFUNCTION_BUSOFF         ((uint8)0x09)
#define CAN_44_TCAN4x5x_SID_MAINFUNCTION_WAKEUP         ((uint8)0x0a)
#define CAN_44_TCAN4x5x_SID_CHECKWAKEUP                 ((uint8)0x0b)
#define CAN_44_TCAN4x5x_SID_MAINFUNCTION_MODE           ((uint8)0x0c)
#define CAN_44_TCAN4x5x_SID_SET_BAUDRATE                ((uint8)0x0f)
#define CAN_44_TCAN4x5x_SET_ICOMCONFIGURATION_INIT      ((uint8)0x1f)

#define CAN_44_TCAN4x5x_E_ICOM_CONFIG_INVALID           ((uint8)0x2f)

#if (CAN_44_TCAN4x5x_DEV_ERROR_DETECT == STD_ON)

/** API Service called with wrong parameter: Config is a null pointer */
#define CAN_44_TCAN4x5x_E_PARAM_POINTER    ((uint8)1)
/** API called with wrong parameter: Handle parameter is not a
   configured Hardware Transmit Handle */
#define CAN_44_TCAN4x5x_E_PARAM_HANDLE     ((uint8)2)
/** API called with wrong parameter : Parameter Data length is
  more than 8 bytes   */
#define CAN_44_TCAN4x5x_E_PARAM_DLC        ((uint8)3)
/** API called with wrong parameter : Parameter Controller is out of range  */
#define CAN_44_TCAN4x5x_E_PARAM_CONTROLLER ((uint8)4)
/** Invalid invalid transition requested OR Invalid CAN controller state for
   API service call */
#define CAN_44_TCAN4x5x_E_TRANSITION       ((uint8)6)
/** DET for Over write error in receive ISR */
#define CAN_44_TCAN4x5x_E_DATALOST         ((uint8)7)
/** API called with invalid / unsupported baudrate */
#define CAN_44_TCAN4x5x_E_PARAM_BAUDRATE   ((uint8)8)

 /** API Service called with wrong */
#define CAN_44_TCAN4x5x_E_INIT_FAILED                        ((uint8)9)
#define CAN_44_TCAN4x5x_E_INVALID_NOMINAL_BITTIMING_PARAM    ((uint8)0x0a)
#define CAN_44_TCAN4x5x_E_INVALID_FILTER_OBJECTS_POINTER     ((uint8)0x0b)
#define CAN_44_TCAN4x5x_E_INVALID_DATA_BITTIMING_PARAM       ((uint8)0x0c)
#define CAN_44_TCAN4x5x_E_RX_BUF_FULL                        ((uint8)0x0d)
#define CAN_44_TCAN4x5x_E_NO_RX_BUFFER_AVAIL                 ((uint8)0x0e)
#define CAN_44_TCAN4x5x_E_TX_EVENT_FIFO_ELEMENT_LOST         ((uint8)0x0f)
#define CAN_44_TCAN4x5x_E_SETBAUDRATE_FAILED                 ((uint8)0x1a)
#define CAN_44_TCAN4x5x_E_SETMODE_FAILED                     ((uint8)0x2a)
#define CAN_44_TCAN4x5x_E_INVALID_PARAMETER                  ((uint8)0x6a)
#define CAN_44_TCAN4x5x_E_TX_BUFFER_BUSY                     ((uint8)0x7a)
//#define CAN_44_TCAN4x5x_E_DEINIT                             ((uint8)0x8a)
#define CAN_44_TCAN4x5x_E_INVALID_ARGUMENTS                  ((uint8)0xba)
#endif // #if (CAN_44_TCAN4x5x_DEV_ERROR_DETECT == STD_ON)


/** Link time configuration parameters */
#define CAN_44_TCAN4x5x_MSG_FILTER_RANGE                (0UL)
#define CAN_44_TCAN4x5x_MSG_FILTER_DUAL                 (1UL)
#define CAN_44_TCAN4x5x_MSG_FILTER_CLASSIC              (2UL)
#define CAN_44_TCAN4x5x_MSG_FILTER_DISABLED             (3UL)

/** Wakeup Status*/
#define CAN_44_TCAN4x5x_ONE                             (1U)
#define CAN_44_TCAN4x5x_INIT_WAIT_TIMEOUT               (10)

#define CAN_44_TCAN4x5x_TCAN4x5x                        0u
#define CAN_44_TCAN4x5x_TCAN1043                        1u

#define CAN_44_TCAN4x5x_START_SEC_VAR_CLEARED_8
#include "Can_44_TCAN4x5x_MemMap.h"

extern volatile VAR(uint8, CAN_44_TCAN4x5x_VAR_CLEARED) Can_44_TCAN4x5x_WasCanTrcvInSleep;

#define CAN_44_TCAN4x5x_STOP_SEC_VAR_CLEARED_8
#include "Can_44_TCAN4x5x_MemMap.h"

#if (CAN_44_TCAN4x5x_DEV_ERROR_DETECT == STD_ON)
#define CAN_44_TCAN4x5x_START_SEC_VAR_CLEARED_8
#include "Can_44_TCAN4x5x_MemMap.h"

/** Variable to store the ApiID to report Det error */
extern VAR(uint8, CAN_44_TCAN4x5x_VAR_CLEARED) Can_44_TCAN4x5x_ApiId;

#define CAN_44_TCAN4x5x_STOP_SEC_VAR_CLEARED_8
#include "Can_44_TCAN4x5x_MemMap.h"
#endif /** CAN_44_TCAN4x5x_DEV_ERROR_DETECT == STD_ON */

#define CAN_44_TCAN4x5x_START_SEC_VAR_CLEARED_8
#include "Can_44_TCAN4x5x_MemMap.h"
/** Variable - Can_44_TCAN4x5x_ExCntxt */
extern volatile uint8
                Can_44_TCAN4x5x_ExCntxt[CAN_44_TCAN4x5x_NUM_CONTROLLER_MAX];
#define CAN_44_TCAN4x5x_STOP_SEC_VAR_CLEARED_8
#include "Can_44_TCAN4x5x_MemMap.h"

/*******************************************************************************
 *                      EXPORTED STRUCTURES, ENUMS, TYPEDEFS
 ******************************************************************************/
/** Type for CAN Driver states */
/** CAN_DesignId_014 */
typedef uint8 Can_44_TCAN4x5x_DriverStateType;

/** Type to reperesent identifier type of an L-PDU */
/** CAN_DesignId_015 */
typedef uint8 Can_44_TCAN4x5x_FrameType;

/** Type of CanIcomSignalOperation */
/** CAN_DesignId_016 */
typedef enum
{
    CAN_44_TCAN4x5x_ICOM_SIG_OP_AND = 0u,
    CAN_44_TCAN4x5x_ICOM_SIG_OP_EQUAL,
    CAN_44_TCAN4x5x_ICOM_SIG_OP_GREATER,
    CAN_44_TCAN4x5x_ICOM_SIG_OP_SMALLER,
    CAN_44_TCAN4x5x_ICOM_SIG_OP_XOR
}Can_44_TCAN4x5x_IcomSignalOperationType;

/** Type of CanMessageBufferType */
#define CAN_44_TCAN4x5x_BUF_TYPE_FIFO0  (1u)
#define CAN_44_TCAN4x5x_BUF_TYPE_FIFO1  (2u)
#define CAN_44_TCAN4x5x_BUF_TYPE_DEDICATED_RX_BUFFER (7u)

/** CanFD Baudrate Configuration Parameters */
/** CAN_DesignId_017 */
typedef struct
{
    /** \brief CanController CanFDControllerBRP */
    uint8 CanFDControllerBRP;

    /** \brief CanController FD PropSeg */
    uint8 CanControllerDataPropSeg;

    /** \brief CanController FD Seg1 */
    uint8 CanControllerDataSeg1;

    /** \brief CanController FD Seg2 */
    uint8 CanControllerDataSeg2;

    /** \brief CanController FD SyncJumpWidth */
    uint8 CanControllerDataSyncJumpWidth;

    /** \brief CanControllerTrcvDelayCompensationOffset CanFD*/
    uint8 CanControllerDataTrcvDelayCompensationOffset;

    /** \brief CanController FD TxBitRateSwitch */
    uint8 CanControllerDataTxBitRateSwitch;

} Can_44_TCAN4x5x_ControllerFdBaudrateConfigType;


/** Baudrate Configuration Parameters */
/** CAN_DesignId_018 */
typedef struct
{
    /** \brief CanController CanControllerBRP */
    uint8 CanControllerBRP;

    /** \brief CanController PropSeg */
    uint8 CanControllerPropSeg;

    /** \brief CanController Seg1 */
    uint8 CanControllerSeg1;

    /** \brief CanController Seg2 */
    uint8 CanControllerSeg2;

    /** \brief CanController SyncJumpWidth */
    uint8 CanControllerSyncJumpWidth;

    /** \brief Pointer to CanFD Configurations */
    const Can_44_TCAN4x5x_ControllerFdBaudrateConfigType
                                                *CanControllerFdBaudrateConf;

} Can_44_TCAN4x5x_ControllerBaudrateConfigType;

/** CAN Hardware Objects of Rx - CanObjectId Mapping */
/** CAN_DesignId_019 */
typedef uint8 Can_44_TCAN4x5x_FilterIdxToRxObjMappingType;

/** CAN Hardware Objects (HRH) - HwFilters Configurations */
/** CAN_DesignId_020 */
typedef struct
{
    /** \brief CanIdType */
    uint8 CanIdType;

    /** \brief CanFilterType */
    uint8 CanFilterType;

    /** \brief CanMsgBufferType */
    uint8 CanMessageBufferType;

    /** \brief CanHwFilterCode */
    uint32 CanHwFilterCode;

    /** \brief CanHwFilterMask */
    uint32 CanHwFilterMask;


} Can_44_TCAN4x5x_RxObjFilterConfigType;

/** CAN Hardware Tx Objects (Can_44_TCAN4x5x_TxObj) Configurations */
/** CAN_DesignId_021 */
typedef struct
{
    /** \brief CanFdPaddingValue */
    uint8 CanFdPaddingValue;

    /** \brief canHwObjectCount */
    uint8 CanHwObjectCount;

    /** \brief CanTxBufferIndex */
    uint8 CanTxBufferIndex;

    /** \brief Trigger Transmit */
    uint8 CanTriggerTransmitEnable;


} Can_44_TCAN4x5x_TxObjConfigType;

/** Message RAM Configuration*/
/** CAN_DesignId_022 */
typedef struct
{
    /** \brief Maximum HwFilters for Standard Id of RX objects */
    uint8 CanStdandardIDFiltersCount;

    /** \brief Maximum HwFilters for Extended Id of RX objects */
    uint8 CanExtendedIDFiltersCount;

    /** \brief Maximum Rx Fifo0 Count */
    uint8 CanFifo0BufferCount;

    /** \brief Maximum Rx Fifo1 Count */
    uint8 CanFifo1BufferCount;

    /** \brief Maximum Dedicated Rx Buffer Count */
    uint8 CanDedicatedRxBufferCount;

    /** \brief Maximum Tx Buffer Count */
    uint8 CanTxBufferCount;
}Can_44_TCAN4x5x_MsgRamConfigType;

/** Type of CAN external data structure containing the overall initialization
   data for the CAN Driver and settings affecting all controllers */
/** CAN_DesignId_023 */
   typedef struct
{

    /** \brief CanControllerDefaultBaudrate */
    uint8 CanControllerDefaultBaudrate;

    /** \brief CAN Message RAM Configurations */
    Can_44_TCAN4x5x_MsgRamConfigType CanMsgRamConfig;

    /** \brief Number of baudrates configured */
    uint8 CanNumOfBaudRateConf;

    /** \brief Pointer to CAN Baudrate Configurations */
    const Can_44_TCAN4x5x_ControllerBaudrateConfigType
                                            *CanControllerBaudrateConfigPtr;

    /** \brief Pointer to CAN Hardware Objects (HRH) Configurations - ROM copy used as the base for loading CanRxObjFilterConfigPtr with dynamic IDs */
    const Can_44_TCAN4x5x_RxObjFilterConfigType *CanRxObjFilterBaseConfigPtr;

    /** \brief Pointer to CAN Hardware Objects (HRH) Configurations */
    Can_44_TCAN4x5x_RxObjFilterConfigType *CanRxObjFilterConfigPtr;

    /** \brief Pointer to CAN Hardware Objects (HTH) Configurations */
    const Can_44_TCAN4x5x_TxObjConfigType *CanTxObjConfigPtr;

    /** \brief Pointer to CAN Hardware Objects CAN_44_TCAN4x5x_FilterIdx to
                                                         RxObjMap(Standard) */
    const Can_44_TCAN4x5x_FilterIdxToRxObjMappingType
                                                *CanFilterIdxToRxObjStandard;

    /** \brief Pointer to CAN Hardware Objects CAN_44_TCAN4x5x_FilterIdx to
                                                    RxObjMap(Extended) */
    const Can_44_TCAN4x5x_FilterIdxToRxObjMappingType
                                                *CanFilterIdxToRxObjExtended;

	/** \brief SpiBaudrate */
    uint32 CanSpiBaudrateRef;

} Can_44_TCAN4x5x_ControllerConfigType;

/** HTH Mapping Configuration*/
/** CAN_DesignId_024 */
typedef struct
{
    /** \brief Can Controller Id */
    uint8 CanCtrlId;
	/** \brief Can Index */
    uint8 CanHthIndex;

}Can_44_TCAN4x5x_HTHMappingConfigType;

/** CAN Hardware Configuration*/
/** CAN_DesignId_025 */
typedef struct
{
    /** \brief HTH size */
    uint16 CanHTHobjectsCount;

    /** \brief HRH size */
    uint16 CanHRHobjectsCount;

	/** \brief Time between retries */
    uint32 CanSpiCommRetryDelay;

	/** \brief maximum number of communication retries in case of a failed SPI communication. */
	uint8 CanSpiCommRetries;

    /** \brief Pointer to CAN HTH Mapping Configurations */
    const Can_44_TCAN4x5x_HTHMappingConfigType
                                            *CanHTHMappingConfigPtr;
	/** \brief Pointer to CAN Controller */
    const Can_44_TCAN4x5x_ControllerConfigType *CanControllerConfiguration;

} Can_44_TCAN4x5x_ConfigType;

/** CAN Controller Configuration */
/** CAN_DesignId_026 */
typedef struct
{
    /** \brief Crystal clock */
    uint32 CanXtalClock;
    /** \brief Reference to Spi Sequence for API's */
    Spi_SequenceType CanSpiSequenceRef[3];
	/** \brief Reference to Spi Channel for API's */
    uint8 CanSpiChannel[3];
	/** \brief Can FD Support */
	uint8 CanFDBaudRateEnabled;
	/** \brief Can FD Data Buffer Size*/
	uint8 CanFdDataBufSize;
	/** \brief CanBusoffProcessing */
	uint8 CanBusoffProcessing;
	/** \brief CanRxProcessing */
	uint8 CanRxProcessing;
	/** \brief CanTxProcessing */
	uint8 CanTxProcessing;
    #if (CAN_44_TCAN4x5x_PUBLIC_ICOM_SUPPORT == STD_ON)
    /** \brief Wakeup source used for ICOM wakeup */
	EcuM_WakeupSourceType CanWakeupRef;
    #endif
} Can_44_TCAN4x5x_ControllerConfigType_PC;

/** \brief Extern declarations of CAN configuration */
CAN_44_TCAN4x5x_CFG_EXTERN_DECLARATIONS


#if (CAN_44_TCAN4x5x_PUBLIC_ICOM_SUPPORT == STD_ON)

/** CAN ICOM Configurations - CAN_44_TCAN4x5x_IcomRxMessagesignals */
/** CAN_DesignId_027 */
typedef struct
{
    /** \brief CanIcomSignalMask */
    uint64 CanIcomSignalMask;

    /** \brief CanIcomSignalOperation */
    Can_44_TCAN4x5x_IcomSignalOperationType CanIcomSignalOperation;

    /** \brief CanIcomSignalValue */
    uint64 CanIcomSignalValue;

} Can_44_TCAN4x5x_IcomRxMessageSignalConfigType;


/** CAN ICOM Configurations - CAN_44_TCAN4x5x_IcomRxMessages */
/** CAN_DesignId_028 */
typedef struct
{
    /** \brief CanIcomCounterValue */
    #if (CAN_44_TCAN4x5x_ICOM_COUNTER_VALUE == STD_ON)
    uint16 CanIcomCounterValue;
    #endif

    /** \brief Pointer to CAN ICOM - RxMsg Configurations */
    const Can_44_TCAN4x5x_RxObjFilterConfigType *CanRxObjFilterIComConfigPtr;

    /** \brief CanIcomMissingMessageTimerValue */
    #if (CAN_44_TCAN4x5x_ICOM_MISSING_MESSAGE_TIMER_VALUE == STD_ON)
    uint32 CanIcomMissingMessageTimerValue;
    #endif

    /** \brief CanIcomPayloadLengthError */
    boolean CanIcomPayloadLengthError;

    /** \brief CanIcomRxMessageSignalConfigs Size */
    uint8 CanIcomRxMessageSignalConfig_size;

    /** \brief CAN_44_TCAN4x5x_IcomRxMessagesignalconfig Pointer */
    #if (CAN_44_TCAN4x5x_ICOM_RX_MESSAGE_SINAL_CONFIGURATION == STD_ON)
    const Can_44_TCAN4x5x_IcomRxMessageSignalConfigType
                                            *CanIcomRxMessageSignalConfigPtr;
    #endif

} Can_44_TCAN4x5x_IcomRxMessageType;

/** CAN ICOM Configurations - CAN_44_TCAN4x5x_IcomRxMessagesignals */
/** CAN_DesignId_029 */
typedef struct
{
    /** \brief canIcomWakeOnBusOff */
    boolean canIcomWakeOnBusOff;

    /** \brief canIcomRxMessages Size */
    uint8 canIcomRxMessages_size;

    /** \brief CAN_44_TCAN4x5x_IcomRxMessage Pointer */
    const Can_44_TCAN4x5x_IcomRxMessageType *CanIcomRxMessagePtr;

} Can_44_TCAN4x5x_IcomConfigType;

#endif

#if (CAN_44_TCAN4x5x_PUBLIC_ICOM_SUPPORT == STD_ON)
/** \brief Can Icom Structure Accessing Declaration*/
extern CONST(Can_44_TCAN4x5x_IcomConfigType, CAN_44_TCAN4x5x_CONFIG_DATA)
                                                Can_44_TCAN4x5x_Icomconfig[];
#endif

#define CAN_44_TCAN4x5x_START_SEC_CONFIG_DATA_UNSPECIFIED
#include "Can_44_TCAN4x5x_MemMap.h"

extern CONST(Can_44_TCAN4x5x_ControllerConfigType_PC,
             CAN_44_TCAN4x5x_CONFIG_DATA)
      Can_44_TCAN4x5x_ControllerSetting_PC[CAN_44_TCAN4x5x_NUM_CONTROLLER_MAX];

#define CAN_44_TCAN4x5x_STOP_SEC_CONFIG_DATA_UNSPECIFIED
#include "Can_44_TCAN4x5x_MemMap.h"


#define CAN_44_TCAN4x5x_START_SEC_VAR_SLOW_CLEARED_8
#include "Can_44_TCAN4x5x_MemMap.h"
/** This flag stores the CAN driver state */
extern VAR(Can_44_TCAN4x5x_DriverStateType, CAN_44_TCAN4x5x_VAR_SLOW_CLEARED)
                                                    Can_44_TCAN4x5x_DriverState;

#define CAN_44_TCAN4x5x_STOP_SEC_VAR_SLOW_CLEARED_8
#include "Can_44_TCAN4x5x_MemMap.h"


#define CAN_44_TCAN4x5x_START_SEC_VAR_CLEARED_UNSPECIFIED
#include "Can_44_TCAN4x5x_MemMap.h"

/** State transitions that are used by the function CanIf_SetControllerMode*/
extern VAR(Can_StateTransitionType, CAN_44_TCAN4x5x_VAR_FAST_CLEARED)
             Can_44_TCAN4x5x_ControllerMode[CAN_44_TCAN4x5x_NUM_CONTROLLER_MAX];

#define CAN_44_TCAN4x5x_STOP_SEC_VAR_CLEARED_UNSPECIFIED
#include "Can_44_TCAN4x5x_MemMap.h"

#define CAN_44_TCAN4x5x_START_SEC_VAR_CLEARED_UNSPECIFIED
#include "Can_44_TCAN4x5x_MemMap.h"

extern P2CONST(Can_44_TCAN4x5x_ConfigType, CAN_44_TCAN4x5x_VAR_CLEARED,
                          CAN_44_TCAN4x5x_APPL_DATA)Can_44_TCAN4x5x_ConfigPtr;

#define CAN_44_TCAN4x5x_STOP_SEC_VAR_CLEARED_UNSPECIFIED
#include "Can_44_TCAN4x5x_MemMap.h"
/*******************************************************************************
 *                      EXPORTED FUNCTIONS
 ******************************************************************************/
#define CAN_44_TCAN4x5x_START_SEC_CODE_SLOW
#include "Can_44_TCAN4x5x_MemMap.h"

extern FUNC(void, CAN_44_TCAN4x5x_CODE_SLOW) Can_44_TCAN4x5x_Init
(P2CONST(Can_44_TCAN4x5x_ConfigType, AUTOMATIC, CAN_44_TCAN4x5x_APPL_DATA)
                                                                    Config);

#define CAN_44_TCAN4x5x_STOP_SEC_CODE_SLOW
#include "Can_44_TCAN4x5x_MemMap.h"

#if(CAN_44_TCAN4x5x_VERSION_INFO_API == STD_ON)
#define CAN_44_TCAN4x5x_START_SEC_CODE_SLOW
#include "Can_44_TCAN4x5x_MemMap.h"
extern FUNC(void, CAN_44_TCAN4x5x_CODE_SLOW) Can_44_TCAN4x5x_GetVersionInfo
(P2VAR(Std_VersionInfoType, AUTOMATIC, CAN_44_TCAN4x5x_APPL_DATA) versioninfo);

#define CAN_44_TCAN4x5x_STOP_SEC_CODE_SLOW
#include "Can_44_TCAN4x5x_MemMap.h"
#endif

#if(CAN_44_TCAN4x5x_SET_BAUDRATE_API == STD_ON)
#define CAN_44_TCAN4x5x_START_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"
extern FUNC(Std_ReturnType, CAN_44_TCAN4x5x_CODE_FAST)
Can_44_TCAN4x5x_SetBaudrate(VAR(uint8, AUTOMATIC) Controller,
                                VAR(uint16, AUTOMATIC) BaudRateConfigID);
#define CAN_44_TCAN4x5x_STOP_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"
#endif

#define CAN_44_TCAN4x5x_START_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"
extern FUNC(Can_ReturnType, CAN_44_TCAN4x5x_CODE_FAST)
Can_44_TCAN4x5x_SetControllerMode (VAR(uint8, AUTOMATIC) Controller,
                VAR(Can_StateTransitionType, AUTOMATIC) Transition);
#define CAN_44_TCAN4x5x_STOP_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"

#define CAN_44_TCAN4x5x_START_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"
extern FUNC(void, CAN_44_TCAN4x5x_CODE_FAST)
Can_44_TCAN4x5x_DisableControllerInterrupts (VAR(uint8, AUTOMATIC) Controller);
#define CAN_44_TCAN4x5x_STOP_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"

#define CAN_44_TCAN4x5x_START_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"
extern FUNC(void, CAN_44_TCAN4x5x_CODE_FAST)
Can_44_TCAN4x5x_EnableControllerInterrupts(VAR(uint8, AUTOMATIC) Controller);
#define CAN_44_TCAN4x5x_STOP_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"

#define CAN_44_TCAN4x5x_START_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"
extern FUNC(Can_ReturnType, CAN_44_TCAN4x5x_CODE_FAST)
Can_44_TCAN4x5x_Write(VAR(Can_HwHandleType, AUTOMATIC) Hth,
P2CONST(Can_PduType, AUTOMATIC, CAN_44_TCAN4x5x_APPL_DATA) PduInfo);
#define CAN_44_TCAN4x5x_STOP_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"

#if(CAN_44_TCAN4x5x_WAKEUP_FUNCTIONALITY_API == STD_ON)
#define CAN_44_TCAN4x5x_START_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"
extern FUNC(Can_ReturnType, CAN_44_TCAN4x5x_CODE_FAST)
                Can_44_TCAN4x5x_CheckWakeup(VAR(uint8, AUTOMATIC) Controller);
#define CAN_44_TCAN4x5x_STOP_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"
#endif

#if (CAN_44_TCAN4x5x_PUBLIC_ICOM_SUPPORT == STD_ON)
#define CAN_44_TCAN4x5x_START_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"
extern FUNC(Std_ReturnType, CAN_44_TCAN4x5x_CODE_FAST)
       Can_44_TCAN4x5x_SetIcomConfiguration (VAR(uint8, AUTOMATIC) Controller,
                            VAR(IcomConfigIdType, AUTOMATIC) ConfigurationId);

#define CAN_44_TCAN4x5x_STOP_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"
#endif

#define CAN_44_TCAN4x5x_START_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"
extern FUNC(void, CAN_44_TCAN4x5x_CODE_FAST)Can_44_TCAN4x5x_Irq_Handler(
                               VAR(uint32, AUTOMATIC) aId);
#define CAN_44_TCAN4x5x_STOP_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"

#define CAN_44_TCAN4x5x_START_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"
extern FUNC(uint16, CAN_44_TCAN4x5x_CODE_FAST) Can_44_TCAN4x5x_GetCurrentTimeStampCounterValue
                                 (VAR(uint8, AUTOMATIC) aId);
#define CAN_44_TCAN4x5x_STOP_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"

#define CAN_44_TCAN4x5x_START_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"
extern FUNC(uint16, CAN_44_TCAN4x5x_CODE_FAST) Can_44_TCAN4x5x_GetCurrentTxTimeStamp
                                 (VAR(uint8, AUTOMATIC) aId);
#define CAN_44_TCAN4x5x_STOP_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"

/*******************************************************************************
 *                      EXPORTED VARIABLES
 ******************************************************************************/
/*******************************************************************************
 *                      VERSION CHECK
 ******************************************************************************/
/** SW version check of *.h/c of the own module */
#if ((CAN_44_TCAN4x5x_SW_MAJOR_VERSION != \
                                    CAN_44_TCAN4x5x_SW_MAJOR_VERSION_CFG_H) || \
     (CAN_44_TCAN4x5x_SW_MINOR_VERSION !=  \
                                    CAN_44_TCAN4x5x_SW_MINOR_VERSION_CFG_H) || \
     (CAN_44_TCAN4x5x_SW_PATCH_VERSION != \
                                    CAN_44_TCAN4x5x_SW_PATCH_VERSION_CFG_H))
    #error "The SW version number of Can_44_TCAN4x5x.h and \
            Can_44_TCAN4x5x_Cfg.h are different."
#endif

#ifdef __cplusplus
}
#endif

#endif
/******************************************************************************
    Version     Date            Author           Description
    -------     ----            ------           -----------
    1.0.0     11-05-2017       Lakshmi H      Initial Version
	1.0.1     07-08-2017       Lakshmi H      Performed QAC Analysis and
											  justification is given for Misra
											  Rule voilations
	1.0.2     26-09-2017       Lakshmi H      Commenting style is changed to
											  Doxygen style
    1.0.3    17-11-2017       Lakshmi H       TCAN1043 code added and Software
                                               version changed
******************************************************************************/
/*******************************************************************************
 *  End of File: Can_44_TCAN4x5x.h
 ******************************************************************************/
