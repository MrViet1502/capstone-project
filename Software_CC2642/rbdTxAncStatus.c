//----------------------------------------------------------------------------
// CONFIDENTIAL
//
// COPYRIGHT Robert Bosch (Australia) Pty. Ltd. 2023.
// All rights reserved, also regarding any disposal, exploitation,
// reproduction, editing, distribution, as well as in the event of
// applications for industrial property rights.
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
/** \file
\brief Transmit Anchor Status to Master

*/
//----------------------------------------------------------------------------
#include <string.h>
#include "CanNm.h"
#include "PduR.h"
#include "rbDtcMgr_Api.h"
#include "rbdTxAncStatus.h"
#include "rbdRxSysCtrl.h"
#include "rbdElTime_Api.h"
#if ((1u == RBD_PK_12H_REBOOT) || defined(GTESTING))
#include "rbdPwrAncRam.h"
#endif
//============================================================================
// Local Macros
//============================================================================

//============================================================================
// Public Variables
//============================================================================


//============================================================================
// Local Types
//============================================================================

//============================================================================
// Local Variables
//============================================================================

static rbd_ElTime_rbTimer_st rbdTxAncStatus_Cyclic_Timer;
static boolean rbdTxAncStatus_TxEnabled;
//============================================================================
// Local Function Prototypes
//============================================================================
static void rbdTxAncStatus_Transmit(void);

//----------------------------------------------------------------------------
/** \brief  Initialize this module
*///--------------------------------------------------------------------------
void rbdTxAncStatus_Init(void)
{
    //Initialize Cyclic Timer
    rbd_ElTime_rbTimerInit(&rbdTxAncStatus_Cyclic_Timer);
}

//----------------------------------------------------------------------------
/** \brief  Module Cyclic Function

@rst
..  impl::
    :id: rbdTxAncStatus_Cyclic
    :links_satisfies: rbdTxAncStatus_Tx_Trigger_1, rbdTxAncStatus_NmState_Validation_1
@endrst
*///--------------------------------------------------------------------------
void rbdTxAncStatus_Cyclic(void)
{
    CanNm_StateType NmState;
    CanNm_ModeType NmMode = NM_MODE_BUS_SLEEP;
    static CanNm_ModeType prev_NmMode = NM_MODE_BUS_SLEEP;
    uint32 cyclicTime = cp_BB_APS_CanStatusMsgCycleTime;
    Std_ReturnType NmStateResult;

    NmStateResult = CanNm_GetState(&NmState, &NmMode);

    //Is NM state valid?
    if((NmStateResult != E_OK) || (NmState >= NM_STATE_MAX_NUMBER)) /*AXIVION Line CertC-STR34 : comparison is of same types*/
    {
        //No, set the Nmode to Bus Sleep
        NmMode = NM_MODE_BUS_SLEEP;
    }

    //Is NM State Repeat Message?
    if(NmState == NM_STATE_REPEAT_MESSAGE) /*AXIVION Line CertC-STR34 : comparison is of same types*/
    {
        //Yes, Set cyclic time to Fast Cyclic
        cyclicTime = cp_BB_APS_CanStatusMsgCycleTimeFast;
    }

    //Did Nm Mode changed to Network Mode?
    if((NM_MODE_NETWORK == NmMode) && (NM_MODE_NETWORK != prev_NmMode)) /*AXIVION Line CertC-STR34 : comparison is of same types*/
    {
        //Yes, immediately transmit Anchor Status
        rbdTxAncStatus_Transmit();

        //Start cyclic timer.
        rbd_ElTime_rbTimerStart(&rbdTxAncStatus_Cyclic_Timer,cyclicTime);
    }
    //Is NM Mode in Network Mode.
    else if(NM_MODE_NETWORK == NmMode) /*AXIVION Line CertC-STR34 : comparison is of same types*/
    {
        //Yes, Check if cyclic timer elapsed.
        rbd_ElTime_rbTimerState_t timerState = rbd_ElTime_rbTimerUpdate(&rbdTxAncStatus_Cyclic_Timer);

        //Cyclic Timer Elapsed?
        if (RBD_ELTIME_RBTIMER_STATE_ELAPSED == timerState) /*AXIVION Line CertC-STR34 : comparison is of same types*/
        {
            //Yes, Restart cyclic timer.
            rbd_ElTime_rbTimerStart(&rbdTxAncStatus_Cyclic_Timer,cyclicTime);

            //Transmit Anchor Status.
            rbdTxAncStatus_Transmit();
        }
    }
    else
    {
        //Do Nothing
    }

    prev_NmMode = NmMode;
}

//----------------------------------------------------------------------------
/** \brief  Transmit Anchor Status to Master

@rst
..  impl::
    :id: rbdTxAncStatus_Transmit
    :links_satisfies: rbdTxAncStatus_Tx_Format_2
@endrst
*///--------------------------------------------------------------------------
static void rbdTxAncStatus_Transmit(void)
{
    if(rbdTxAncStatus_TxEnabled == TRUE)
    {
        uint8 AncStatus[16] = {[0 ... 15] = 0};
        static uint8 dtcIndex = 0;
        uint8 dtcStatus[DTC_MGR_CFG_NUMBER_DTCS][5];
        uint16 dataLength = 0;
        PduInfoType PduInfo;

        //Populate Anchor Status Message
        AncStatus[0] = (uint8)(((uint8)rbdRxSysCtrl_GetBBAnchorPwrModeTarget_Rx() & 0x0Fu) | ((uint8)rbdPwrMgmtAnc_GetAnchorPwrMode() << 4u));
    #if ((1u == RBD_PK_12H_REBOOT) || defined(GTESTING))
        AncStatus[1] = (uint8)((rbdPwrMgmtAnc_GetBBUwbPwrMode() & 0x0Fu) | ((rbdPwrAncRam_SecureBootCounter_Read() & 0x03) << 4u));
    #else
        AncStatus[1] = (uint8)(rbdPwrMgmtAnc_GetBBUwbPwrMode() & 0x0Fu);
    #endif

        if(RB_UDS_14229_PR == DtcMgr_ApiUdsReadDtcSupported(&dtcStatus[0][0],&dataLength,(uint16)sizeof(dtcStatus)))
        {
            //3 bytes DTC Code
            AncStatus[8] = dtcStatus[dtcIndex][1];
            AncStatus[9] = dtcStatus[dtcIndex][2];
            AncStatus[10] = dtcStatus[dtcIndex][3];
            //1 Byte Dtc Status
            AncStatus[11] = dtcStatus[dtcIndex][4];
        }

        //Tx PDU
        PduInfo.SduDataPtr = AncStatus;
        PduInfo.SduLength = sizeof(AncStatus);

        //Transmit Anchor status.
        (void)PduR_Transmit(PduR_TxPdu_AnchorStatus, &PduInfo); /*AXIVION Line CertC-STR34 : comparison is of same types*/

        //Cyclic through different DTC every transmit.
        dtcIndex++;
        if(dtcIndex >= (uint8)DTC_MGR_CFG_NUMBER_DTCS) /*AXIVION Line CertC-STR34 : comparison is of same types*/
        {
            dtcIndex = 0;
        }
    }
}

//----------------------------------------------------------------------------
/** \brief Inform rbdTxAncStatus of the current CANIdx

\param[in] CanIdx current CANIdx

@rst
..  impl::
    :id: rbdTxAncStatus_SetCanIdx
    :links_satisfies: rbdTxAncStatus_NoTransmit_CanIdx_F_1
@endrst
*///----------------------------------------------------------------------------
void rbdTxAncStatus_SetCanIdx(uint8 CanIdx)
{
    if(CanIdx < 0xFu)
    {
        rbdTxAncStatus_TxEnabled = TRUE;
    }
    else
    {
        rbdTxAncStatus_TxEnabled = FALSE;
    }
}
