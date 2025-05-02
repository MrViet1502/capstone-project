//----------------------------------------------------------------------------
// CONFIDENTIAL
//
// COPYRIGHT RESERVED, 2020 Robert Bosch (Australia) Pty Ltd.
// All rights reserved.  The reproduction, distribution and utilisation of
// this document as well as the communication of its contents to others
// without explicit authorisation is prohibited.  Offenders will be held liable
// for the payment of damages.  All rights reserved in the event of the grant
// of a patent, utility model or design.
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//! \file
//! \brief CanNm: CAN Network Management
//!
//! Module Description
//! The AUTOSAR CAN Network Management is a hardware independent protocol.
//! Its main purpose is to coordinate the transition between normal
//! operation and bus-sleep mode of the network.
//! In addition to the core functionality configurable features are provided
//! e.g. to implement a service to detect all present nodes or to detect if all
//! other nodes are ready to sleep.
//! The CAN Network Management (CanNm) function provides an adaptation between
//! Network Management Interface (NmIf) and CAN Interface (CanIf) module.
//----------------------------------------------------------------------------

//============================================================================
//  Includes
//============================================================================
#include <string.h>
#include "CanNm.h"
#include "CanNm_Cfg.h"
#include "CanNm_Cbk.h"
#include "CanNm_Private.h"
#include "PduR.h"
#include "rbdPlantData_Api.h"

#include "CanSm.h"
#include "rbdMacros.h"

#include "Det.h"
#include "Det_rbApi.h"
#include "CanIf.h"
#include "CanIf_Cfg.h"

#if defined(RBD_PK_APP) && !defined(RBD_PK_CPU_S32)
// Nm is compiled in the FBL, but it shouldn't be: see 620276
#include "Appl_NmCbk.h"
#endif

#include "rbdPwrReq.h"
//============================================================================
// Local Macros
//============================================================================



#define CANNM_TXMESSAGE_TIMEOUT     500u
#define CANNM_FRAME_MIN_LENGTH      2u
//============================================================================
// Global Variables
//============================================================================


//============================================================================
// Local Variables
//============================================================================
static CanNm_InfoType   CanNm_Info;
static const CanNm_InfoType* const CanNm_Info_p = &CanNm_Info;
static CanNm_MsgInfo_St CanNm_TxMsg_Info;
static CanNm_StateType CanNm_nextState;
static CanNm_AdditionalNetworkStateType CanNm_Network_State;
static boolean CanNm_CanFitted = TRUE;
static uint8 CanNm_PwrReqID;
static uint16 CanNm_ImmediateTransmissionCount;
//============================================================================
// Local Function Prototypes
//============================================================================
static boolean CanNm_IsModuleInitialized(void);
static void CanNm_SetState(const CanNm_StateType NmState);
static Std_ReturnType CanNm_TriggerTransmission ( void );
static void CanNm_MsgTimeoutTimerHandling(void);
static void CanNm_MsgCycleTimerHandling(const CanNm_StateType nmState);
static void CanNm_NmTimeoutTimerHandling(const CanNm_StateType nmState);
static void CanNm_NetworkRequestHandling(const CanNm_StateType nmState, const CanNm_AdditionalNetworkStateType networkState );
static void CanNm_RepeatMessageTimeoutHandling(const CanNm_StateType nmState);

//============================================================================
// Function declarations.
//============================================================================

//============================================================================
// Function Definitions.
//============================================================================

//----------------------------------------------------------------------------
//! \brief  Initialize the CanNm module
//----------------------------------------------------------------------------
void CanNm_Init ( void )
{
    CanNm_nextState = NM_STATE_BUS_SLEEP;
    CanNm_Network_State = NM_NETWORK_STATE_RELEASED;

    CanNm_Info.Initialized = (boolean)TRUE;
    CanNm_Info.CanNm_State = NM_STATE_BUS_SLEEP;
    CanNm_Info.TxEnabled = (boolean)TRUE;

    rbd_ElTime_rbTimerInit(&CanNm_Info.NmTimeoutTimer);
    rbd_ElTime_rbTimerInit(&CanNm_Info.MsgCycleTimer);
    rbd_ElTime_rbTimerInit(&CanNm_Info.MsgTimeoutTimer);

    CanNm_TxMsg_Info.Id = PduR_TxPdu_AnchorNm;
    CanNm_TxMsg_Info.Length = CANNM_FRAME_LENGTH;
    memset(CanNm_TxMsg_Info.data.b, 0, CanNm_TxMsg_Info.Length);

    if (rbdPlantData_GetCanLoading(0) == rbdPCID_keNone)
    {
        CanNm_CanFitted = (boolean)FALSE;
    }
    else
    {
        CanNm_CanFitted = (boolean)TRUE;
    }
    CanNm_PwrReqID = rbdPwrReq_GetID();

    CanNm_ImmediateTransmissionCount = 0u;
}

//----------------------------------------------------------------------------
/** \brief  Passive startup of the AUTOSAR CAN NM

\return Std_ReturnType E_OK if passive startup started else return E_NOT_OK

@rst
..  impl::
    :id: CanNm_PassiveStartUp
    :links_satisfies: CanNm_States_BusSleep_1,CanNm_States_PrepareBusSleep_1
@endrst
*///--------------------------------------------------------------------------
Std_ReturnType CanNm_PassiveStartUp ( void )
{
    Std_ReturnType retval = E_NOT_OK;
    CanNm_StateType nmState;
    CanNm_ModeType nmMode;

    if ((boolean)FALSE == CanNm_IsModuleInitialized())
    {
        Det_ReportError( DET_MODULEID_CANNM, 0, CANNM_APIID_PASSIVESTARTUP, CANNM_E_UNINIT );
    }
    else
    {
        if (E_OK != CanNm_GetState ( &nmState, &nmMode ))
        {
            Det_ReportError( DET_MODULEID_CANNM, 0, CANNM_APIID_PASSIVESTARTUP, CANNM_E_PARAM_INVALID );
        }
        else
        {
            if ((NM_MODE_BUS_SLEEP == nmMode) || (NM_MODE_PREPARE_BUS_SLEEP == nmMode))
            {
                CanNm_nextState = NM_STATE_REPEAT_MESSAGE;
                retval = E_OK;
            }
        }
    }

    return retval;
}

//----------------------------------------------------------------------------
/** \brief  Request the network, since ECU needs to communicate on the bus.

\return Std_ReturnType E_OK if Network Request successful else return E_NOT_OK

@rst
..  impl::
    :id: CanNm_NetworkRequest
    :links_satisfies: CanNm_States_ReadySleep_1,CanNm_States_RepeatMessage_1,
                      CanNm_States_PrepareBusSleep_1,CanNm_States_BusSleep_1
@endrst
*///--------------------------------------------------------------------------
Std_ReturnType CanNm_NetworkRequest ( void )
{
    Std_ReturnType retval = E_NOT_OK;
    CanNm_StateType nmState;
    CanNm_ModeType nmMode;

    RBDCANNM_PRINTF("%s %i Can Network Request\r\n", __func__, __LINE__);

    if ((boolean)FALSE == CanNm_IsModuleInitialized())
    {
        Det_ReportError( DET_MODULEID_CANNM, 0, CANNM_APIID_NETWORKREQUEST, CANNM_E_UNINIT );
    }
    else
    {
        if (E_OK != CanNm_GetState ( &nmState, &nmMode ))
        {
            Det_ReportError( DET_MODULEID_CANNM, 0, CANNM_APIID_DISABLECOMMUNICATION, CANNM_E_PARAM_INVALID );
        }
        else
        {
            CanNm_Network_State = NM_NETWORK_STATE_REQUESTED;
            retval = E_OK;
        }
    }


    return retval;
}

//----------------------------------------------------------------------------
/** \brief  Release the network, since ECU doesn't have to communicate
on the bus.

\return Std_ReturnType E_OK if Network Release successful else return E_NOT_OK

@rst
..  impl::
    :id: CanNm_NetworkRelease
    :links_satisfies: CanNm_States_Normal_1
@endrst
*///--------------------------------------------------------------------------
Std_ReturnType CanNm_NetworkRelease ( void )
{
    Std_ReturnType retval = E_NOT_OK;
    CanNm_StateType nmState;
    CanNm_ModeType nmMode;

    RBDCANNM_PRINTF("%s %i Can Network Release\r\n", __func__, __LINE__);

    if ((boolean)FALSE == CanNm_IsModuleInitialized())
    {
        Det_ReportError( DET_MODULEID_CANNM, 0, CANNM_APIID_NETWORKRELEASE, CANNM_E_UNINIT );
    }
    else
    {
        if (E_OK != CanNm_GetState ( &nmState, &nmMode ))
        {
            Det_ReportError( DET_MODULEID_CANNM, 0, CANNM_APIID_DISABLECOMMUNICATION, CANNM_E_PARAM_INVALID );
        }
        else
        {
            CanNm_Network_State = NM_NETWORK_STATE_RELEASED;
            retval = E_OK;
        }
    }

    return retval;
}

//----------------------------------------------------------------------------
//! \brief  Provide external function caller with the current state and mode of network management.
//!
//!
//! \param   nmStatePtr: Pointer where state of the network management shall be copied to
//! \param   nmModePtr: Pointer where the mode of the network management shall be copied to
//!
//! \retval  Std_ReturnType
//----------------------------------------------------------------------------
/*AXIVION Next Line MisraC2012-8.7 Routine can be declared static in primary file: this is called by other files in non-FBL build*/
Std_ReturnType CanNm_GetState ( CanNm_StateType* nmStatePtr, CanNm_ModeType* nmModePtr )
{
    Std_ReturnType retval = E_NOT_OK;

    /*AXIVION Next Line MisraC2012-14.3, MisraC2012-2.2 Redundant code: defensive coding*/
    if ((NULL == nmStatePtr) || (NULL == nmModePtr))
    {
        Det_ReportError( DET_MODULEID_CANNM, 0, CANNM_APIID_GETSTATE, CANNM_E_PARAM_INVALID );
    }
    else
    {
        retval = E_OK;

        switch (CanNm_Info_p->CanNm_State)
        {
        case NM_STATE_BUS_SLEEP:
            *nmModePtr = NM_MODE_BUS_SLEEP;
            break;
        case NM_STATE_PREPARE_BUS_SLEEP:
            *nmModePtr = NM_MODE_PREPARE_BUS_SLEEP;
            break;
        case NM_STATE_READY_SLEEP:
        case NM_STATE_NORMAL_OPERATION:
        case NM_STATE_REPEAT_MESSAGE:
            *nmModePtr = NM_MODE_NETWORK;
            break;
        default:
            *nmModePtr = NM_MODE_MAX_NUMBER;
            *nmStatePtr = NM_STATE_MAX_NUMBER;
            retval = E_NOT_OK;
            break;
        }
        *nmStatePtr = CanNm_Info_p->CanNm_State;
    }

    return retval;
}

//----------------------------------------------------------------------------
//! \brief  confirms the transmission of a PDU
//!
//! The lower layer communication interface module confirms the transmission of a PDU,
//! or the failure to transmit a PDU.
//!
//! \param   TxPduId: ID of the PDU that has been transmitted.
//----------------------------------------------------------------------------
void CanNm_TxConfirmation ( PduIdType TxPduId )
{
    RBD_UNUSED( TxPduId );

    if ((boolean)FALSE == CanNm_IsModuleInitialized())
    {
        Det_ReportError( DET_MODULEID_CANNM, 0, CANNM_APIID_TXCONFIRMATION, CANNM_E_UNINIT );
    }
    else
    {
        rbd_ElTime_rbTimerInit(&CanNm_Info.MsgTimeoutTimer);
    }
}

//----------------------------------------------------------------------------
/** \brief Indication of a received PDU from a lower layer communication
interface module.

\param[in] RxPduId PDU ID of the received message
\param[in] PduInfoPtr Payload information of the received message
(pointer to data and data length)

@rst
..  impl::
    :id: CanNm_RxIndication
    :links_satisfies: CanNm_rx_format_1,CanNm_States_Normal_1,
                      CanNm_States_ReadySleep_1
@endrst
*///--------------------------------------------------------------------------
void CanNm_RxIndication ( PduIdType RxPduId, const PduInfoType* PduInfoPtr )
{
    CanNm_Frame_st   CanNm_RxPduData;
    CanNm_StateType  nmState;
    CanNm_ModeType   nmMode;


    if ((boolean)FALSE == CanNm_IsModuleInitialized())
    {
        Det_ReportError( DET_MODULEID_CANNM, 0, CANNM_APIID_RXINDICATION, CANNM_E_UNINIT );
    }
    else
    {
        if (PduInfoPtr->SduLength < CANNM_FRAME_MIN_LENGTH)
        {
            Det_ReportError( DET_MODULEID_CANNM, 0, CANNM_APIID_RXINDICATION, CANNM_E_DATA_INVALID );
        }
        else
        {
            // Copying depends on the message length
            if( PduInfoPtr->SduLength >= sizeof(CanNm_RxPduData) )
            {
                // Copying the data is a slightly expensive way of avoiding MISRA warnings
                // We only copy the number of bytes we need, ignoring any extra bytes.
                memcpy( (uint8 *)&CanNm_RxPduData, PduInfoPtr->SduDataPtr, sizeof(CanNm_RxPduData));
            }
            else
            {
                // Clear the buffer before copying, to force undefined bytes to zero
                memset( (uint8 *)&CanNm_RxPduData, 0, sizeof(CanNm_RxPduData));
                /*AXIVION Next Line MisraC2012-21.18 : Size argument has been range checked above*/
                memcpy( (uint8 *)&CanNm_RxPduData, PduInfoPtr->SduDataPtr, PduInfoPtr->SduLength);
            }

            if (E_OK != CanNm_GetState ( &nmState, &nmMode ))
            {
                Det_ReportError( DET_MODULEID_CANNM, 0, CANNM_APIID_RXINDICATION, CANNM_E_PARAM_INVALID );
            }
            else
            {
                switch(nmState)
                {
                case NM_STATE_BUS_SLEEP:
                case NM_STATE_PREPARE_BUS_SLEEP:
                    CanNm_nextState = NM_STATE_REPEAT_MESSAGE;
                    break;
                case NM_STATE_READY_SLEEP:
                    rbd_ElTime_rbTimerStart(&CanNm_Info.NmTimeoutTimer, cp_BB_APS_CanNmTimeoutTime);
                    break;
                case NM_STATE_NORMAL_OPERATION:
                    if(cp_BB_CanNmMsgReducedTime[CanNm_Info_p->CanIdx] > 0u)
                    {
                        rbd_ElTime_rbTimerStart(&CanNm_Info.MsgCycleTimer, cp_BB_CanNmMsgReducedTime[CanNm_Info_p->CanIdx]);
                    }
                    break;
                case NM_STATE_REPEAT_MESSAGE:
                    break;
                default:
                    //nothing to do
                    break;
                }

                // [SWS_CanNm_00111] A received RepeatMessageRequest applies in ReadySleep state
                // [SWS_CanNm_00119] and in Normal state
                if(   (1u == CanNm_RxPduData.AutosarNMControl.RepeatMessageRequest)
                   && ((NM_STATE_NORMAL_OPERATION == nmState) || (NM_STATE_READY_SLEEP == nmState)) )
                {
                    CanNm_nextState = NM_STATE_REPEAT_MESSAGE;
                }
            }
        }
    }
}

//----------------------------------------------------------------------------
//! \brief  Main function of CanNm Module
//!
//! Main function of the CanNm which processes the algorithm describes in that document.
//----------------------------------------------------------------------------
void CanNm_MainFunction ( void )
{
    CanNm_StateType nmState;
    CanNm_ModeType nmMode;

    if ((CANSM_ONLINE != CanSm_GetState()) && ((boolean)TRUE == CanNm_CanFitted)) // Note: CanNm module is also used even when CAN is not fitted.
    {                                                                             //       Imc is always online and requires CanNm module to run.
        //RBDCANNM_PRINTF("%s %i CanSm is not Online\r\n", __func__, __LINE__);
    }
    else if ((boolean)FALSE == CanNm_IsModuleInitialized())
    {
        Det_ReportError( DET_MODULEID_CANNM, 0, CANNM_APIID_MAINFUNCTION, CANNM_E_UNINIT );
    }
    else
    {
        if (E_OK != CanNm_GetState ( &nmState, &nmMode ))
        {
            Det_ReportError( DET_MODULEID_CANNM, 0, CANNM_APIID_DISABLECOMMUNICATION, CANNM_E_PARAM_INVALID );
        }
        else
        {
            CanNm_MsgCycleTimerHandling(nmState);

            CanNm_NmTimeoutTimerHandling(nmState);

            CanNm_RepeatMessageTimeoutHandling(nmState);

            CanNm_NetworkRequestHandling(nmState, CanNm_Network_State);

            CanNm_MsgTimeoutTimerHandling();

            if (CanNm_nextState != nmState)// NM state changed
            {
                RBDCANNM_PRINTF("%s %i CanNm State changed, %i->%i\r\n", __func__, __LINE__, nmState, CanNm_nextState);

                if (NM_STATE_REPEAT_MESSAGE == nmState)
                {
                    CanNm_TxMsg_Info.data.f.AutosarNMControl.RepeatMessageRequest = 0u; //When exit REPEAT MESSAGE state clear Repeat Message Bit
                }

                CanNm_SetState(CanNm_nextState);
            }
        }
    }
}

//----------------------------------------------------------------------------
//! \brief  Check initialization state
//!
//! This service return the initialization state of the CanNm module
//!
//! \retval  boolean
//----------------------------------------------------------------------------
static boolean CanNm_IsModuleInitialized(void)
{
    return CanNm_Info_p->Initialized;
}

//----------------------------------------------------------------------------
/** \brief Set the state of the network management.

\param[in] NmState state of the network management shall be switched to

@rst
..  impl::
    :id: CanNm_SetState
    :links_satisfies: CanNm_rx_format_1,CanNm_States_Normal_1,
                      CanNm_States_ReadySleep_1,CanNm_States_BusSleep_1,
                      CanNm_States_PrepareBusSleep_1,CanNm_States_RepeatMessage_1,
                      CanNm_ImmediateMode_1
@endrst
*///----------------------------------------------------------------------------

static void CanNm_SetState(const CanNm_StateType NmState)
{
    CanNm_Info.CanNm_State = NmState;
    switch(NmState)
    {
    case NM_STATE_BUS_SLEEP:
        rbd_ElTime_rbTimerInit(&CanNm_Info.NmTimeoutTimer);
        rbd_ElTime_rbTimerInit(&CanNm_Info.MsgCycleTimer);
        /*AXIVION Next Line MisraC2012-2.2: Function is redundant for S32*/
        rbdPwrMgmtAnc_SleepIndication();
        rbdPwrReq_ReleaseService(rbdPwrReq_Service_CPU,CanNm_PwrReqID);
        break;
    case NM_STATE_PREPARE_BUS_SLEEP:
        rbd_ElTime_rbTimerStart(&CanNm_Info.NmTimeoutTimer, cp_BB_APS_CanNmWaitBusSleepTime);
        rbd_ElTime_rbTimerInit(&CanNm_Info.MsgCycleTimer);
        break;
    case NM_STATE_READY_SLEEP:
        rbd_ElTime_rbTimerStart(&CanNm_Info.NmTimeoutTimer, cp_BB_APS_CanNmTimeoutTime);
        rbd_ElTime_rbTimerInit(&CanNm_Info.MsgCycleTimer);
        break;
    case NM_STATE_NORMAL_OPERATION:
        rbd_ElTime_rbTimerInit(&CanNm_Info.NmTimeoutTimer);
        //Is the cyclic timer not yet started. Do no not restart timer if its already running.
        if (RBD_ELTIME_RBTIMER_STATE_RUNNING != rbd_ElTime_rbTimerUpdate(&CanNm_Info.MsgCycleTimer))
        {
            //yes, start the cyclic timer.
            rbd_ElTime_rbTimerStart(&CanNm_Info.MsgCycleTimer, cp_BB_APS_CanNmMsgCycleTime);
            (void)CanNm_TriggerTransmission();
        }
        rbdPwrReq_RequestService(rbdPwrReq_Service_CPU,CanNm_PwrReqID);
        break;
    case NM_STATE_REPEAT_MESSAGE:
        rbd_ElTime_rbTimerStart(&CanNm_Info.NmTimeoutTimer, cp_BB_APS_CanNmRepeatMessageTime);
        if (0u < CanNm_ImmediateTransmissionCount)
        {
            rbd_ElTime_rbTimerStart(&CanNm_Info.MsgCycleTimer, cp_BB_CanNmImmediateNmCycleTime);
            CanNm_ImmediateTransmissionCount--;
        }
        else
        {
            rbd_ElTime_rbTimerStart(&CanNm_Info.MsgCycleTimer, cp_BB_APS_CanNmMsgCycleTime);
        }
        (void)CanNm_TriggerTransmission();
        rbdPwrReq_RequestService(rbdPwrReq_Service_CPU,CanNm_PwrReqID);
        break;
    default:
        //do nothing
        break;
    }
}

//----------------------------------------------------------------------------
/** \brief Requests transmission of a PDU.

\return Std_ReturnType E_OK if transmit request successful else return E_NOT_OK

@rst
..  impl::
    :id: CanNm_TriggerTransmission
    :links_satisfies: CanNm_States_Normal_1, CanNm_States_RepeatMessage_1
@endrst
*///----------------------------------------------------------------------------
static Std_ReturnType CanNm_TriggerTransmission ( void )
{
    Std_ReturnType retval = E_NOT_OK;
    const PduIdType TxPduId = CanNm_TxMsg_Info.Id;
    PduInfoType PduInfo;

    if ((boolean)FALSE == CanNm_Info_p->TxEnabled)
    {
        RBDCANNM_PRINTF("%s %i Can NM Tx is disabled\r\n", __func__, __LINE__);
    }
    else
    {
        if( NM_NETWORK_STATE_REQUESTED == CanNm_Network_State)
        {
            CanNm_TxMsg_Info.data.f.AutosarNMControl.ActiveWakeup = 1u;
        }
        else
        {
            CanNm_TxMsg_Info.data.f.AutosarNMControl.ActiveWakeup = 0u;
        }

        PduInfo.SduDataPtr = CanNm_TxMsg_Info.data.b;
        PduInfo.SduLength = CanNm_TxMsg_Info.Length;

        if (E_OK == PduR_Transmit(TxPduId, &PduInfo))
        {
            rbd_ElTime_rbTimerStart(&CanNm_Info.MsgTimeoutTimer, CANNM_TXMESSAGE_TIMEOUT);//start the NM Tx timeout timer
            retval = E_OK;
        }
        else
        {
            Det_ReportError( DET_MODULEID_CANNM, 0, CANNM_APIID_TRIGGERTRANSMISSION, CANNM_E_RETURN_INVALID );
        }
    }

    return retval;
}

//----------------------------------------------------------------------------
/** \brief Check the NM message TX timeout timer.
*///----------------------------------------------------------------------------
static void CanNm_MsgTimeoutTimerHandling(void)
{
    if (RBD_ELTIME_RBTIMER_STATE_ELAPSED == rbd_ElTime_rbTimerUpdate(&CanNm_Info.MsgTimeoutTimer))
    {
        rbd_ElTime_rbTimerInit(&CanNm_Info.MsgTimeoutTimer);
        Det_ReportError( DET_MODULEID_CANNM, 0, CANNM_APIID_TIMEOUTHANDLING, CANNM_E_TIMEOUT );
    }
}

//----------------------------------------------------------------------------
/** \brief Check the NM message TX cycle timer.

\param[in] nmState state of the network management

@rst
..  impl::
    :id: CanNm_MsgCycleTimerHandling
    :links_satisfies: CanNm_States_Normal_1,CanNm_States_RepeatMessage_1,
                      CanNm_ImmediateMode_1
@endrst
*///----------------------------------------------------------------------------
static void CanNm_MsgCycleTimerHandling(const CanNm_StateType nmState)
{
    if (RBD_ELTIME_RBTIMER_STATE_ELAPSED == rbd_ElTime_rbTimerUpdate(&CanNm_Info.MsgCycleTimer))
    {
        (void)CanNm_TriggerTransmission();

        //restart the Msg Cycle timer
        switch (nmState)
        {
        case NM_STATE_REPEAT_MESSAGE:
            //Is Nm immediate transmission Counter is not yet 0.
            if (0u < CanNm_ImmediateTransmissionCount)
            {
                //Yes, decrement the counter.
                CanNm_ImmediateTransmissionCount--;
            }

            //All Nm immediate transmission transmitted?
            if(0u == CanNm_ImmediateTransmissionCount)
            {
                //Yes, Restart timer with cannm message cyclic time.
                rbd_ElTime_rbTimerStart(&CanNm_Info.MsgCycleTimer, cp_BB_APS_CanNmMsgCycleTime);
            }
            else
            {
                //No, Restart timer with immediate nm cyclic time.
                rbd_ElTime_rbTimerStart(&CanNm_Info.MsgCycleTimer, cp_BB_CanNmImmediateNmCycleTime);
            }

            break;
        case NM_STATE_NORMAL_OPERATION:
            rbd_ElTime_rbTimerStart(&CanNm_Info.MsgCycleTimer, cp_BB_APS_CanNmMsgCycleTime);
            break;
        default:
            //nothing to do
            break;
        }
    }
}

//----------------------------------------------------------------------------
/** \brief Check the NM state timeout timer.

\param[in] nmState state of the network management

@rst
..  impl::
    :id: CanNm_NmTimeoutTimerHandling
    :links_satisfies: CanNm_States_ReadySleep_1,CanNm_States_RepeatMessage_1,
                      CanNm_States_PrepareBusSleep_1
@endrst
*///----------------------------------------------------------------------------
static void CanNm_NmTimeoutTimerHandling(const CanNm_StateType nmState)
{
    if (RBD_ELTIME_RBTIMER_STATE_ELAPSED == rbd_ElTime_rbTimerUpdate(&CanNm_Info.NmTimeoutTimer))
    {
        switch (nmState)
        {
        case NM_STATE_PREPARE_BUS_SLEEP:
            CanNm_nextState = NM_STATE_BUS_SLEEP;
            break;
        case NM_STATE_READY_SLEEP:
            CanNm_nextState = NM_STATE_PREPARE_BUS_SLEEP;
            break;
        default:
            //nothing to do
            break;
        }
    }
}

//----------------------------------------------------------------------------
/** \brief  Handle the network request and release commands from upper module.

\param[in] nmState state of the network management
\param[in] networkState command

@rst
..  impl::
    :id: CanNm_NetworkRequestHandling
    :links_satisfies: CanNm_ImmediateMode_1,CanNm_States_ReadySleep_1,
                      CanNm_States_Normal_1,CanNm_States_RepeatMessage_1
@endrst
*///----------------------------------------------------------------------------
static void CanNm_NetworkRequestHandling(const CanNm_StateType nmState, const CanNm_AdditionalNetworkStateType networkState )
{
    switch (nmState)
    {
    case NM_STATE_BUS_SLEEP:
    case NM_STATE_PREPARE_BUS_SLEEP:
        if (NM_NETWORK_STATE_REQUESTED == networkState)
        {
            //Immediate Transmission is needed if network is requested from bus sleep / prepare bus sleep state.
            CanNm_ImmediateTransmissionCount = cp_BB_CanNmImmediateNmTransmissions;
            CanNm_nextState = NM_STATE_REPEAT_MESSAGE;
        }
        break;
    case NM_STATE_READY_SLEEP:
        if (NM_NETWORK_STATE_REQUESTED == networkState)
        {
            CanNm_nextState = NM_STATE_NORMAL_OPERATION;
        }
        break;
    case NM_STATE_NORMAL_OPERATION:
        if (NM_NETWORK_STATE_RELEASED == networkState)
        {
            CanNm_nextState = NM_STATE_READY_SLEEP;
        }
        break;
    default:
        //nothing to do
        break;
    }
}

//----------------------------------------------------------------------------
/** \brief Inform CANNm of the current CANIdx

\param[in] CanIdx current CANIdx

@rst
..  impl::
    :id: CanNm_SetCanIdx1
    :links_satisfies: CanNm_NoTransmit_CanIdx_F_1
@endrst
*///----------------------------------------------------------------------------
void CanNm_SetCanIdx(uint8 CanIdx)
{
    CanNm_TxMsg_Info.data.f.NodeId = CanNm_NodeId[CanIdx];

    if(CanIdx < 0xFu)
    {
        CanNm_Info.TxEnabled = TRUE;
    }
    else
    {
        CanNm_Info.TxEnabled = FALSE;
    }

    CanNm_Info.CanIdx = CanIdx;
}

//----------------------------------------------------------------------------
/** \brief Check if all required Immediate Nm Transmissions have been sent.

\param[in] nmState state of the network management

@rst
..  impl::
    :id: CanNm_RepeatMessageTimeoutHandling
    :links_satisfies: CanNm_ImmediateMode_1
@endrst
*///----------------------------------------------------------------------------
static void CanNm_RepeatMessageTimeoutHandling(const CanNm_StateType nmState)
{
    if (0u == CanNm_ImmediateTransmissionCount)
    {
        switch (nmState)
        {
        case NM_STATE_REPEAT_MESSAGE:
            if (RBD_ELTIME_RBTIMER_STATE_ELAPSED == rbd_ElTime_rbTimerUpdate(&CanNm_Info.NmTimeoutTimer))
            {
                if (NM_NETWORK_STATE_REQUESTED == CanNm_Network_State)
                {
                    CanNm_nextState = NM_STATE_NORMAL_OPERATION;
                }
                else
                {
                    CanNm_nextState = NM_STATE_READY_SLEEP;
                }
            }
            break;
        default:
            //nothing to do
            break;
        }
    }
}
//-------------------- End of File -------------------------------------------
