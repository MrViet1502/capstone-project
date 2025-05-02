//----------------------------------------------------------------------------
// CONFIDENTIAL
//
// COPYRIGHT RESERVED, 2018 Robert Bosch (Australia) Pty Ltd.
// All rights reserved.  The reproduction, distribution and utilisation of
// this document as well as the communication of its contents to others
// without explicit authorisation is prohibited.  Offenders will be held liable
// for the payment of damages.  All rights reserved in the event of the grant
// of a patent, utility model or design.
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//! \file
//! \brief  Configuration for Imc
//----------------------------------------------------------------------------

#if defined(RBD_PK_CPU_SPC58ECx) && defined(RBD_PK_APP)
#define RBDIMC_WITH_RTE_APP
#endif

//============================================================================
//  Includes
//============================================================================

#include <stdlib.h>     // for NULL

#include "rbdImc.h"
#include "rbdImc_Private.h"
#include "ImcIf_Api.h"
#include "rbdElTime_Api.h"
#include "rbdImc_CfgPrj.h"

#ifdef RBDIMC_WITH_RTE_APP
#include "Rte_rbdImc.h" /* PRQA S 0857 */ /* MD_MSR_1.1_857 */
#include "Dio.h"
#include "rbdSpecialRam_Api.h"
#endif

//============================================================================
// Local Macros
//============================================================================

#define RBDIMC_BLE_RESET_CYCLES 1       //!< Number of call cycles to hold BLE chip in reset



//============================================================================
// Local Types
//============================================================================
#ifdef RBDIMC_WITH_RTE_APP
static rbd_ElTime_rbTimer_st powerModeTxTimer;
#endif

//============================================================================
// Local Function Prototypes
//============================================================================

//============================================================================
// Global Variables
//============================================================================




//============================================================================
// Module Variables
//============================================================================
#ifdef RBDIMC_WITH_RTE_APP
static uint8 rbdImc_BleResetDelay = 0;      //!< Number of cycles until BLE reset is released
static boolean rbdImc_IsFastSleepActive = FALSE;
#endif

//============================================================================
// Function declarations.
//============================================================================








#ifdef RBDIMC_WITH_RTE_APP
//----------------------------------------------------------------------------
//! \brief  RTE entry point for Imc stack
//!
//! This function is required because of a naming difference in the Autosar
//! configuration.
//----------------------------------------------------------------------------
FUNC(void, rbd_cddImc_CODE) rbd_cddImc_Init(void)
{
    rbd_ElTime_rbTimerInit( &powerModeTxTimer );

    // If there was a power-on reset, reset the BLE chip
    if( TRUE == rbdSpecialRam_isPOR() )
    {
        Dio_WriteChannel( DioConf_DioChannel_dOP_BLE_RST, STD_LOW );
        rbdImc_BleResetDelay = RBDIMC_BLE_RESET_CYCLES;
    }
}




//----------------------------------------------------------------------------
//! \brief  Additional cyclic tasks for the MasterMcu
//----------------------------------------------------------------------------
void rbd_cddImc_Cyclic( void )
{
    uint8 stateNm_u8;

    //Do not send NM messages if Fast sleep is Active
    if( FALSE == rbdImc_IsFastSleepActive)
    {
        (void)Rte_Read_RP_NmState_State(&stateNm_u8);

        // Check that data has been received successful
        if(  (stateNm_u8 == RBD_PK_NM_STATE_NORMAL_OPERATION) || (stateNm_u8 == RBD_PK_NM_STATE_REPEAT_MESSAGE))
        {
            if(RBD_ELTIME_RBTIMER_STATE_RUNNING != rbd_ElTime_rbTimerUpdate(&powerModeTxTimer))
            {
                ImcIf_CfgMcu_TxCanNm();
                rbd_ElTime_rbTimerStart(&powerModeTxTimer, RBDIMC_CANNM_PERIOD_MS);
            }
        }
        else
        {
            rbd_ElTime_rbTimerStop(&powerModeTxTimer);
        }
    }
    else
    {
        //MISRA
    }

    // Delay a number of cycles, then take BLE chip out of reset.
    if( rbdImc_BleResetDelay > 0u )
    {
        rbdImc_BleResetDelay--;
        if( 0u == rbdImc_BleResetDelay )
        {
            Dio_WriteChannel( DioConf_DioChannel_dOP_BLE_RST, STD_HIGH );
        }
    }
}


FUNC(void, rbd_cddImc_CODE) rbd_cddImc_FastSleepReq(uint8 mode)
{
    ImcIf_CfgMcu_TxFastSleep(mode);

    //Fast SLEEP is Active
    rbdImc_IsFastSleepActive = TRUE;
}



#endif // RBDIMC_WITH_RTE_APP


//-------------------- End of File -------------------------------------------
