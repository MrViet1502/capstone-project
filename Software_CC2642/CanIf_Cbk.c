/*##############################################################################
#                                ROBERT BOSCH                                  #
#                            Melbourne,  AUSTRALIA                             #
###########################################################################*//**
@file
@brief CAN Interface Callbacks.

 These are the callback functions used by the CAN driver to notify of events.


*//*############################################################################
# CONFIDENTIAL                                                                 #
#                                                                              #
# COPYRIGHT RESERVED, 2023 Robert Bosch (Australia) Pty Ltd. All rights        #
# reserved.  The reproduction, distribution and utilisation of this document   #
# as well as the communication of its contents to others without explicit      #
# authorisation is prohibited.  Offenders will be held liable for the payment  #
# of damages.  All rights reserved in the event of the grant of a patent,      #
# utility model or design.                                                     #
#                                                                              #
##############################################################################*/

/*==============================================================================

	INCLUDES

==============================================================================*/
#include "CanIf.h"
#include "CanIf_Cbk.h"
#include "CanIf_Private.h"
#include "Can_id.h"
#include "Det.h"
#include "CanTp_Cbk.h"
#include "PduR.h"


//! @cond
/*==============================================================================

	DEFINES

==============================================================================*/


/*==============================================================================

	TYPES

==============================================================================*/


/*==============================================================================

	NATIONAL FUNCTION DECLARATIONS

==============================================================================*/
static Std_ReturnType CanIf_GetPduId( Can_IdType CanId, PduIdType * const PduIdPtr );


/*==============================================================================

	GLOBAL VARIABLE DEFINITIONS

==============================================================================*/


/*==============================================================================

	NATIONAL VARIABLE DEFINITIONS

==============================================================================*/


//! @endcond
/*==============================================================================

	GLOBAL FUNCTION DEFINITIONS

==============================================================================*/

/*-------------------------------------------------------------------------*//**
@brief Notify of CAN frame transmission.
 This service confirms a previously successfully processed transmission
 of a CAN TxPDU.

@param[in] CanTxPduId    L-PDU handle of CAN L-PDU successfully transmitted.
This ID specifies the corresponding CAN L-PDU ID
and implicitly the CAN Driver instance as well as the
corresponding CAN controller device.

@returns Nil.

@rst
..  impl::
    :id: CanIf_TxConfirmation
    :links_satisfies: CanIf_TxConfirmation_1,
@endrst
*//*--------------------------------------------------------------------------*/
void CanIf_TxConfirmation( PduIdType CanTxPduId )
{
#if (CANIF_DEV_ERROR_DETECT == STD_ON)
    if ((PduIdType)CanIf_Config.CanIfPduMax <= CanTxPduId)
    {
        Det_ReportError( DET_MODULEID_CANIF, 0, CANIF_API_TXCONFIRMATION, CANIF_ERR_PARAM_PDUID );
    }
    else
#endif
    {
        const PduIdType TxPduId = CanIf_Config.CanIfPduHandleCfg[CanTxPduId].UpPduId;

        // Only execute the callback if there is actually a pending Tx Confirmation.
        // This is because we fake some Tx Confirmations for TP messages in CanIf_RxIndication()
        // See: ALM ticket 410391
        if( (boolean)TRUE==CanIf_Config.CanIfPendingTxConfirm[CanTxPduId] )
        {
            // Clear the pending flag - we only want to send one confirmation.
            CanIf_Config.CanIfPendingTxConfirm[CanTxPduId] = (boolean)FALSE;

            switch (CanIf_Config.CanIfPduUserCallbackULCfg[CanTxPduId])
            {
                case CANIF_CAN_TP:
                {
                    CanTp_TxConfirmation( TxPduId );
                    break;
                }
//S32 doesn't send TIMESYNC to the master (it doesn't have a RTC to keep time in standby)
#if (RBD_PK_BLE_TIMESYNC != 0) && defined(RBD_PK_CPU_CC26xx)

                case CANIF_CAN_TSYN:
                    //do we have a timestamp cb?
                    if( CanIf_Config.CanIfTimestampFrequency != 0u )
                    {
                        PduR_SetTxConfirmationLatency(TxPduId, (((uint32)CanIf_Config.CanTrcv_GetCurrentTimeStampCounterValue(0) - (uint32)CanIf_Config.CanTrcv_GetCurrentTxTimeStamp(0)) & 0x0000FFFFu) * (1000000000u / CanIf_Config.CanIfTimestampFrequency));
                    }
                    PduR_TxConfirmation( TxPduId );
                    break;
#endif
                case CANIF_PDUR:
                {
                    PduR_TxConfirmation( TxPduId );
                    break;
                }
                case CANIF_CDD:
                {
                    CanIf_PduUserTxConfirmationNameType UserTxConfirmationFn = CanIf_Config.CanIfPduUserCallbackNameCfg[CanTxPduId].UserTxConfirmationFn;

                    if (NULL_PTR != UserTxConfirmationFn)
                    {
                        UserTxConfirmationFn( TxPduId );
                    }
                    break;
                }
                default:
                    //do nothing
                    break;
            }
        }
    }
}


/*-------------------------------------------------------------------------*//**
@brief Indicate CAN PDU Reception.
 This service indicates a successful reception of a received CAN Rx LPDU
 to the CanIf after passing all filters and validation checks.

@param[in] Mailbox    Identifies the HRH and its corresponding CAN Controller
@param[in] PduInfoPtr Pointer to the received L-PDU

@returns Nil.

@rst
..  impl::
    :id: CanIf_RxIndication
    :links_satisfies: CanIf_RxIndication_1
@endrst
*//*--------------------------------------------------------------------------*/
void CanIf_RxIndication( const Can_HwType *Mailbox, PduInfoType *PduInfoPtr )
{
#if (CANIF_DEV_ERROR_DETECT == STD_ON)
    if ((NULL_PTR == Mailbox) || (NULL_PTR == PduInfoPtr))
    {
        Det_ReportError( DET_MODULEID_CANIF, 0, CANIF_API_RXINDICATION, CANIF_ERR_PARAM_POINTER );
    }
    else
#endif
    {
        VAR(PduIdType, AUTOMATIC) PduId;

        if (E_OK == CanIf_GetPduId( Mailbox->CanId, &PduId ))
        {
            const PduIdType RxPduId = CanIf_Config.CanIfPduHandleCfg[PduId].UpPduId;

            if( CanIf_Config.CanIfPduUserCallbackNameCfg[PduId].FakeTxConfirmPduId < (PduIdType)CanIf_Config.CanIfPduMax )
            {
                // Fake a Tx Confirmation.
                // Note: The Tx Confirm function won't do anything unless a confirmation is actually pending.
                // This is to handle a race condition between TxConfirmation and RxIndication of the next message.
                // See ALM open point 410391
                CanIf_TxConfirmation( CanIf_Config.CanIfPduUserCallbackNameCfg[PduId].FakeTxConfirmPduId );
            }


            switch (CanIf_Config.CanIfPduUserCallbackULCfg[PduId])
            {
                case CANIF_CAN_TP:
                {
                    CanTp_RxIndication( RxPduId, PduInfoPtr );
                    break;
                }
#if (RBD_PK_BLE_TIMESYNC != 0)
                case CANIF_CAN_TSYN:
                    //do we have a timestamp cb?
                    if( CanIf_Config.CanIfTimestampFrequency != 0u )
                    {
                        PduInfoPtr->SduTimeStamp = ((CanIf_Config.CanTrcv_GetCurrentTimeStampCounterValue(0) - PduInfoPtr->SduTimeStamp) & 0xFFFFu) * (1000000000u / CanIf_Config.CanIfTimestampFrequency); // now SduTimeStamp contains latency in nanosecond rather than timestamp
                    }
                    PduR_RxIndication( RxPduId, PduInfoPtr );
                    break;
#endif
                case CANIF_PDUR:
                {
                    PduR_RxIndication( RxPduId, PduInfoPtr );
                    break;
                }
                case CANIF_CDD:
                {
                    CanIf_PduUserRxIndicationNameType UserRxIndicationFn = CanIf_Config.CanIfPduUserCallbackNameCfg[PduId].UserRxIndicationFn;

                    if (NULL_PTR != UserRxIndicationFn)
                    {
                        UserRxIndicationFn( RxPduId, PduInfoPtr );
                    }
                    break;
                }
                default:
                    //do nothing
                    break;
            }
        }
    }
}


//! @cond
/*==============================================================================

	NATIONAL FUNCTIONS

==============================================================================*/
/*------------------------------------------------------------------------------
DESCRIPTION:
 This function gets the PDU Id for a given CAN Id.
PARAMETERS:
  -> CAN Identifier
  -> The pointer to the container of PDU Id
RETURNS:
 <-  E_OK     CAN Id has been found.
     E_NOT_OK CAN Id has not been found.
------------------------------------------------------------------------------*/
static Std_ReturnType CanIf_GetPduId( Can_IdType CanId, PduIdType * const PduIdPtr )
{
    Std_ReturnType RetValue = E_NOT_OK;

#if (CANIF_DEV_ERROR_DETECT == STD_ON)
    if (NULL_PTR == PduIdPtr)
    {
        Det_ReportError( DET_MODULEID_CANIF, 0, CANIF_API_GET_PDUID, CANIF_ERR_PARAM_POINTER );
    }
    else
#endif
    {
        PduIdType PduId;

        for (PduId = 0; PduId < (PduIdType)CanIf_Config.CanIfPduMax; PduId++)
        {
            // Ignore FD mask during comparison
            if (0U == ((CanId ^ CanIf_Config.CanIfPduCanIdMappingCfg[PduId]) & (uint32)(~(CAN_FD_FRAME_MASK)))) // AXIVION Line MisraC2012-10.1, MisraC2012-12.4: checked
            {
                *PduIdPtr = PduId;
                RetValue = E_OK;
                break;
            }
        }
    }

    return RetValue;
}


//! @endcond


