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
 *  \file     Can_44_TCAN4x5x_Irq.c
 *
 *  \brief    This file contains CAN IRQ Functions
 */
 
/*******************************************************************************
 *                      MISRA-C VIOLATIONS
 ******************************************************************************/
#ifdef __cplusplus
extern "C"{
#endif

/*******************************************************************************
 *                      INCLUDE FILES
 ******************************************************************************/
#include "Can_44_TCAN4x5x.h"
#include "Can_44_TCAN4x5x_MCan.h"
/*******************************************************************************
 *                      GLOBAL CONSTANTS
 ******************************************************************************/

/*******************************************************************************
 *                      GLOBAL VARIABLES
 ******************************************************************************/

/*******************************************************************************
 *                      LOCAL STRUCTURES, ENUMS, TYPEDEFS
 ******************************************************************************/

/*******************************************************************************
 *                      FUNCTION DEFINITIONS
 ******************************************************************************/
#if (CAN_44_TCAN4x5x_NUM_CONTROLLER_MAX > 0u)
#define CAN_44_TCAN4x5x_START_SEC_CODE_FAST
#include "Can_44_TCAN4x5x_MemMap.h"

FUNC(void, CAN_44_TCAN4x5x_CODE_FAST)Can_44_TCAN4x5x_Irq_Handler
    (VAR(uint32, AUTOMATIC) aId)
{   
    if((Can_44_TCAN4x5x_ControllerSetting_PC[aId].CanRxProcessing == 
                                            CAN_44_TCAN4x5x_INTERRUPT)
    || (Can_44_TCAN4x5x_ControllerSetting_PC[aId].CanTxProcessing == 
                                            CAN_44_TCAN4x5x_INTERRUPT)
    || (Can_44_TCAN4x5x_ControllerSetting_PC[aId].CanBusoffProcessing == 
                                            CAN_44_TCAN4x5x_INTERRUPT))
    {
        Can_44_TCAN4x5x_MCAN_ProcessCanIRQLine0(aId);
    }
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
    1.0.0    11-05-2017       Lakshmi H        Initial Version
    1.0.1    07-08-2017       Lakshmi H        Performed QAC Analysis
    1.0.2    26-09-2017       Lakshmi H        Commenting style is changed to 
											   Doxygen style
    1.0.3    17-11-2017       Lakshmi H        TCAN1043 code added and Software
                                               version changed
 ******************************************************************************/
/*******************************************************************************
 *  End of File: Can_44_TCAN4x5x_Irq.c
 ******************************************************************************/
