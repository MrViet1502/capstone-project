/* ======================================================================
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
 * ==================================================================== */

/*=============================================================================
|                       MISRA-C VIOLATIONS                                    |
=============================================================================*/
/*
JUSTIFICATION_1: <.........>
*/

#ifndef CAN_44_TCAN4x5x_COMPILER_CFG_H
#define CAN_44_TCAN4x5x_COMPILER_CFG_H

#ifdef __cplusplus
extern "C"{
#endif
/*=============================================================================
|                       INCLUDE FILES                                         |
=============================================================================*/



/*=============================================================================
|                       EXPORTED CONSTANTS and MACROS                         |
=============================================================================*/
/* [SWS_BSW_00059] Published information elements. */
#define COMPILER_VENDOR_ID_CFG   44u
#define COMPILER_MODULE_ID_CFG  198u

#define COMPILER_AR_RELEASE_MAJOR_VERSION_CFG     4u
#define COMPILER_AR_RELEASE_MINOR_VERSION_CFG     2u
#define COMPILER_AR_RELEASE_REVISION_VERSION_CFG  1u

#define COMPILER_SW_MAJOR_VERSION_CFG     1u
#define COMPILER_SW_MINOR_VERSION_CFG     0u
#define COMPILER_SW_PATCH_VERSION_CFG     0u


/* [SWS_COMPILER_00040] */
#define REGSPACE

#define WDG_44_TCAN4x5x_APPL_DATA

#define WDG_44_TCAN4x5x_CODE

#define WDG_44_TCAN4x5x_CODE_FAST

#define WDG_44_TCAN4x5x_CODE_SLOW

#define WDG_44_TCAN4x5x_VAR_CLEARED

#define WDG_44_TCAN4x5x_CONFIG_DATA
/* TCAN CAN driver */
#define CAN_44_TCAN4x5x_CODE

#define CAN_44_TCAN4x5x_CODE_FAST

#define CAN_44_TCAN4x5x_CODE_SLOW

#define CAN_44_TCAN4x5x_CONST

#define CAN_44_TCAN4x5x_CALIB

#define CAN_44_TCAN4x5x_CONFIG_DATA

#define CAN_44_TCAN4x5x_APPL_DATA
#define CAN_44_TCAN4x5x_APPL_CONST
#define CAN_44_TCAN4x5x_VAR_NO_INIT

#define CAN_44_TCAN4x5x_VAR_CLEARED

#define CAN_44_TCAN4x5x_VAR_POWER_ON_CLEARED

#define CAN_44_TCAN4x5x_VAR_INIT

#define CAN_44_TCAN4x5x_VAR_POWER_ON_INIT

#define CAN_44_TCAN4x5x_VAR_FAST_NO_INIT

#define CAN_44_TCAN4x5x_VAR_FAST_CLEARED

#define CAN_44_TCAN4x5x_VAR_FAST_POWER_ON_CLEARED

#define CAN_44_TCAN4x5x_VAR_FAST_INIT

#define CAN_44_TCAN4x5x_VAR_FAST_POWER_ON_INIT

#define CAN_44_TCAN4x5x_VAR_SLOW_NO_INIT

#define CAN_44_TCAN4x5x_VAR_SLOW_CLEARED

#define CAN_44_TCAN4x5x_VAR_SLOW_POWER_ON_CLEARED

#define CAN_44_TCAN4x5x_VAR_SLOW_INIT

#define CAN_44_TCAN4x5x_VAR_SLOW_POWER_ON_INIT

#define CAN_44_TCAN4x5x_INTERNAL_VAR_NO_INIT

#define CAN_44_TCAN4x5x_INTERNAL_VAR_CLEARED

#define CAN_44_TCAN4x5x_INTERNAL_VAR_POWER_ON_CLEARED

#define CAN_44_TCAN4x5x_INTERNAL_VAR_INIT

#define CAN_44_TCAN4x5x_INTERNAL_VAR_POWER_ON_INIT


/* TCAN CAN TRCV driver */
#define CANTRCV_44_TCAN4x5x_CODE

#define CANTRCV_44_TCAN4x5x_CODE_FAST

#define CANTRCV_44_TCAN4x5x_CODE_SLOW

#define CANTRCV_44_TCAN4x5x_CONST

#define CANTRCV_44_TCAN4x5x_CALIB

#define CANTRCV_44_TCAN4x5x_CONFIG_DATA

#define CANTRCV_44_TCAN4x5x_APPL_DATA
#define CANTRCV_44_TCAN4x5x_APPL_CONST
#define CANTRCV_44_TCAN4x5x_VAR_NO_INIT

#define CANTRCV_44_TCAN4x5x_VAR_CLEARED

#define CANTRCV_44_TCAN4x5x_VAR_POWER_ON_CLEARED

#define CANTRCV_44_TCAN4x5x_VAR_INIT

#define CANTRCV_44_TCAN4x5x_VAR_POWER_ON_INIT

#define CANTRCV_44_TCAN4x5x_VAR_FAST_NO_INIT

#define CANTRCV_44_TCAN4x5x_VAR_FAST_CLEARED

#define CANTRCV_44_TCAN4x5x_VAR_FAST_POWER_ON_CLEARED

#define CANTRCV_44_TCAN4x5x_VAR_FAST_INIT

#define CANTRCV_44_TCAN4x5x_VAR_FAST_POWER_ON_INIT

#define CANTRCV_44_TCAN4x5x_VAR_SLOW_NO_INIT

#define CANTRCV_44_TCAN4x5x_VAR_SLOW_CLEARED

#define CANTRCV_44_TCAN4x5x_VAR_SLOW_POWER_ON_CLEARED

#define CANTRCV_44_TCAN4x5x_VAR_SLOW_INIT

#define CANTRCV_44_TCAN4x5x_VAR_SLOW_POWER_ON_INIT

#define CANTRCV_44_TCAN4x5x_INTERNAL_VAR_NO_INIT

#define CANTRCV_44_TCAN4x5x_INTERNAL_VAR_CLEARED

#define CANTRCV_44_TCAN4x5x_INTERNAL_VAR_POWER_ON_CLEARED

#define CANTRCV_44_TCAN4x5x_INTERNAL_VAR_INIT

#define CANTRCV_44_TCAN4x5x_INTERNAL_VAR_POWER_ON_INIT


/* RTE driver */
#define RTE_CODE

#define RTE_CODE_FAST

#define RTE_CODE_SLOW

#define RTE_CONST

#define RTE_CALIB

#define RTE_CONFIG_DATA

#define RTE_APPL_DATA
#define RTE_APPL_CONST
#define RTE_VAR_NO_INIT

#define RTE_VAR_CLEARED

#define RTE_VAR_POWER_ON_CLEARED

#define RTE_VAR_INIT

#define RTE_VAR_POWER_ON_INIT

#define RTE_VAR_FAST_NO_INIT

#define RTE_VAR_FAST_CLEARED

#define RTE_VAR_FAST_POWER_ON_CLEARED

#define RTE_VAR_FAST_INIT

#define RTE_VAR_FAST_POWER_ON_INIT

#define RTE_VAR_SLOW_NO_INIT

#define RTE_VAR_SLOW_CLEARED

#define RTE_VAR_SLOW_POWER_ON_CLEARED

#define RTE_VAR_SLOW_INIT

#define RTE_VAR_SLOW_POWER_ON_INIT

#define RTE_INTERNAL_VAR_NO_INIT

#define RTE_INTERNAL_VAR_CLEARED

#define RTE_INTERNAL_VAR_POWER_ON_CLEARED

#define RTE_INTERNAL_VAR_INIT

#define RTE_INTERNAL_VAR_POWER_ON_INIT


/* ECUM driver */
#define ECUM_CODE

#define ECUM_CODE_FAST

#define ECUM_CODE_SLOW

#define ECUM_CONST

#define ECUM_CALIB

#define ECUM_CONFIG_DATA

#define ECUM_APPL_DATA
#define ECUM_APPL_CONST
#define ECUM_VAR_NO_INIT

#define ECUM_VAR_CLEARED

#define ECUM_VAR_POWER_ON_CLEARED

#define ECUM_VAR_INIT

#define ECUM_VAR_POWER_ON_INIT

#define ECUM_VAR_FAST_NO_INIT

#define ECUM_VAR_FAST_CLEARED

#define ECUM_VAR_FAST_POWER_ON_CLEARED

#define ECUM_VAR_FAST_INIT

#define ECUM_VAR_FAST_POWER_ON_INIT

#define ECUM_VAR_SLOW_NO_INIT

#define ECUM_VAR_SLOW_CLEARED

#define ECUM_VAR_SLOW_POWER_ON_CLEARED

#define ECUM_VAR_SLOW_INIT

#define ECUM_VAR_SLOW_POWER_ON_INIT

#define ECUM_INTERNAL_VAR_NO_INIT

#define ECUM_INTERNAL_VAR_CLEARED

#define ECUM_INTERNAL_VAR_POWER_ON_CLEARED

#define ECUM_INTERNAL_VAR_INIT

#define ECUM_INTERNAL_VAR_POWER_ON_INIT


/*=============================================================================
|                       EXPORTED STRUCTURES, ENUMS, TYPEDEFS                  |
=============================================================================*/



/*=============================================================================
|                       EXPORTED FUNCTIONS                                    |
=============================================================================*/



/*=============================================================================
|                       EXPORTED VARIABLES                                    |
=============================================================================*/



/*=============================================================================
|                       VERSION CHECK                                         |
=============================================================================*/



#ifdef __cplusplus
}
#endif

#endif /* CAN_44_TCAN4x5x_COMPILER_CFG_H */

