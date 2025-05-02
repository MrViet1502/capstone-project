//----------------------------------------------------------------------------
// CONFIDENTIAL
//
// COPYRIGHT Robert Bosch (Australia) Pty. Ltd. 2019.
// All rights reserved, also regarding any disposal, exploitation,
// reproduction, editing, distribution, as well as in the event of
// applications for industrial property rights.
//----------------------------------------------------------------------------

//============================================================================
// Include Files
//============================================================================
#include "PduR.h"
#include "PduR_Private.h"

//============================================================================
// Module variables
//============================================================================
PduR_ConfigType const * PduR_Config = NULL_PTR;

// Remove dependency with CanTSyn by introducing callback pointers that CanTSyn will register to
#if defined(RBD_PK_APP)
#if defined(RBD_PK_CPU_CC26xx)
PduR_SetTxConfirmationLatencyFuncType pPduR_SetTxConfirmationLatency_cb;
#endif
#if defined(RBD_PK_CPU_CC26xx) || defined(RBD_PK_CPU_S32)
PduR_TxInterruptFuncType pPduR_TxInterrupt_cb;
#endif
#endif

//============================================================================
// Function Definitions.
//============================================================================


#if defined(DOXYGEN) && !defined(PDUR_ROUTING_ENABLE)
#define PDUR_ROUTING_ENABLE
#endif

#ifdef PDUR_ROUTING_ENABLE

//-----------------------------------------------------------------------------------------------------------------
//! \brief  Initialization function.
//-----------------------------------------------------------------------------------------------------------------
void PduR_Init( PduR_ConfigType const * const config )
{
    PduR_Config = config;
}

//-----------------------------------------------------------------------------------------------------------------
//! \brief  This function is called by the upper layer to transmit a single frame.
//!
//! \param  id         Identification of the PDU.
//! \param  info       Length of and pointer to the PDU data.
//!
//! \retval  E_OK           Transmit request has been accepted.
//!          E_NOT_OK       Transmit request has not been accepted.
//-----------------------------------------------------------------------------------------------------------------
Std_ReturnType PduR_Transmit(PduIdType id, const PduInfoType* info)
{
    Std_ReturnType result = E_NOT_OK;

    /*AXIVION Next Line MisraC2012-2.2 MisraC2012-14.3: defensive checking */
    if ((id < (PduIdType)PDUR_PDU_TX_MAX) && (NULL_PTR!=PduR_Config) && (PduR_Config->RouteTblTx[id].LoTransmit != NULL_PTR))
    {
        result = PduR_Config->RouteTblTx[id].LoTransmit(PduR_Config->RouteTblTx[id].LoTxPduId, info);
    }

    return result;
}

//-----------------------------------------------------------------------------------------------------------------
//! \brief  This function is called by the upper layer to cancel a single frame transmission
//!
//! \param  id         Identification of the PDU.
//!
//! \retval  E_OK           Transmit request has been accepted.
//!          E_NOT_OK       Transmit request has not been accepted.
//-----------------------------------------------------------------------------------------------------------------
Std_ReturnType PduR_CancelTransmit(PduIdType id)
{
    Std_ReturnType result = E_NOT_OK;

    /*AXIVION Next Line MisraC2012-2.2 MisraC2012-14.3: defensive checking */
    if ((id < (PduIdType)PDUR_PDU_TX_MAX) && (NULL_PTR!=PduR_Config) && (PduR_Config->RouteTblTx[id].LoCancelTransmit != NULL_PTR))
    {
        result = PduR_Config->RouteTblTx[id].LoCancelTransmit( PduR_Config->RouteTblTx[id].LoTxPduId );
    }

    return result;
}

//-----------------------------------------------------------------------------------------------------------------
//! \brief  This function is called by the lower layer to confirm a single frame transmition.
//!
//! \param  id         Identification of the PDU.
//-----------------------------------------------------------------------------------------------------------------
void PduR_TxConfirmation(PduIdType id)
{
    /*AXIVION Next Line MisraC2012-2.2 MisraC2012-14.3: defensive checking */
    if ((id < (PduIdType)PDUR_PDU_TX_MAX) && (NULL_PTR!=PduR_Config) && (PduR_Config->RouteTblTx[id].UpTxConfirmation != NULL_PTR))
    {
        (*PduR_Config->RouteTblTx[id].UpTxConfirmation)( PduR_Config->RouteTblTx[id].UpTxPduId ); /*AXIVION Line MisraC2012Directive-4.1: UpTxConfirmation is checked before */
    }
}

//-----------------------------------------------------------------------------------------------------------------
//! \brief  This function is called by the lower layer to indicate a single frame reception.
//!
//! \param  id         Identification of the PDU.
//! \param  info       Length of and pointer to the PDU data.
//-----------------------------------------------------------------------------------------------------------------
void PduR_RxIndication(PduIdType id, const PduInfoType* info)
{
    /*AXIVION Next Line MisraC2012-2.2 MisraC2012-14.3: defensive checking */
    if ((id < (PduIdType)PDUR_PDU_RX_MAX) && (NULL_PTR!=PduR_Config) && (PduR_Config->RouteTblRx[id].UpRxIndication != NULL_PTR))
    {
        (*PduR_Config->RouteTblRx[id].UpRxIndication)( PduR_Config->RouteTblRx[id].UpRxPduId, info );
    }
}

#if defined(RBD_PK_APP) && defined(RBD_PK_CPU_CC26xx)
void PduR_SetTxConfirmationLatency(PduIdType id, uint32 latency)
{
    /*AXIVION Next Line MisraC2012-2.2 MisraC2012-14.3: defensive checking */
    if ((id < (PduIdType)PDUR_PDU_TX_MAX) && (NULL_PTR!=PduR_Config))
    {

        if ( PduR_Config->SetTxConfirmationLatency != NULL_PTR)
        {
            (*PduR_Config->SetTxConfirmationLatency)(PduR_Config->RouteTblTx[id].UpTxPduId, latency);
        }
    }
}

#endif

#if defined(RBD_PK_APP) && (defined(RBD_PK_CPU_CC26xx) || defined(RBD_PK_CPU_S32))
void PduR_TxInterrupt(PduIdType id)
{
    /*AXIVION Next Line MisraC2012-2.2 MisraC2012-14.3: defensive checking */
    if ((id < (PduIdType)PDUR_PDU_TX_MAX) && (NULL_PTR!=PduR_Config))
    {
        if ( PduR_Config->TxInterrupt != NULL_PTR)
        {
            (*PduR_Config->TxInterrupt)(PduR_Config->RouteTblTx[id].UpTxPduId);
        }
    }
}

#endif
#endif // PDUR_ROUTING_ENABLE
