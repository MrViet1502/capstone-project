//----------------------------------------------------------------------------
// CONFIDENTIAL
//
// COPYRIGHT Robert Bosch (Australia) Pty. Ltd. 2019.
// All rights reserved, also regarding any disposal, exploitation,
// reproduction, editing, distribution, as well as in the event of
// applications for industrial property rights.
//----------------------------------------------------------------------------

#ifndef PDUR_H
#define PDUR_H

//============================================================================
// Include Files
//============================================================================
#include "PduR_Types.h"
#include "Det.h"
#include "PduR_Cfg.h"

//============================================================================
// Public Macros
//============================================================================



//============================================================================
// Public Macros
//============================================================================

typedef struct
{
    PduR_RouteTblTx_st                      RouteTblTx[PDUR_PDU_TX_MAX];
    PduR_RouteTblRx_st                      RouteTblRx[PDUR_PDU_RX_MAX];

    PduR_TpRouteTblTx_st                    TpRouteTblTx[PDUR_PDU_TP_TX_MAX];
    PduR_TpRouteTblRx_st                    TpRouteTblRx[PDUR_PDU_TP_RX_MAX];
    // Remove dependency with CanTSyn by adding function pointers in the config
    PduR_TxInterruptFuncType                TxInterrupt;
    PduR_SetTxConfirmationLatencyFuncType   SetTxConfirmationLatency;
} PduR_ConfigType;

//============================================================================
// Prototypes
//============================================================================



void PduR_Init( PduR_ConfigType const * const config );

#ifdef PDUR_ROUTING_ENABLE
Std_ReturnType PduR_Transmit(PduIdType id, const PduInfoType* info);
Std_ReturnType PduR_CancelTransmit(PduIdType id);
void PduR_TxConfirmation(PduIdType id);
void PduR_RxIndication(PduIdType id, const PduInfoType* info);


Std_ReturnType    PduR_TpTransmit(PduIdType id, const PduInfoType* info);
Std_ReturnType    PduR_TpCancelTransmit(PduIdType id);
BufReq_ReturnType PduR_TpCopyTxData(PduIdType id, PduInfoType* info, RetryInfoType* retry, PduLengthType* avail);
void PduR_TpTxConfirmation(PduIdType id, Std_ReturnType result);

BufReq_ReturnType PduR_TpStartOfReception(PduIdType id, PduInfoType* info, PduLengthType length, PduLengthType* size);
BufReq_ReturnType PduR_TpCopyRxData(PduIdType id, PduInfoType* info, PduLengthType* size);
void PduR_TpRxIndication(PduIdType id, Std_ReturnType result);

#if defined(RBD_PK_APP) && defined(RBD_PK_CPU_CC26xx)
/*AXIVION Next Line MisraC2012-8.6: function is valid in some build variant */
void PduR_SetTxConfirmationLatency(PduIdType id, uint32 latency);
#endif

#if defined(RBD_PK_APP) && (defined(RBD_PK_CPU_CC26xx) || defined(RBD_PK_CPU_S32))
/*AXIVION Next Line MisraC2012-8.6: function is valid in some build variant */
void PduR_TxInterrupt(PduIdType id);
#endif

#endif // PDUR_ROUTING_ENABLE



#endif // PDUR_H
