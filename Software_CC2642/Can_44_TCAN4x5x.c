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
 *  \file     Can_44_TCAN4x5x.c
 *
 *  \brief    This file contains CAN MCAL driver implementation.
 *
 */
/*******************************************************************************
 *                      MISRA-C VIOLATIONS
 ******************************************************************************/
/**
 * \brief
 * JUSTIFICATION_1: Dir-1.1(Required) To have a better throughput keyword inline
 *                  has been used.
 *                : Dir-1.1(Required) Number of Macro definitions are exceeding
 *                  1024.
*/

#ifdef __cplusplus
extern "C"{
#endif
/*******************************************************************************
 *                      INCLUDE FILES
 ******************************************************************************/

/** \brief CAN module header file, this includes CAN Module configuration header
 file*/
#include "Can_44_TCAN4x5x.h"

#if (CAN_44_TCAN4x5x_DEV_ERROR_DETECT == STD_ON)
#include "Det.h"
#endif
/** \brief Include the declarations of CAN callback functions and types */
#include "CanIf_Cbk.h"
#include "Can_44_TCAN4x5x_MCan.h"
#include "SchM_Can_44_TCAN4x5x.h"
#include "EcuM.h"

/*******************************************************************************
 *                      GLOBAL VARIABLES
 ******************************************************************************/

#if (CAN_44_TCAN4x5x_DEV_ERROR_DETECT == STD_ON)
#define CAN_44_TCAN4x5x_START_SEC_VAR_CLEARED_8
#include "Can_44_TCAN4x5x_MemMap.h"

/** Variable to store the ApiID to report Det error */
VAR(uint8, CAN_44_TCAN4x5x_VAR_CLEARED) Can_44_TCAN4x5x_ApiId;

#define CAN_44_TCAN4x5x_STOP_SEC_VAR_CLEARED_8
#include "Can_44_TCAN4x5x_MemMap.h"
#endif

#define CAN_44_TCAN4x5x_START_SEC_VAR_CLEARED_8
#include "Can_44_TCAN4x5x_MemMap.h"
/** Variable to store Can_44_TCAN4x5x_ControllerIf_State */
static VAR(uint8, CAN_44_TCAN4x5x_VAR_CLEARED)
    Can_44_TCAN4x5x_ControllerIf_State[CAN_44_TCAN4x5x_NUM_CONTROLLER_MAX];

#define CAN_44_TCAN4x5x_STOP_SEC_VAR_CLEARED_8
#include "Can_44_TCAN4x5x_MemMap.h"

#define CAN_44_TCAN4x5x_START_SEC_VAR_CLEARED_8
#include "Can_44_TCAN4x5x_MemMap.h"

volatile VAR(uint8, CAN_44_TCAN4x5x_VAR_CLEARED) Can_44_TCAN4x5x_WasCanTrcvInSleep;

#define CAN_44_TCAN4x5x_STOP_SEC_VAR_CLEARED_8
#include "Can_44_TCAN4x5x_MemMap.h"

/*******************************************************************************
 *                      LOCAL STRUCTURES, ENUMS, TYPEDEFS
 ******************************************************************************/

/*******************************************************************************
 *                      LOCAL CONSTANTS and MACROS
 ******************************************************************************/

#define CAN_44_TCAN4x5x_AR_RELEASE_MAJOR_VERSION_C     4u
#define CAN_44_TCAN4x5x_AR_RELEASE_MINOR_VERSION_C     2u
#define CAN_44_TCAN4x5x_AR_RELEASE_REVISION_VERSION_C  1u

#define CAN_44_TCAN4x5x_SW_MAJOR_VERSION_C     2u
#define CAN_44_TCAN4x5x_SW_MINOR_VERSION_C     3u
#define CAN_44_TCAN4x5x_SW_PATCH_VERSION_C     0u

/** Maximum size of the CAN Data */
#define CAN_44_TCAN4x5x_MAX_DATA_LENGTH             (8U)
/** Maximum size of the CAN FD  */
#define CAN_44_TCAN4x5x_MAX_FD_LENGTH               (64U)

/** Macro to set MSB for Extended messages */
#define CAN_44_TCAN4x5x_EXTENDED_MSB_SET            (0x80000000U)

/** Values */
#define CAN_44_TCAN4x5x_ZERO                        (0U)
#define CAN_44_TCAN4x5x_TWO                         (2U)
#define CAN_44_TCAN4x5x_THREE                       (3U)
#define CAN_44_TCAN4x5x_FOUR                        (4U)
#define CAN_44_TCAN4x5x_SIX                         (6U)
#define CAN_44_TCAN4x5x_SEVEN                       (7U)
#define CAN_44_TCAN4x5x_EIGHT                       (8U)
#define CAN_44_TCAN4x5x_NINE                        (9U)
#define CAN_44_TCAN4x5x_ELEVEN                      (11U)
#define CAN_44_TCAN4x5x_FIFTEEN                     (15U)
#define CAN_44_TCAN4x5x_TWENTY_FOUR                 (24U)
#define CAN_44_TCAN4x5x_THIRTY_SIX                  (36U)

/** API Service used without initialization */
#define CAN_44_TCAN4x5x_E_UNINIT           ((uint8)5)
/** Invalid CAN controller Id. Used for mapping non-activated CAN
        Controllers */
#define CAN_44_TCAN4x5x_INVALID_HW_CONTROLLER_ID    ((uint8)255)

#define CAN_44_TCAN4x5x_MODE_TRANSITION_IN_PROGRESS  (0xFFu)

/*******************************************************************************
|                       LOCAL FUNCTION PROTOTYPES                              |
*******************************************************************************/
#if (CAN_44_TCAN4x5x_DEV_ERROR_DETECT == STD_ON)
#define CAN_44_TCAN4x5x_START_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"

/** \brief Function for checking wrong parameter (Controller) to an API.
  Used in APIs affecting CAN controller */
/** MISRA DIR 1.1 VIOLATION: JUSTIFICATION_1 */
LOCAL_INLINE FUNC(Std_ReturnType, CAN_44_TCAN4x5x_CODE_FAST)
    Can_44_TCAN4x5x_ReportDetParamController
        (VAR(uint8, AUTOMATIC) ControllerId, VAR(uint8, AUTOMATIC) ServiceId);
#define CAN_44_TCAN4x5x_STOP_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"

#define CAN_44_TCAN4x5x_START_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"
/** MISRA DIR 1.1 VIOLATION: JUSTIFICATION_1 */
/** \brief Function to check for CAN driver initialization status for DET
             checking */
LOCAL_INLINE FUNC(Std_ReturnType, CAN_44_TCAN4x5x_CODE_FAST)
             Can_44_TCAN4x5x_ReportDetUninit(VAR(uint8, AUTOMATIC) ServiceId);
#define CAN_44_TCAN4x5x_STOP_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"

#define CAN_44_TCAN4x5x_START_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"
/** MISRA DIR 1.1 VIOLATION: JUSTIFICATION_1 */
/** \brief Function to check for invalid CAN controller state transitions */
LOCAL_INLINE FUNC(Std_ReturnType, CAN_44_TCAN4x5x_CODE_FAST)
             Can_44_TCAN4x5x_ModeDetCheck(VAR(uint8, AUTOMATIC) Controller,
               VAR(Can_StateTransitionType, AUTOMATIC) Transition);
#define CAN_44_TCAN4x5x_STOP_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"

#define CAN_44_TCAN4x5x_START_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"
/** MISRA DIR 1.1 VIOLATION: JUSTIFICATION_1 */
/** \brief Function to check DET for Can_44_TCAN4x5x_Write API  */
LOCAL_INLINE FUNC(Std_ReturnType, CAN_44_TCAN4x5x_CODE_FAST)
Can_44_TCAN4x5x_WriteDetCheck(VAR(Can_HwHandleType, AUTOMATIC) Hth,
P2CONST(Can_PduType, AUTOMATIC, CAN_44_TCAN4x5x_APPL_DATA) PduInfo);

#define CAN_44_TCAN4x5x_STOP_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"
#endif // (CAN_44_TCAN4x5x_DEV_ERROR_DETECT == STD_ON)
/*******************************************************************************
 *                      LOCAL VARIABLES
 ******************************************************************************/
#define CAN_44_TCAN4x5x_START_SEC_VAR_SLOW_CLEARED_8
#include "Can_44_TCAN4x5x_MemMap.h"

/** This flag stores the CAN driver state */
VAR(Can_44_TCAN4x5x_DriverStateType, CAN_44_TCAN4x5x_VAR_SLOW_CLEARED)
                                                   Can_44_TCAN4x5x_DriverState;

#define CAN_44_TCAN4x5x_STOP_SEC_VAR_SLOW_CLEARED_8
#include "Can_44_TCAN4x5x_MemMap.h"

#define CAN_44_TCAN4x5x_START_SEC_VAR_CLEARED_UNSPECIFIED
#include "Can_44_TCAN4x5x_MemMap.h"

/** State transitions that are used by the function CanIf_SetControllerMode*/
VAR(Can_StateTransitionType, CAN_44_TCAN4x5x_VAR_FAST_CLEARED)
             Can_44_TCAN4x5x_ControllerMode[CAN_44_TCAN4x5x_NUM_CONTROLLER_MAX];

#define CAN_44_TCAN4x5x_STOP_SEC_VAR_CLEARED_UNSPECIFIED
#include "Can_44_TCAN4x5x_MemMap.h"

#define CAN_44_TCAN4x5x_START_SEC_VAR_CLEARED_UNSPECIFIED
#include "Can_44_TCAN4x5x_MemMap.h"

P2CONST(Can_44_TCAN4x5x_ConfigType, CAN_44_TCAN4x5x_VAR_CLEARED,
                          CAN_44_TCAN4x5x_APPL_DATA)Can_44_TCAN4x5x_ConfigPtr;

#define CAN_44_TCAN4x5x_STOP_SEC_VAR_CLEARED_UNSPECIFIED
#include "Can_44_TCAN4x5x_MemMap.h"

#define CAN_44_TCAN4x5x_START_SEC_VAR_FAST_CLEARED_8
#include "Can_44_TCAN4x5x_MemMap.h"

/** This variable keeps track of number of calls to
   Can_44_TCAN4x5x_DisableControllerInterrupts API. It is used to implement
   incremental disable functionality. */
static VAR(uint8, CAN_44_TCAN4x5x_VAR_FAST_CLEARED)
     Can_44_TCAN4x5x_InterruptDisableCount[CAN_44_TCAN4x5x_NUM_CONTROLLER_MAX];
/** MISRA DIR 1.1 VIOLATION: JUSTIFICATION_1 */
#define CAN_44_TCAN4x5x_STOP_SEC_VAR_FAST_CLEARED_8
#include "Can_44_TCAN4x5x_MemMap.h"

#define CAN_44_TCAN4x5x_START_SEC_VAR_FAST_CLEARED_8
#include "Can_44_TCAN4x5x_MemMap.h"

static VAR(uint8, CAN_44_TCAN4x5x_VAR_FAST_CLEARED)
        Can_44_TCAN4x5x_HthMutex[32U * CAN_44_TCAN4x5x_NUM_CONTROLLER_MAX];

#define CAN_44_TCAN4x5x_STOP_SEC_VAR_FAST_CLEARED_8
#include "Can_44_TCAN4x5x_MemMap.h"

/*******************************************************************************
 *                      VERSION CHECK
 ******************************************************************************/
/** \brief [SWS_BSW_00036] Inter module version check. */
#ifndef DISABLE_TCAN4x5x_INTERMODULE_ASR_VERSION_CHECK
#if (CAN_44_TCAN4x5x_DEV_ERROR_DETECT == STD_ON)
    #if ((CAN_44_TCAN4x5x_AR_RELEASE_MAJOR_VERSION != \
                                              DET_AR_RELEASE_MAJOR_VERSION) || \
         (CAN_44_TCAN4x5x_AR_RELEASE_MINOR_VERSION != \
                                               DET_AR_RELEASE_MINOR_VERSION))
        #error "Autosar version numbers of Can_44_TCAN4x5x.h and Det.h are \
                different."
    #endif
#endif
#if ((CAN_44_TCAN4x5x_AR_RELEASE_MAJOR_VERSION != \
                                              RTE_AR_RELEASE_MAJOR_VERSION) || \
         (CAN_44_TCAN4x5x_AR_RELEASE_MINOR_VERSION != \
                                               RTE_AR_RELEASE_MINOR_VERSION))
        #error "Autosar version numbers of Can_44_TCAN4x5x.h and \
                SchM_Can_44_TCAN4x5x.h are different."
#endif

#if ((CAN_44_TCAN4x5x_AR_RELEASE_MAJOR_VERSION != \
                                              ECUM_AR_RELEASE_MAJOR_VERSION) || \
         (CAN_44_TCAN4x5x_AR_RELEASE_MINOR_VERSION != \
                                               ECUM_AR_RELEASE_MINOR_VERSION))
        #error "Autosar version numbers of Can_44_TCAN4x5x.h and \
                EcuM.h are different."
#endif

#if ((CAN_44_TCAN4x5x_AR_RELEASE_MAJOR_VERSION != \
                                              CANIF_AR_RELEASE_MAJOR_VERSION) || \
         (CAN_44_TCAN4x5x_AR_RELEASE_MINOR_VERSION != \
                                               CANIF_AR_RELEASE_MINOR_VERSION))
        #error "Autosar version numbers of Can_44_TCAN4x5x.h and \
                CanIf_Cbk.h are different."
#endif
#endif

/** \brief SW version check of *.h/c of the own module */
#if ((CAN_44_TCAN4x5x_SW_MAJOR_VERSION_C != \
                                         CAN_44_TCAN4x5x_SW_MAJOR_VERSION) || \
     (CAN_44_TCAN4x5x_SW_MINOR_VERSION_C != \
                                         CAN_44_TCAN4x5x_SW_MINOR_VERSION) || \
     (CAN_44_TCAN4x5x_SW_PATCH_VERSION_C != CAN_44_TCAN4x5x_SW_PATCH_VERSION))
    #error "The SW version number of Can_44_TCAN4x5x.c and Can_44_TCAN4x5x.h \
            are different."
#endif
/*******************************************************************************
 *                      FUNCTION DEFINITIONS
 ******************************************************************************/
/** \brief Memory map of the CAN driver code */
#define CAN_44_TCAN4x5x_START_SEC_CODE_SLOW
#include "Can_44_TCAN4x5x_MemMap.h"
/**
 * \verbatim
 *  Service Name...... : Can_44_TCAN4x5x_Init
 *  Service ID (hex).. : 0x00
 *  Sync/Async........ : Synchronous
 *  Reentrancy........ : Non Reentrant
 *  Parameters (IN)... : Config
 *  Parameters (INOUT) : None
 *  Parameters (OUT).. : None
 *  Return Value...... : None
 *  Description....... : This function initializes the module.
 *  Design IDs Covered : CAN_DesignId_001
 *
 * \endverbatim
**/
FUNC(void, CAN_44_TCAN4x5x_CODE_SLOW) Can_44_TCAN4x5x_Init
(P2CONST(Can_44_TCAN4x5x_ConfigType, AUTOMATIC,
                                     CAN_44_TCAN4x5x_APPL_DATA) Config)
{
    VAR(uint8, AUTOMATIC) CtrlIndex;
    #if (CAN_44_TCAN4x5x_DEV_ERROR_DETECT == STD_ON)
    VAR(Std_ReturnType, AUTOMATIC) Status = E_OK;
    #endif

    #if (CAN_44_TCAN4x5x_DEV_ERROR_DETECT == STD_ON)
    Can_44_TCAN4x5x_ApiId = CAN_44_TCAN4x5x_SID_INIT;
    #endif //CAN_44_TCAN4x5x_DEV_ERROR_DETECT == STD_ON

    #ifdef CAN_44_TCAN4x5x_VARIANT_PRE_COMPILE
    Can_44_TCAN4x5x_ConfigPtr = &Can_44_TCAN4x5x_Config;
    #endif//else of #ifdef CAN_44_TCAN4x5x_VARIANT_PRE_COMPILE

    #if (CAN_44_TCAN4x5x_DEV_ERROR_DETECT == STD_ON)

    #ifdef CAN_44_TCAN4x5x_VARIANT_POST_BUILD
    if (NULL_PTR == Config)
    {
        /** Report to DET */
        (void)Det_ReportError(CAN_44_TCAN4x5x_MODULE_ID,
                             CAN_44_TCAN4x5x_INSTANCE_ID, Can_44_TCAN4x5x_ApiId,
                                               CAN_44_TCAN4x5x_E_PARAM_POINTER);
        Status = E_NOT_OK;
    }

    if(Status == E_OK)
    #endif /** CAN_44_TCAN4x5x_VARIANT_POST_BUILD */
    {
        if((uint8)CAN_44_TCAN4x5x_UNINIT != (uint8)Can_44_TCAN4x5x_DriverState)
        {
            /** Report to DET */
            (void)Det_ReportError(CAN_44_TCAN4x5x_MODULE_ID,
                            CAN_44_TCAN4x5x_INSTANCE_ID, Can_44_TCAN4x5x_ApiId,
                                                 CAN_44_TCAN4x5x_E_TRANSITION);
            Status = E_NOT_OK;
        }
        else
        {
            /** CAN Controller is not in state UNINIT */
            for(CtrlIndex=CAN_44_TCAN4x5x_ZERO;
                       CtrlIndex<CAN_44_TCAN4x5x_NUM_CONTROLLER_MAX;CtrlIndex++)
            {
                /** SWS_Can_44_TCAN4x5x_00408 */
                if(CAN_44_TCAN4x5x_STATE_UNINIT !=
                              (uint32)Can_44_TCAN4x5x_ControllerMode[CtrlIndex])
                {
                    (void)Det_ReportError(CAN_44_TCAN4x5x_MODULE_ID,
                                          CAN_44_TCAN4x5x_INSTANCE_ID,
                                          Can_44_TCAN4x5x_ApiId,
                                          CAN_44_TCAN4x5x_E_TRANSITION);
                }
            }
        }
    }
    if (E_OK == Status)
    #endif
    {
        /**Store config pointer, such that it can be used throughout driver*/
        #ifdef CAN_44_TCAN4x5x_VARIANT_POST_BUILD
        Can_44_TCAN4x5x_ConfigPtr = Config;
        #endif
        for(CtrlIndex=CAN_44_TCAN4x5x_ZERO;
                       CtrlIndex<CAN_44_TCAN4x5x_NUM_CONTROLLER_MAX;CtrlIndex++)
        {
            Can_44_TCAN4x5x_MCAN_Init(CtrlIndex, Can_44_TCAN4x5x_ConfigPtr);

            if(Can_44_TCAN4x5x_MajorRevId != (uint8)0x01){
                Can_44_TCAN4x5x_ControllerMode[CtrlIndex] = CAN_T_STOP;
            }
            else{
            /* Do Nothing */
            }

        }
        if(Can_44_TCAN4x5x_MajorRevId != (uint8)0x01){
            /** Change the driver state to initialized */
            Can_44_TCAN4x5x_DriverState = (uint8)CAN_44_TCAN4x5x_READY;
        }
        else{
            Can_44_TCAN4x5x_DriverState =(uint8) CAN_44_TCAN4x5x_E_UNINIT;
        }
    }
}
#define CAN_44_TCAN4x5x_STOP_SEC_CODE_SLOW
#include "Can_44_TCAN4x5x_MemMap.h"

#if(CAN_44_TCAN4x5x_VERSION_INFO_API == STD_ON)
#define CAN_44_TCAN4x5x_START_SEC_CODE_SLOW
#include "Can_44_TCAN4x5x_MemMap.h"
/**
 * \verbatim
 *  Service Name...... : Can_44_TCAN4x5x_GetVersionInfo
 *  Service ID (hex).. : 0x07
 *  Sync/Async........ : Synchronous
 *  Reentrancy........ : Reentrant
 *  Parameters (IN)... : None
 *  Parameters (INOUT) : None
 *  Parameters (OUT).. : versioninfo
 *  Return Value...... : None
 *  Description....... : This function returns version information of this module.
 *  Design IDs Covered : CAN_DesignId_002
 *
 * \endverbatim
**/
FUNC(void, CAN_44_TCAN4x5x_CODE_SLOW) Can_44_TCAN4x5x_GetVersionInfo
(P2VAR(Std_VersionInfoType, AUTOMATIC, CAN_44_TCAN4x5x_APPL_DATA) versioninfo)
{
    #if (CAN_44_TCAN4x5x_DEV_ERROR_DETECT == STD_ON)
    Can_44_TCAN4x5x_ApiId = CAN_44_TCAN4x5x_SID_GETVERSIONINFO;
    #endif /** CAN_44_TCAN4x5x_DEV_ERROR_DETECT == STD_ON */

    #if (CAN_44_TCAN4x5x_DEV_ERROR_DETECT == STD_ON)
    /** If the parameter versionInfo is a null pointer */
    if (NULL_PTR == versioninfo)
    {
        /** Report to DET */
        (void)Det_ReportError(CAN_44_TCAN4x5x_MODULE_ID,
                              CAN_44_TCAN4x5x_INSTANCE_ID,
                              Can_44_TCAN4x5x_ApiId,
                              CAN_44_TCAN4x5x_E_PARAM_POINTER);
    }
    else
    #endif
    {
        /** module vendor Id */
        versioninfo->vendorID = CAN_44_TCAN4x5x_VENDOR_ID;
        /** can module Id */
        versioninfo->moduleID = CAN_44_TCAN4x5x_MODULE_ID;
        /** module major version */
        versioninfo->sw_major_version = CAN_44_TCAN4x5x_SW_MAJOR_VERSION;
        /** module minor version */
        versioninfo->sw_minor_version = CAN_44_TCAN4x5x_SW_MINOR_VERSION;
        /** module patch version */
        versioninfo->sw_patch_version = CAN_44_TCAN4x5x_SW_PATCH_VERSION;
    }
}

#define CAN_44_TCAN4x5x_STOP_SEC_CODE_SLOW
#include "Can_44_TCAN4x5x_MemMap.h"
#endif

#if(CAN_44_TCAN4x5x_SET_BAUDRATE_API == STD_ON)
#define CAN_44_TCAN4x5x_START_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"
/**
 * \verbatim
 *  Service Name...... : Can_44_TCAN4x5x_SetBaudrate
 *  Service ID (hex).. : 0x0f
 *  Sync/Async........ : Synchronous
 *  Reentrancy........ : Reentrant for different Controllers. Non reentrant for the
 *                      same Controller.
 *  Parameters (IN)... : Controller, BaudRateConfigID
 *  Parameters (INOUT) : None
 *  Parameters (OUT).. : None
 *  Return Value...... : E_OK, E_NOT_OK
 *  Description....... : This service shall set the baud rate configuration of the
 *                      CAN controller. Depending on necessary baud rate
 *                      modifications the controller might have to reset.
 *  Design IDs Covered : CAN_DesignId_003
 *
 * \endverbatim
**/
FUNC(Std_ReturnType, CAN_44_TCAN4x5x_CODE_FAST) Can_44_TCAN4x5x_SetBaudrate
(VAR(uint8, AUTOMATIC) Controller, VAR(uint16, AUTOMATIC) BaudRateConfigID)
{
    VAR(Std_ReturnType, AUTOMATIC) Status;

    #if (CAN_44_TCAN4x5x_DEV_ERROR_DETECT == STD_ON)
    Can_44_TCAN4x5x_ApiId = CAN_44_TCAN4x5x_SID_SET_BAUDRATE;
    #endif /** CAN_44_TCAN4x5x_DEV_ERROR_DETECT == STD_ON */

    #if (CAN_44_TCAN4x5x_DEV_ERROR_DETECT == STD_ON)
    /** Check if CAN driver is initialized */
    Status = Can_44_TCAN4x5x_ReportDetUninit(Can_44_TCAN4x5x_ApiId);

    if(E_OK == Status)
    {
        /** Check if Controller is valid */
        Status = Can_44_TCAN4x5x_ReportDetParamController(Controller,
                                                         Can_44_TCAN4x5x_ApiId);

        /** if Controller is valid, then check the state */
        if (E_OK == Status)
        {
            /** If the CAN controller is not in "stopped" state */
            if (Can_44_TCAN4x5x_ControllerMode[Controller] !=
                                                         CAN_T_STOP)
            {
                /** Report invalid state to DET */
                (void)Det_ReportError(CAN_44_TCAN4x5x_MODULE_ID,
                                      CAN_44_TCAN4x5x_INSTANCE_ID,
                                      Can_44_TCAN4x5x_ApiId,
                                      CAN_44_TCAN4x5x_E_TRANSITION);
                Status = E_NOT_OK ;
            }
        }
        else
        {
            Status = (uint8)CAN_NOT_OK ;
        }
    }

    if (E_OK == Status)
    #endif // (CAN_44_TCAN4x5x_DEV_ERROR_DETECT == STD_ON)
    {
        #if (CAN_44_TCAN4x5x_DEV_ERROR_DETECT == STD_ON)
        /** check if BaudRateConfigID is invalid/unsupported */

        if(BaudRateConfigID >= ((uint16)(Can_44_TCAN4x5x_ConfigPtr->
        CanControllerConfiguration[Controller].CanNumOfBaudRateConf)))
        {
            /** Report invalid baudrate to DET */
            (void)Det_ReportError(CAN_44_TCAN4x5x_MODULE_ID,
                                  CAN_44_TCAN4x5x_INSTANCE_ID,
                                  Can_44_TCAN4x5x_ApiId,
                                  CAN_44_TCAN4x5x_E_PARAM_BAUDRATE);
            Status = E_NOT_OK;
        }
        else
        #endif // (CAN_44_TCAN4x5x_DEV_ERROR_DETECT == STD_ON)
        {
            Status = E_OK;
            Can_44_TCAN4x5x_MCAN_SetCCE(Controller);
            (void)Can_44_TCAN4x5x_MCAN_SetBaudrate(Controller,
            &Can_44_TCAN4x5x_ConfigPtr->
            CanControllerConfiguration[Controller].
            CanControllerBaudrateConfigPtr[BaudRateConfigID]);
        }
    }
    return (Status);
}
#define CAN_44_TCAN4x5x_STOP_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"
#endif

#define CAN_44_TCAN4x5x_START_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"
/**
 * \verbatim
 *  Service Name...... : Can_44_TCAN4x5x_SetControllerMode
 *  Service ID (hex).. : 0x03
 *  Sync/Async........ : Asynchronous
 *  Reentrancy........ : Non Reentrant
 *  Parameters (IN)... : Controller, Transition
 *  Parameters (INOUT) : None
 *  Parameters (OUT).. : None
 *  Return Value...... : CAN_OK, CAN_NOT_OK
 *  Description....... : This function performs software triggered state
 *                       transitions of the CAN controller State machine.
 *  Design IDs Covered : CAN_DesignId_004
 *
 * \endverbatim
**/
FUNC(Can_ReturnType, CAN_44_TCAN4x5x_CODE_FAST)
        Can_44_TCAN4x5x_SetControllerMode(VAR(uint8, AUTOMATIC) Controller,
                VAR(Can_StateTransitionType, AUTOMATIC) Transition)
{
    VAR(Can_ReturnType, AUTOMATIC) RetValue = CAN_BUSY; //prevent any further action
    #if (CAN_44_TCAN4x5x_DEV_ERROR_DETECT == STD_ON)
    VAR(Std_ReturnType, AUTOMATIC) Status;
    Can_44_TCAN4x5x_ApiId = CAN_44_TCAN4x5x_SID_SETCONTROLLERMODE;
    #endif

    #if (CAN_44_TCAN4x5x_DEV_ERROR_DETECT == STD_ON)
    /** Check if CAN driver is initialized */
    Status = Can_44_TCAN4x5x_ReportDetUninit
                                        (CAN_44_TCAN4x5x_SID_SETCONTROLLERMODE);

    if(E_OK == Status)
    {
        /** Check if Controller is valid */
        Status = Can_44_TCAN4x5x_ReportDetParamController(Controller,
                                         CAN_44_TCAN4x5x_SID_SETCONTROLLERMODE);

        if(E_OK == Status)
        {
            /** Check if requested state transition is valid */
            Status = Can_44_TCAN4x5x_ModeDetCheck(Controller, Transition);
            if (E_NOT_OK == Status)
            {
                /** Report invalid transition request to DET */
                (void)Det_ReportError(CAN_44_TCAN4x5x_MODULE_ID,
                                      CAN_44_TCAN4x5x_INSTANCE_ID,
                                      Can_44_TCAN4x5x_ApiId,
                                      CAN_44_TCAN4x5x_E_TRANSITION);
                RetValue = CAN_NOT_OK;
            }
        }
    }
    if (E_NOT_OK == Status)
    {
        RetValue = CAN_NOT_OK;
    }
    else
    #endif // (CAN_44_TCAN4x5x_DEV_ERROR_DETECT == STD_ON)
    {
        if(Transition == CAN_T_STOP)
        {
            if( Can_44_TCAN4x5x_ControllerMode[Controller] != CAN_T_STOP )
            {
                RetValue = (Can_ReturnType)Can_44_TCAN4x5x_MCAN_SetMode
                                           (Controller, CAN_44_TCAN4x5x_STATE_IDLE);
                if (RetValue == (Can_ReturnType)CAN_BUSY)
                {
                    Can_44_TCAN4x5x_ControllerIf_State[Controller] =
                                 (uint8)CAN_44_TCAN4x5x_MODE_TRANSITION_IN_PROGRESS;
                }
                else
                {
                    Can_44_TCAN4x5x_ControllerIf_State[Controller]
                                                = (uint8)CANIF_CS_STOPPED;
                }
            }
        }
        else if(Transition == CAN_T_START)
        {
            if( Can_44_TCAN4x5x_ControllerMode[Controller] != CAN_T_START )
            {
                if(Can_44_TCAN4x5x_WasCanTrcvInSleep == (uint8)1)
                {
                    Can_44_TCAN4x5x_MCAN_Init(Controller,
                                                    Can_44_TCAN4x5x_ConfigPtr);
                    Can_44_TCAN4x5x_WasCanTrcvInSleep = (uint8)0;
                }
                RetValue = (Can_ReturnType)Can_44_TCAN4x5x_MCAN_SetMode
                                        (Controller, CAN_44_TCAN4x5x_STATE_RUNNING);
                Can_44_TCAN4x5x_ControllerIf_State[Controller]
                                                = (uint8)CANIF_CS_STARTED;
            }
        }
        else if(Transition == CAN_T_WAKEUP)
        {
            if( Can_44_TCAN4x5x_ControllerMode[Controller] != CAN_T_STOP )
            {
                RetValue = (Can_ReturnType)Can_44_TCAN4x5x_MCAN_SetMode
                                        (Controller, CAN_44_TCAN4x5x_STATE_WAKEUP);
                Transition = CAN_T_STOP;
                Can_44_TCAN4x5x_ControllerIf_State[Controller]
                                                        = (uint8)CANIF_CS_STOPPED;
            }
        }
        else
        {
            if( Can_44_TCAN4x5x_ControllerMode[Controller] != CAN_T_SLEEP )
            {
                Transition = CAN_T_SLEEP;
                RetValue = (Can_ReturnType)Can_44_TCAN4x5x_MCAN_SetMode
                                          (Controller, CAN_44_TCAN4x5x_STATE_SLEEP);
                Can_44_TCAN4x5x_ControllerIf_State[Controller]
                                                    =(uint8)CANIF_CS_SLEEP;
            }
        }

        if (RetValue == CAN_OK)
        {
            Can_44_TCAN4x5x_ControllerMode[Controller] = Transition;
            CanIf_ControllerModeIndication(Controller,
                  (CanIf_ControllerModeType)
                            Can_44_TCAN4x5x_ControllerIf_State[Controller]);
        }
        else if (RetValue == (Can_ReturnType)CAN_BUSY)
        {
            RetValue = CAN_OK;
        }
        else /** Mode transition failed */
        {
            RetValue = CAN_NOT_OK;
        }
    }
    return(RetValue);
}
#define CAN_44_TCAN4x5x_STOP_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"

#define CAN_44_TCAN4x5x_START_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"
/**
 * \verbatim
 *  Service Name...... : Can_44_TCAN4x5x_DisableControllerInterrupts
 *  Service ID (hex).. : 0x04
 *  Sync/Async........ : Synchronous
 *  Reentrancy........ : Reentrant
 *  Parameters (IN)... : Controller
 *  Parameters (INOUT) : None
 *  Parameters (OUT).. : None
 *  Return Value...... : None
 *  Description....... : This function disables all interrupts for this
                         controller.
 *  Design IDs Covered : CAN_DesignId_005
 *
 * \endverbatim
**/
FUNC(void, CAN_44_TCAN4x5x_CODE_FAST)
Can_44_TCAN4x5x_DisableControllerInterrupts(VAR(uint8, AUTOMATIC) Controller)
{
    #if (CAN_44_TCAN4x5x_DEV_ERROR_DETECT == STD_ON)
    VAR(Std_ReturnType, AUTOMATIC) Status;
    Can_44_TCAN4x5x_ApiId = CAN_44_TCAN4x5x_SID_DISABLECONTROLLERINTERRUPTS;
    #endif /** CAN_44_TCAN4x5x_DEV_ERROR_DETECT == STD_ON */

    #if (CAN_44_TCAN4x5x_DEV_ERROR_DETECT == STD_ON)
    /** Check if CAN driver is initialized */
    Status = Can_44_TCAN4x5x_ReportDetUninit
                             (CAN_44_TCAN4x5x_SID_DISABLECONTROLLERINTERRUPTS);

    if(E_OK == Status)
    {
        /** Check if Controller is valid */
        Status = Can_44_TCAN4x5x_ReportDetParamController(Controller,
                               CAN_44_TCAN4x5x_SID_DISABLECONTROLLERINTERRUPTS);
    }

    if (E_OK == Status)
    #endif // (CAN_44_TCAN4x5x_DEV_ERROR_DETECT == STD_ON)
    {
        /** Enter Critical Section */
        SchM_Enter_Can_44_TCAN4x5x_Exclusive_Area_00();

        if (Can_44_TCAN4x5x_InterruptDisableCount[Controller] ==
                                                           CAN_44_TCAN4x5x_ZERO)
        {
            /** Disable controller interrupts */
            Can_44_TCAN4x5x_MCAN_DisableAllInterrupts(Controller);
        }
        /**Count the number of times Can_44_TCAN4x5x_DisableControllerInterrupts
         API is called */
        Can_44_TCAN4x5x_InterruptDisableCount[Controller]++;

        /** Exit Critical Section */
        SchM_Exit_Can_44_TCAN4x5x_Exclusive_Area_00();
    }
}
#define CAN_44_TCAN4x5x_STOP_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"

#define CAN_44_TCAN4x5x_START_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"
/**
 * \verbatim
 *  Service Name...... : Can_44_TCAN4x5x_EnableControllerInterrupts
 *  Service ID (hex).. : 0x05
 *  Sync/Async........ : Synchronous
 *  Reentrancy........ : Reentrant
 *  Parameters (IN)... : Controller
 *  Parameters (INOUT) : None
 *  Parameters (OUT).. : None
 *  Return Value...... : None
 *  Description....... : This function enables all allowed interrupts.
 *  Design IDs Covered : CAN_DesignId_006
 *
 * \endverbatim
**/
FUNC(void, CAN_44_TCAN4x5x_CODE_FAST)
Can_44_TCAN4x5x_EnableControllerInterrupts(VAR(uint8, AUTOMATIC) Controller)
{
    #if (CAN_44_TCAN4x5x_DEV_ERROR_DETECT == STD_ON)
    VAR(Std_ReturnType, AUTOMATIC) Status;
    Can_44_TCAN4x5x_ApiId = CAN_44_TCAN4x5x_SID_ENABLECONTROLLERINTERRUPTS;
    #endif /** CAN_44_TCAN4x5x_DEV_ERROR_DETECT == STD_ON */

    #if (CAN_44_TCAN4x5x_DEV_ERROR_DETECT == STD_ON)

    /** Check if CAN driver is initialized */
    Status = Can_44_TCAN4x5x_ReportDetUninit
                            (CAN_44_TCAN4x5x_SID_ENABLECONTROLLERINTERRUPTS);

    if(E_OK == Status)
    {
        /** Check if Controller is valid */
        Status = Can_44_TCAN4x5x_ReportDetParamController(Controller,
                                CAN_44_TCAN4x5x_SID_ENABLECONTROLLERINTERRUPTS);
    }

    if (E_OK == Status)
    #endif // (CAN_44_TCAN4x5x_DEV_ERROR_DETECT == STD_ON)
    {
        /** Enter Critical Section */
        SchM_Enter_Can_44_TCAN4x5x_Exclusive_Area_01();

        /** No action shall be performed when
        Can_44_TCAN4x5x_DisableControllerInterrupts has not been called before*/
        if (Can_44_TCAN4x5x_InterruptDisableCount[Controller] >
                                                           CAN_44_TCAN4x5x_ZERO)
        {
            /** Incremental disable/enable count */
            Can_44_TCAN4x5x_InterruptDisableCount[Controller]-- ;

            if (Can_44_TCAN4x5x_InterruptDisableCount[Controller] ==
                                                           CAN_44_TCAN4x5x_ZERO)
            {
                /** Enable controller interrupts */
                Can_44_TCAN4x5x_MCAN_EnableAllInterrupts(Controller);
            }
        }
        /** Exit Critical Section */
        SchM_Exit_Can_44_TCAN4x5x_Exclusive_Area_01();
    }
}
#define CAN_44_TCAN4x5x_STOP_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"

#if(CAN_44_TCAN4x5x_WAKEUP_FUNCTIONALITY_API == STD_ON)
#define CAN_44_TCAN4x5x_START_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"
/**
 * \verbatim
 *  Service Name...... : Can_44_TCAN4x5x_CheckWakeup
 *  Service ID (hex).. : 0x0b
 *  Sync/Async........ : Synchronous
 *  Reentrancy........ : Non Reentrant
 *  Parameters (IN)... : Controller
 *  Parameters (INOUT) : None
 *  Parameters (OUT).. : None
 *  Return Value...... : CAN_OK, CAN_NOT_OK
 *  Description....... : This function checks if a wakeup has occurred for
 *                      the given controller.
 *  Design IDs Covered : CAN_DesignId_007
 *
 * \endverbatim
**/
FUNC(Can_ReturnType, CAN_44_TCAN4x5x_CODE_FAST)
                Can_44_TCAN4x5x_CheckWakeup(VAR(uint8, AUTOMATIC) Controller)
{
    #if (CAN_44_TCAN4x5x_DEV_ERROR_DETECT == STD_ON)
    VAR(Std_ReturnType, AUTOMATIC) Status;
    #endif

    #if (CAN_44_TCAN4x5x_DEV_ERROR_DETECT == STD_ON)
    Can_44_TCAN4x5x_ApiId = CAN_44_TCAN4x5x_SID_CHECKWAKEUP;
    #endif /** CAN_44_TCAN4x5x_DEV_ERROR_DETECT == STD_ON */

    VAR(Can_ReturnType, AUTOMATIC) RetValue;
    RetValue = CAN_NOT_OK;

    #if (CAN_44_TCAN4x5x_DEV_ERROR_DETECT == STD_ON)
    /** Check if CAN driver is initialized */
    Status = Can_44_TCAN4x5x_ReportDetUninit(CAN_44_TCAN4x5x_SID_CHECKWAKEUP);

    if(E_OK == Status)
    {
        /** Check if Controller is valid */
        Status = Can_44_TCAN4x5x_ReportDetParamController(Controller,
                                               CAN_44_TCAN4x5x_SID_CHECKWAKEUP);
    }

    if(E_OK == Status)
    #endif // (CAN_44_TCAN4x5x_DEV_ERROR_DETECT == STD_ON)
    {
        #if (CAN_44_TCAN4x5x_PUBLIC_ICOM_SUPPORT == STD_ON)
        /** Check if wakeup was detected */
        if(CAN_44_TCAN4x5x_ONE == Can_44_TCAN4x5x_IcomWakeup[Controller])
        {
            /** Clear wakeup status and return CAN_OK */
            Can_44_TCAN4x5x_IcomWakeup[Controller] = CAN_44_TCAN4x5x_ZERO;
            RetValue = CAN_OK;

            /** Call EcuM module to set the Wakeup */
            EcuM_SetWakeupEvent((EcuM_WakeupSourceType)
                 Can_44_TCAN4x5x_ControllerSetting_PC[Controller].CanWakeupRef);
        }
        #endif
    }
    return(RetValue);
}
#define CAN_44_TCAN4x5x_STOP_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"
#endif

#define CAN_44_TCAN4x5x_START_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"
/**
 * \verbatim
 *  Service Name...... : Can_44_TCAN4x5x_Write
 *  Service ID (hex).. : 0x06
 *  Sync/Async........ : Synchronous
 *  Reentrancy........ : Reentrant (thread-safe)
 *  Parameters (IN)... : Hth, PduInfo
 *  Parameters (INOUT) : None
 *  Parameters (OUT).. : None
 *  Return Value...... : CAN_OK, CAN_NOT_OK,
 *                      CAN_BUSY
 *  Description....... : This function is called by CanIf to pass a CAN message
 *                       to CanDrv for transmission.
 *  Design IDs Covered : CAN_DesignId_008
 *
 * \endverbatim
**/
FUNC(Can_ReturnType, CAN_44_TCAN4x5x_CODE_FAST)
Can_44_TCAN4x5x_Write(VAR(Can_HwHandleType, AUTOMATIC) Hth,
P2CONST(Can_PduType, AUTOMATIC, CAN_44_TCAN4x5x_APPL_DATA) PduInfo)
{
    VAR(uint8, AUTOMATIC) ctrlId;
    VAR(uint8, AUTOMATIC) index;
    VAR(Can_ReturnType, AUTOMATIC) RetValue;
    VAR(uint32, AUTOMATIC) ret;
    #if (CAN_44_TCAN4x5x_DEV_ERROR_DETECT == STD_ON)
    VAR(Std_ReturnType, AUTOMATIC) Status;
    #endif

    #if (CAN_44_TCAN4x5x_DEV_ERROR_DETECT == STD_ON)
    Can_44_TCAN4x5x_ApiId = CAN_44_TCAN4x5x_SID_WRITE;
    #endif /** CAN_44_TCAN4x5x_DEV_ERROR_DETECT == STD_ON */


    #if (CAN_44_TCAN4x5x_DEV_ERROR_DETECT == STD_ON)
    /** Check if CAN driver is initialized */
    Status = Can_44_TCAN4x5x_ReportDetUninit(CAN_44_TCAN4x5x_SID_WRITE);

    if (E_OK == Status)
    {
        /** Check for DET */
        Status = Can_44_TCAN4x5x_WriteDetCheck(Hth, PduInfo);
    }
    if (E_NOT_OK == Status)
    {
        RetValue = CAN_NOT_OK;
    }
    else
    #endif // (CAN_44_TCAN4x5x_DEV_ERROR_DETECT == STD_ON)
    {
        RetValue = CAN_OK;
        Hth = Hth - Can_44_TCAN4x5x_ConfigPtr->CanHRHobjectsCount;

        /** Enter Critical Section and check for mutex */
        SchM_Enter_Can_44_TCAN4x5x_Exclusive_Area_02();
        if (Can_44_TCAN4x5x_HthMutex[Hth] == (uint8)0)
        {
            /** Set mutex */
            Can_44_TCAN4x5x_HthMutex[Hth] = (uint8)1;
            SchM_Exit_Can_44_TCAN4x5x_Exclusive_Area_02();
            ctrlId =
            Can_44_TCAN4x5x_ConfigPtr->CanHTHMappingConfigPtr[Hth].CanCtrlId;
            index =
            Can_44_TCAN4x5x_ConfigPtr->CanHTHMappingConfigPtr[Hth].CanHthIndex;

            #if (CAN_44_TCAN4x5x_PUBLIC_ICOM_SUPPORT == STD_ON)
            if(Can_44_TCAN4x5x_GComConfigId[ctrlId] != 0U)
            {
                RetValue = CAN_BUSY;
            }
            else
            #endif
            {
                /** Write the message into TX buffer */
                ret = Can_44_TCAN4x5x_MCAN_Write(ctrlId, PduInfo,
                      &Can_44_TCAN4x5x_ConfigPtr->
                      CanControllerConfiguration[ctrlId].
                      CanTxObjConfigPtr[index]);
                if ((uint32)CAN_BUSY == ret)
                {
                    /** There is no free message buffer, return BUSY */
                    RetValue = CAN_BUSY;
                }
                else if (ret != (uint32)E_OK)
                {
                    /** Report error to DET */
                    #if (CAN_44_TCAN4x5x_DEV_ERROR_DETECT == STD_ON)
                    (void)Det_ReportError(CAN_44_TCAN4x5x_MODULE_ID,
                                        CAN_44_TCAN4x5x_INSTANCE_ID,
                                        CAN_44_TCAN4x5x_SID_WRITE,
                                        CAN_44_TCAN4x5x_E_PARAM_POINTER);
                    #endif
                    RetValue = CAN_NOT_OK;
                }
                else
                {
                    /** Do nothing */
                }
                SchM_Enter_Can_44_TCAN4x5x_Exclusive_Area_02();
                /** Clear mutex */
                Can_44_TCAN4x5x_HthMutex[Hth] = (uint8)0;
                SchM_Exit_Can_44_TCAN4x5x_Exclusive_Area_02();
            }
        }
        else
        {
            /** Return CAN_BUSY for preemptive call of this function */
            SchM_Exit_Can_44_TCAN4x5x_Exclusive_Area_02();
            RetValue = CAN_BUSY;
        }
    }
    return(RetValue);
}
#define CAN_44_TCAN4x5x_STOP_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"

#define CAN_44_TCAN4x5x_START_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"
/**
 * \verbatim
 *  Service Name...... : Can_44_TCAN4x5x_MainFunction_Write
 *  Service ID (hex).. : 0x01
 *  Sync/Async........ : NA
 *  Reentrancy........ : NA
 *  Parameters (IN)... : None
 *  Parameters (INOUT) : None
 *  Parameters (OUT).. : None
 *  Return Value...... : None
 *  Description....... : This function performs the polling of TX confirmation
 *                       when CAN_44_TCAN4x5x_TX_PROCESSING is set to POLLING.
 *  Design IDs Covered : CAN_DesignId_009
 *
 * \endverbatim
**/
FUNC(void, CAN_44_TCAN4x5x_CODE_FAST) Can_44_TCAN4x5x_MainFunction_Write(void)
{
    VAR(Std_ReturnType, AUTOMATIC) CtrlIndex;
    VAR(uint8, AUTOMATIC) exec_context_backup;

    #if (CAN_44_TCAN4x5x_DEV_ERROR_DETECT == STD_ON)
    VAR(Std_ReturnType, AUTOMATIC) Status;
    Can_44_TCAN4x5x_ApiId = CAN_44_TCAN4x5x_SID_MAINFUNCTION_WRITE;
    #endif /** CAN_44_TCAN4x5x_DEV_ERROR_DETECT == STD_ON */

    #if (CAN_44_TCAN4x5x_DEV_ERROR_DETECT == STD_ON)

    /** Check if CAN driver is initialized */
    Status = Can_44_TCAN4x5x_ReportDetUninit
                                    (CAN_44_TCAN4x5x_SID_MAINFUNCTION_WRITE);
    if(E_OK == Status)
    #else
    if(CAN_44_TCAN4x5x_E_UNINIT != Can_44_TCAN4x5x_DriverState)
    #endif // (CAN_44_TCAN4x5x_DEV_ERROR_DETECT == STD_ON)
    {
        /** Call transmit handler function for each of the CAN controllers
        configured as POLLING mode for transmission successful event handling */
        for(CtrlIndex=CAN_44_TCAN4x5x_ZERO;
            CtrlIndex< (uint8)CAN_44_TCAN4x5x_NUM_CONTROLLER_MAX; CtrlIndex++)
        {
            if(Can_44_TCAN4x5x_ControllerSetting_PC[CtrlIndex].CanTxProcessing
                                            == CAN_44_TCAN4x5x_POLLING)
            {
                exec_context_backup = Can_44_TCAN4x5x_ExCntxt[CtrlIndex];
                Can_44_TCAN4x5x_ExCntxt[CtrlIndex] = MAIN_FUNCTION_CONTEXT;
                Can_44_TCAN4x5x_MCAN_ProcessCanTx(CtrlIndex);
                Can_44_TCAN4x5x_ExCntxt[CtrlIndex] = exec_context_backup;
            }
        }
    }
}
#define CAN_44_TCAN4x5x_STOP_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"

#define CAN_44_TCAN4x5x_START_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"
/**
 * \verbatim
 *  Service Name...... : Can_44_TCAN4x5x_MainFunction_Read
 *  Service ID (hex).. : 0x08
 *  Sync/Async........ : NA
 *  Reentrancy........ : NA
 *  Parameters (IN)... : None
 *  Parameters (INOUT) : None
 *  Parameters (OUT).. : None
 *  Return Value...... : None
 *  Description....... : This function performs the polling of RX indications
 *                       when CAN_44_TCAN4x5x_RX_PROCESSING is set to POLLING.
 *  Design IDs Covered : CAN_DesignId_010
 *
 * \endverbatim
**/
FUNC(void, CAN_44_TCAN4x5x_CODE_FAST) Can_44_TCAN4x5x_MainFunction_Read(void)
{
    VAR(Std_ReturnType, AUTOMATIC) CtrlIndex;
    VAR(uint8, AUTOMATIC) exec_context_backup;

    #if (CAN_44_TCAN4x5x_DEV_ERROR_DETECT == STD_ON)
    VAR(Std_ReturnType, AUTOMATIC) Status;
    Can_44_TCAN4x5x_ApiId = CAN_44_TCAN4x5x_SID_MAINFUNCTION_READ;
    #endif /** CAN_44_TCAN4x5x_DEV_ERROR_DETECT == STD_ON */

    #if (CAN_44_TCAN4x5x_DEV_ERROR_DETECT == STD_ON)

    /** Check if CAN driver is initialized */
    Status = Can_44_TCAN4x5x_ReportDetUninit
                                    (CAN_44_TCAN4x5x_SID_MAINFUNCTION_READ);
    if(E_OK == Status)
    #else
    if(CAN_44_TCAN4x5x_E_UNINIT != Can_44_TCAN4x5x_DriverState)
    #endif // (CAN_44_TCAN4x5x_DEV_ERROR_DETECT == STD_ON)
    {
        /** Call receive handler function for each of the CAN controllers
        configured as POLLING mode for reception successful event handling */
        for(CtrlIndex=CAN_44_TCAN4x5x_ZERO;
        CtrlIndex< (uint8)CAN_44_TCAN4x5x_NUM_CONTROLLER_MAX; CtrlIndex++)
        {
            if(Can_44_TCAN4x5x_ControllerSetting_PC[CtrlIndex].CanRxProcessing
                                            == CAN_44_TCAN4x5x_POLLING)
            {
                exec_context_backup = Can_44_TCAN4x5x_ExCntxt[CtrlIndex];
                Can_44_TCAN4x5x_ExCntxt[CtrlIndex] = MAIN_FUNCTION_CONTEXT;
                Can_44_TCAN4x5x_MCAN_ProcessCanRx(CtrlIndex);
                Can_44_TCAN4x5x_ExCntxt[CtrlIndex] = exec_context_backup;
            }
        }
    }
}
#define CAN_44_TCAN4x5x_STOP_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"

#define CAN_44_TCAN4x5x_START_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"
/**
 * \verbatim
 *  Service Name...... : Can_44_TCAN4x5x_MainFunction_BusOff
 *  Service ID (hex).. : 0x09
 *  Sync/Async........ : NA
 *  Reentrancy........ : NA
 *  Parameters (IN)... : None
 *  Parameters (INOUT) : None
 *  Parameters (OUT).. : None
 *  Return Value...... : None
 *  Description....... : This function performs the polling of bus-off events
 *                       that are configured statically as 'to be polled'.
 *  Design IDs Covered : CAN_DesignId_011
 *
 * \endverbatim
**/
FUNC(void, CAN_44_TCAN4x5x_CODE_FAST) Can_44_TCAN4x5x_MainFunction_BusOff(void)
{
    VAR(Std_ReturnType, AUTOMATIC) CtrlIndex;
    VAR(uint8, AUTOMATIC) exec_context_backup;

    #if (CAN_44_TCAN4x5x_DEV_ERROR_DETECT == STD_ON)
    VAR(Std_ReturnType, AUTOMATIC) Status;
    Can_44_TCAN4x5x_ApiId = CAN_44_TCAN4x5x_SID_MAINFUNCTION_BUSOFF;
    #endif /** CAN_44_TCAN4x5x_DEV_ERROR_DETECT == STD_ON */

    #if (CAN_44_TCAN4x5x_DEV_ERROR_DETECT == STD_ON)

    /** Check if CAN driver is initialized */
    Status = Can_44_TCAN4x5x_ReportDetUninit
                                      (CAN_44_TCAN4x5x_SID_MAINFUNCTION_BUSOFF);
    if(E_OK == Status)
    #else
    if(CAN_44_TCAN4x5x_E_UNINIT != Can_44_TCAN4x5x_DriverState)
    #endif // (CAN_44_TCAN4x5x_DEV_ERROR_DETECT == STD_ON)
    {
        /** Busoff event handling */
        for(CtrlIndex=CAN_44_TCAN4x5x_ZERO;
        CtrlIndex < (uint8)CAN_44_TCAN4x5x_NUM_CONTROLLER_MAX; CtrlIndex++)
        {
            if(Can_44_TCAN4x5x_ControllerSetting_PC[CtrlIndex].
            CanBusoffProcessing == CAN_44_TCAN4x5x_POLLING)
            {
                exec_context_backup = Can_44_TCAN4x5x_ExCntxt[CtrlIndex];
                Can_44_TCAN4x5x_ExCntxt[CtrlIndex] = MAIN_FUNCTION_CONTEXT;
                Can_44_TCAN4x5x_MCAN_BusoffProcess(CtrlIndex);
                Can_44_TCAN4x5x_ExCntxt[CtrlIndex] = exec_context_backup;
            }
        }
    }
}
#define CAN_44_TCAN4x5x_STOP_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"

#define CAN_44_TCAN4x5x_START_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"
/**
 * \verbatim
 *  Service Name...... : Can_44_TCAN4x5x_MainFunction_Mode
 *  Service ID (hex).. : 0x0c
 *  Sync/Async........ : NA
 *  Reentrancy........ : NA
 *  Parameters (IN)... : None
 *  Parameters (INOUT) : None
 *  Parameters (OUT).. : None
 *  Return Value...... : None
 *  Description....... : This function performs the polling of CAN controller mode
 *                       transitions.
 *  Design IDs Covered : CAN_DesignId_012
 *
 * \endverbatim
**/
FUNC(void, CAN_44_TCAN4x5x_CODE_FAST) Can_44_TCAN4x5x_MainFunction_Mode(void)
{
    VAR(uint8, AUTOMATIC) CtrlIndex;
    #if (CAN_44_TCAN4x5x_DEV_ERROR_DETECT == STD_ON)
    VAR(Std_ReturnType, AUTOMATIC) Status;
    Can_44_TCAN4x5x_ApiId = CAN_44_TCAN4x5x_SID_MAINFUNCTION_MODE;
    #endif /** CAN_44_TCAN4x5x_DEV_ERROR_DETECT == STD_ON */

    #if (CAN_44_TCAN4x5x_DEV_ERROR_DETECT == STD_ON)
    Status = E_OK;

    /** Check if CAN driver is initialized */
    Status = Can_44_TCAN4x5x_ReportDetUninit
                                    (CAN_44_TCAN4x5x_SID_MAINFUNCTION_MODE);
    if(E_OK == Status)
    #endif // (CAN_44_TCAN4x5x_DEV_ERROR_DETECT == STD_ON)
    {
        /** Notify successful state transition to upper layer */
        for(CtrlIndex=CAN_44_TCAN4x5x_ZERO;
              CtrlIndex< (uint8)CAN_44_TCAN4x5x_NUM_CONTROLLER_MAX; CtrlIndex++)
        {
            if (Can_44_TCAN4x5x_ControllerIf_State[CtrlIndex] ==
                                    CAN_44_TCAN4x5x_MODE_TRANSITION_IN_PROGRESS)
            {
                if (E_OK == Can_44_TCAN4x5x_MCAN_SetMode(CtrlIndex,
                                                    CAN_44_TCAN4x5x_STATE_IDLE))
                {
                    Can_44_TCAN4x5x_ControllerIf_State[CtrlIndex] =
                                                        (uint8)CANIF_CS_STOPPED;
                    Can_44_TCAN4x5x_ControllerMode[CtrlIndex] =
                                                         CAN_T_STOP;
                    CanIf_ControllerModeIndication(CtrlIndex,
                  (CanIf_ControllerModeType)
                  Can_44_TCAN4x5x_ControllerIf_State[CtrlIndex]);
                }
            }
        }
    }
}
#define CAN_44_TCAN4x5x_STOP_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"

#if (CAN_44_TCAN4x5x_PUBLIC_ICOM_SUPPORT == STD_ON)
#define CAN_44_TCAN4x5x_START_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"
/**
 * \verbatim
 *  Service Name...... : Can_44_TCAN4x5x_SetIcomConfiguration
 *  Service ID (hex).. : 0xf
 *  Sync/Async........ : Asynchronous
 *  Reentrancy........ : Reentrant only for different controller Ids
 *  Parameters (IN)... : Controller, ConfigurationId
 *  Parameters (INOUT) : None
 *  Parameters (OUT).. : None
 *  Return Value...... : E_OK, E_NOT_OK
 *  Description....... : This service shall change the Icom Configuration of a
 *                       CAN controller to the requested one.
 *  Design IDs Covered : CAN_DesignId_013
 *
 * \endverbatim
**/
FUNC(Std_ReturnType, CAN_44_TCAN4x5x_CODE_FAST)
 Can_44_TCAN4x5x_SetIcomConfiguration(VAR(uint8, AUTOMATIC) Controller,
                            VAR(IcomConfigIdType, AUTOMATIC) ConfigurationId)
{
    VAR(Std_ReturnType, AUTOMATIC) LddStdRetVal;

    /** Declare local pointer for ICOM config */
    P2CONST(Can_44_TCAN4x5x_IcomConfigType, AUTOMATIC,
                                       CAN_44_TCAN4x5x_APPL_CONST)LpIcomconfig;

    /** Declare local structure for RAM configuration */
    VAR(Can_44_TCAN4x5x_MsgRamConfigType, AUTOMATIC)LCanMsgRamConfig;
    P2CONST(Can_44_TCAN4x5x_MsgRamConfigType, AUTOMATIC,
                                    CAN_44_TCAN4x5x_APPL_DATA)LpCanMsgRamConfig;

    /** Local pointer to the config structure for
                            wakeup causes of received messages */
    P2CONST(Can_44_TCAN4x5x_IcomRxMessageType, AUTOMATIC,
                                CAN_44_TCAN4x5x_APPL_CONST) LpIcomRxmessagePtr;

    P2CONST(Can_44_TCAN4x5x_RxObjFilterConfigType, AUTOMATIC,
    CAN_44_TCAN4x5x_APPL_CONST)LpCanRxObjFilterIComConfig;

    VAR(uint8, AUTOMATIC) LcanIcomRxMessages_size;
    VAR(uint8, AUTOMATIC) filter_size;
    VAR(uint32, AUTOMATIC) Lret_val_set_mode = (uint32)CAN_NOT_OK;
    VAR(uint8, AUTOMATIC) idx;
    VAR(uint32, AUTOMATIC) timeout = CAN_44_TCAN4x5x_INIT_WAIT_TIMEOUT;
    LddStdRetVal = E_OK;

    #if (CAN_44_TCAN4x5x_DEV_ERROR_DETECT == STD_ON)
    /** Check if ConfigurationId neither 0 nor any of the configured
                                                            CanIcomConfigId */
    if (ConfigurationId > CAN_44_TCAN4x5x_ICOM_CONFIG_MAX)
    {
        (void)Det_ReportError(CAN_44_TCAN4x5x_MODULE_ID,
                              CAN_44_TCAN4x5x_INSTANCE_ID,
                              CAN_44_TCAN4x5x_SET_ICOMCONFIGURATION_INIT,
                              CAN_44_TCAN4x5x_E_ICOM_CONFIG_INVALID);
        LddStdRetVal = E_NOT_OK;
    }

    /** Check if any DET error is detected */
    if ((Std_ReturnType)E_OK == LddStdRetVal)
    #endif // (CAN_44_TCAN4x5x_DEV_ERROR_DETECT == STD_ON)
    {
        if (ConfigurationId != CAN_44_TCAN4x5x_ZERO)
        {
            /** Activation of pretending networking */
            /** Set controller mode to idle */
            do
            {
                Lret_val_set_mode = Can_44_TCAN4x5x_MCAN_SetMode(Controller,
                                                    CAN_44_TCAN4x5x_STATE_IDLE);
                timeout--;
            }while((Lret_val_set_mode == (uint32)CAN_BUSY) &&
                                                         (timeout > (uint32)0));
            LpCanMsgRamConfig = &LCanMsgRamConfig;
            /** Initialization of local pointer of first ICOM config structure */
            LpIcomconfig =
             &Can_44_TCAN4x5x_Icomconfig[ConfigurationId - CAN_44_TCAN4x5x_ONE];
            /** Get the value of number of rx messages configured */
            LcanIcomRxMessages_size = LpIcomconfig->canIcomRxMessages_size;
            /** Fill the RAM init structure for activation */
            LCanMsgRamConfig.CanStdandardIDFiltersCount =
                                                    LcanIcomRxMessages_size;
            LCanMsgRamConfig.CanExtendedIDFiltersCount = CAN_44_TCAN4x5x_ZERO;
            LCanMsgRamConfig.CanFifo0BufferCount = LcanIcomRxMessages_size;
            LCanMsgRamConfig.CanFifo1BufferCount = CAN_44_TCAN4x5x_ZERO;
            LCanMsgRamConfig.CanDedicatedRxBufferCount = CAN_44_TCAN4x5x_ZERO;
            LCanMsgRamConfig.CanTxBufferCount = CAN_44_TCAN4x5x_ZERO;

            /** Get the value of ICOM RX message structure pointer in the
                                                      local pointer variable */
            LpIcomRxmessagePtr = LpIcomconfig->CanIcomRxMessagePtr;

            /** Local pointer for Icom receiver object filter */
            LpCanRxObjFilterIComConfig =
                               LpIcomRxmessagePtr->CanRxObjFilterIComConfigPtr;

            /** Initialization of RAM */
            Can_44_TCAN4x5x_MCAN_MessageRAM_Init(Controller, LpCanMsgRamConfig);
            /** Invoke global filter with ICOM filter values */
            Can_44_TCAN4x5x_MCAN_ConfigureGlobalFilters(Controller,
                                            LpCanRxObjFilterIComConfig,
                                                    LcanIcomRxMessages_size);
            Lret_val_set_mode = Can_44_TCAN4x5x_MCAN_SetMode(Controller,
                                                CAN_44_TCAN4x5x_STATE_RUNNING);
        }
        else /** Disable Icom and enable normal configuration */
        {
            filter_size =
            Can_44_TCAN4x5x_ConfigPtr->CanControllerConfiguration[Controller].
                                    CanMsgRamConfig.CanStdandardIDFiltersCount +
            Can_44_TCAN4x5x_ConfigPtr->CanControllerConfiguration[Controller].
                                    CanMsgRamConfig.CanExtendedIDFiltersCount;

            /** Deactivation of pretending networking */
            Lret_val_set_mode = Can_44_TCAN4x5x_MCAN_SetMode(Controller,
                                                   CAN_44_TCAN4x5x_STATE_IDLE);
            /** Normal mode RAM initialization */
            Can_44_TCAN4x5x_MCAN_MessageRAM_Init(Controller,
            &Can_44_TCAN4x5x_ConfigPtr->CanControllerConfiguration[Controller].
            CanMsgRamConfig);
            /** Normal mode filter configuration */
            Can_44_TCAN4x5x_MCAN_ConfigureGlobalFilters(Controller,
             Can_44_TCAN4x5x_ConfigPtr->CanControllerConfiguration[Controller].
             CanRxObjFilterConfigPtr, filter_size);
            Lret_val_set_mode = Can_44_TCAN4x5x_MCAN_SetMode(Controller,
                                             CAN_44_TCAN4x5x_STATE_RUNNING);
        }

        if(Lret_val_set_mode == (uint32)E_OK)
        {
            Can_44_TCAN4x5x_GComConfigId[Controller] = ConfigurationId;
            #if(CAN_44_TCAN4x5x_ICOM_COUNTER_VALUE == STD_ON)
            for (idx=(uint8)0; idx<(uint8)CAN_44_TCAN4x5x_ICOM_RX_MESSAGE_TOTAL;
            idx++)
            {
                Can_44_TCAN4x5x_IcomMsgCounter[Controller][idx] = (uint8)0;
            }
            #endif
            /** Call CanIf_CurrentIcomConfiguration */
            CanIf_CurrentIcomConfiguration(Controller, ConfigurationId,
                                                        ICOM_SWITCH_E_OK);
        }
        else
        {
            CanIf_CurrentIcomConfiguration(Controller, ConfigurationId,
                                                    ICOM_SWITCH_E_FAILED);
        }
    }
    return(LddStdRetVal);
}
#define CAN_44_TCAN4x5x_STOP_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"
#endif // #if (CAN_44_TCAN4x5x_PUBLIC_ICOM_SUPPORT == STD_ON)

#if (CAN_44_TCAN4x5x_DEV_ERROR_DETECT == STD_ON)
#define CAN_44_TCAN4x5x_START_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"
/**
 * \verbatim
 *  Service Name...... : Internal Function
 *  Service ID (hex).. : None
 *  Sync/Async........ : Synchronous
 *  Reentrancy........ : Reentrant
 *  Parameters (IN)... : ServiceId - Service Id of the calling API
 *  Parameters (INOUT) : None
 *  Parameters (OUT).. : None
 *  Return Value...... : E_OK, E_NOT_OK
 *  Description....... : Checks the status of CAN driver initialization, If CAN
 *                       driver is not initialized, reports to DET.
 *  Design IDs Covered :
 *
 * \endverbatim
**/
/** MISRA DIR 1.1 VIOLATION: JUSTIFICATION_1 */
LOCAL_INLINE FUNC(Std_ReturnType, CAN_44_TCAN4x5x_CODE_FAST)
               Can_44_TCAN4x5x_ReportDetUninit(VAR(uint8, AUTOMATIC) ServiceId)
{
    VAR(Std_ReturnType, AUTOMATIC) RetVal;
    RetVal = E_OK;
    /** Check if CAN driver is un-initialized */
    if((uint8)CAN_44_TCAN4x5x_UNINIT == (uint8)Can_44_TCAN4x5x_DriverState)
    {
        /** Report to DET */
        (void)Det_ReportError(CAN_44_TCAN4x5x_MODULE_ID,
                              CAN_44_TCAN4x5x_INSTANCE_ID,
                              ServiceId,
                              CAN_44_TCAN4x5x_E_UNINIT);
        RetVal = E_NOT_OK;
    }
    return(RetVal);
}
#define CAN_44_TCAN4x5x_STOP_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"
#endif // (CAN_44_TCAN4x5x_DEV_ERROR_DETECT == STD_ON)

#if (CAN_44_TCAN4x5x_DEV_ERROR_DETECT == STD_ON)
#define CAN_44_TCAN4x5x_START_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"
/**
 * \verbatim
 *  Service Name...... : Internal Function
 *  Service ID (hex).. : None
 *  Sync/Async........ : Synchronous
 *  Reentrancy........ : Reentrant
 *  Parameters (IN)... : ControllerId - CAN Controller Id, ServiceId - Service
 *                       Id of the calling API
 *  Parameters (INOUT) : None
 *  Parameters (OUT).. : None
 *  Return Value...... : E_OK, E_NOT_OK
 *  Description....... : Checks if ControllerId is valid or not, If ControllerId
 *                       is not valid, then report to DET.
 *  Design IDs Covered :
 *
 * \endverbatim
**/
/** MISRA DIR 1.1 VIOLATION: JUSTIFICATION_1 */
LOCAL_INLINE FUNC(Std_ReturnType, CAN_44_TCAN4x5x_CODE_FAST)
 Can_44_TCAN4x5x_ReportDetParamController(VAR(uint8, AUTOMATIC) ControllerId,
                                          VAR(uint8, AUTOMATIC) ServiceId)
{
    VAR(Std_ReturnType, AUTOMATIC) RetVal;

    /** 'ControllerId' is out of range */
    if(ControllerId < CAN_44_TCAN4x5x_NUM_CONTROLLER_MAX)
    {
        RetVal = E_OK;
    }
    else
    {

        /** Report to DET */
        (void)Det_ReportError(CAN_44_TCAN4x5x_MODULE_ID,
                              CAN_44_TCAN4x5x_INSTANCE_ID,
                              ServiceId,
                              CAN_44_TCAN4x5x_E_PARAM_CONTROLLER);
        RetVal = E_NOT_OK ;

    }
    return(RetVal);
}
#define CAN_44_TCAN4x5x_STOP_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"
#endif

#if (CAN_44_TCAN4x5x_DEV_ERROR_DETECT == STD_ON)
#define CAN_44_TCAN4x5x_START_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"
/**
 * \verbatim
 *  Service Name...... : Internal Function
 *  Service ID (hex).. : None
 *  Sync/Async........ : Synchronous
 *  Reentrancy........ : Non-Reentrant
 *  Parameters (IN)... : Controller - CAN Controller Id, Transition - Requested
 *                       transition
 *  Parameters (INOUT) : None
 *  Parameters (OUT).. : None
 *  Return Value...... : E_OK, E_NOT_OK
 *  Description....... : Checks the validity of software triggered CAN
 *                       Controller state transition.
 *  Design IDs Covered :
 *
 * \endverbatim
**/
/** MISRA DIR 1.1 VIOLATION: JUSTIFICATION_1 */
LOCAL_INLINE FUNC(Std_ReturnType, CAN_44_TCAN4x5x_CODE_FAST)
Can_44_TCAN4x5x_ModeDetCheck(VAR(uint8, AUTOMATIC) Controller,
                             VAR(Can_StateTransitionType,
                                                         AUTOMATIC) Transition)
{
    VAR(Std_ReturnType, AUTOMATIC) RetVal;
    RetVal = E_OK;

    switch (Transition)
    {
        case CAN_T_START :
        {
            /** if current mode is not STOPPED */
            if (Can_44_TCAN4x5x_ControllerMode[Controller] !=
                                                        CAN_T_STOP)
            {
                RetVal = E_NOT_OK;
            }
            break ;
        }
        case CAN_T_SLEEP :
        {
            /** if current mode is neither SLEEP nor STOPPED */
            if((Can_44_TCAN4x5x_ControllerMode[Controller] !=
                                                    CAN_T_SLEEP) &&
                (Can_44_TCAN4x5x_ControllerMode[Controller] !=
                                                    CAN_T_STOP))
            {
                RetVal = E_NOT_OK;
            }
            break ;
        }
        case CAN_T_STOP :
        {
            /** if current mode is neither STARTED nor STOPPED */
            if((Can_44_TCAN4x5x_ControllerMode[Controller] !=
                                                    CAN_T_START) &&
                (Can_44_TCAN4x5x_ControllerMode[Controller] !=
                                                    CAN_T_STOP))
            {
                RetVal = E_NOT_OK;
            }
            break ;
        }
        case CAN_T_WAKEUP :
        {
            /** if current mode is neither SLEEP nor STOPPED */
            if((Can_44_TCAN4x5x_ControllerMode[Controller] !=
                                                   CAN_T_SLEEP) &&
                (Can_44_TCAN4x5x_ControllerMode[Controller] !=
                                                   CAN_T_STOP))
            {
                RetVal = E_NOT_OK;
            }
            break ;
        }
        default:
        {
            /** Invalid state is requested */
            RetVal = E_NOT_OK;
            break;
        }
    }
    return(RetVal);
}
#define CAN_44_TCAN4x5x_STOP_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"
#endif // (CAN_44_TCAN4x5x_DEV_ERROR_DETECT == STD_ON)

#if (CAN_44_TCAN4x5x_DEV_ERROR_DETECT == STD_ON)
#define CAN_44_TCAN4x5x_START_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"
/**
 * \verbatim
 *  Service Name...... : Internal Function
 *  Service ID (hex).. : None
 *  Sync/Async........ : Synchronous
 *  Reentrancy........ : Reentrant
 *  Parameters (IN)... : Hth, PduInfo
 *  Parameters (INOUT) : None
 *  Parameters (OUT).. : None
 *  Return Value...... : E_OK, E_NOT_OK
 *  Description....... : Checks DET Handles for Can_44_TCAN4x5x_Write API
 *  Design IDs Covered :
 *
 * \endverbatim
**/
/** MISRA DIR 1.1 VIOLATION: JUSTIFICATION_1 */
LOCAL_INLINE FUNC(Std_ReturnType, CAN_44_TCAN4x5x_CODE_FAST)
 Can_44_TCAN4x5x_WriteDetCheck(VAR(Can_HwHandleType, AUTOMATIC) Hth,
 P2CONST(Can_PduType, AUTOMATIC, CAN_44_TCAN4x5x_APPL_DATA) PduInfo)
{
    VAR(Std_ReturnType, AUTOMATIC) RetVal;
    VAR(uint8, AUTOMATIC) ctrlId;
    VAR(uint8, AUTOMATIC) index;

    RetVal = E_OK;

    /** Check if 'Hth' is a valid transmit handle */
    if((Hth < Can_44_TCAN4x5x_ConfigPtr->CanHRHobjectsCount) ||
            (Hth >= (Can_44_TCAN4x5x_ConfigPtr->CanHRHobjectsCount +
                            Can_44_TCAN4x5x_ConfigPtr->CanHTHobjectsCount)))
    {
        /** Report to DET */
        (void)Det_ReportError(CAN_44_TCAN4x5x_MODULE_ID,
                              CAN_44_TCAN4x5x_INSTANCE_ID,
                              CAN_44_TCAN4x5x_SID_WRITE,
                              CAN_44_TCAN4x5x_E_PARAM_HANDLE);
        RetVal = E_NOT_OK;
    }
    /** Check if PduInfo is a null pointer */
    else if (PduInfo == NULL_PTR)
    {
        /** Report to DET */
        (void)Det_ReportError(CAN_44_TCAN4x5x_MODULE_ID,
                             CAN_44_TCAN4x5x_INSTANCE_ID,
                             CAN_44_TCAN4x5x_SID_WRITE,
                             CAN_44_TCAN4x5x_E_PARAM_POINTER);
        RetVal = E_NOT_OK;
    }
	else
	{
	    ctrlId = Can_44_TCAN4x5x_ConfigPtr->CanHTHMappingConfigPtr
        [Hth - Can_44_TCAN4x5x_ConfigPtr->CanHRHobjectsCount].CanCtrlId;
        index = Can_44_TCAN4x5x_ConfigPtr->CanHTHMappingConfigPtr
        [Hth - Can_44_TCAN4x5x_ConfigPtr->CanHRHobjectsCount].CanHthIndex;
        /** check if DLC is greater than 8 */
        if((Can_44_TCAN4x5x_ControllerSetting_PC[ctrlId].
            CanFDBaudRateEnabled != 1u) &&
           (PduInfo->length > CAN_44_TCAN4x5x_MAX_DATA_LENGTH))
        {
            /** Report to DET */
            (void)Det_ReportError(CAN_44_TCAN4x5x_MODULE_ID,
                                CAN_44_TCAN4x5x_INSTANCE_ID,
                                CAN_44_TCAN4x5x_SID_WRITE,
                                CAN_44_TCAN4x5x_E_PARAM_DLC);
            RetVal = E_NOT_OK;
        }
        else if(Can_44_TCAN4x5x_ControllerSetting_PC[ctrlId].
                CanFDBaudRateEnabled == 1u)
        {
            /** Valid CAN FD lengths: 0-8,12,16,20,24,32,48,64 bytes */
            if ((PduInfo->length > CAN_44_TCAN4x5x_MAX_DATA_LENGTH) &&
                           (PduInfo->length <= CAN_44_TCAN4x5x_MAX_FD_LENGTH) &&
                        ((PduInfo->id & CAN_44_TCAN4x5x_FRAME_TYPE_MASK) == 0U))
            {
                /** Report to DET */
                (void)Det_ReportError(CAN_44_TCAN4x5x_MODULE_ID,
                                    CAN_44_TCAN4x5x_INSTANCE_ID,
                                    CAN_44_TCAN4x5x_SID_WRITE,
                                    CAN_44_TCAN4x5x_E_PARAM_DLC);
                RetVal = E_NOT_OK;
            }
            else if(PduInfo->length > CAN_44_TCAN4x5x_FD_MAX_DATA_SIZE(ctrlId))
            {
                /** Report to DET */
                (void)Det_ReportError(CAN_44_TCAN4x5x_MODULE_ID,
                                    CAN_44_TCAN4x5x_INSTANCE_ID,
                                    CAN_44_TCAN4x5x_SID_WRITE,
                                    CAN_44_TCAN4x5x_E_PARAM_DLC);
                RetVal = E_NOT_OK;
            }
            else
            {
                /** Do Nothing */
            }
        }
        /** Check if sdu is a null pointer */
        else if ((PduInfo->sdu == NULL_PTR) &&
          (Can_44_TCAN4x5x_ConfigPtr->CanControllerConfiguration[ctrlId].
          CanTxObjConfigPtr[index].CanTriggerTransmitEnable == STD_OFF))
        {
            /** Report to DET */
            (void)Det_ReportError(CAN_44_TCAN4x5x_MODULE_ID,
                                 CAN_44_TCAN4x5x_INSTANCE_ID,
                                 CAN_44_TCAN4x5x_SID_WRITE,
                                 CAN_44_TCAN4x5x_E_PARAM_POINTER);
            RetVal = E_NOT_OK;
        }
        else
        {
            /** Nothing to do */
        }
    }
    return(RetVal);
}

#define CAN_44_TCAN4x5x_STOP_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"
#endif

#ifdef __cplusplus
}
#endif
/*******************************************************************************
    Version     Date            Author           Description
    -------     ----            ------           -----------
    1.0.0    11-05-2017       Lakshmi H         Initial Version
    1.0.1    07-08-2017       Lakshmi H         Performed QAC Analysis and
											    justification is given for Misra
											    Rule voilations
    1.0.2    07-08-2017       Lakshmi H         Design Id's has been added and
                                                Commenting style is changed to
											    Doxygen style
    1.0.3    17-11-2017       Lakshmi H         TCAN1043 code added and Software
                                                version changed
******************************************************************************/
/*******************************************************************************
 *  End of File: Can_44_TCAN4x5x.c
 ******************************************************************************/
