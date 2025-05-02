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
//! \brief  Interface between Imc and PduR
//----------------------------------------------------------------------------



//============================================================================
//  Includes
//============================================================================

#include <stdlib.h>     // for NULL
#include "Std_Types.h"
#include "rbdMacros.h"

#include "rbdImc.h"
#include "rbdImc_PduR_Types.h"
#include "rbdImc_Private.h"
#include "rbdImc_Cb.h"
#include "rbdImc_PduR.h"

#include "ImcIf_Api.h"

#include "Det.h"
#include "Det_rbModuleId.h"

#if defined(RBD_PK_MASTER_MCU)
#include "PduR_rbdImcCom.h"
#else
#include "PduR.h"
#endif

//============================================================================
// Local Macros
//============================================================================





//============================================================================
// Local Types
//============================================================================




//============================================================================
// Local Function Prototypes
//============================================================================




//============================================================================
// Local Variables
//============================================================================



//============================================================================
// Function declarations.
//============================================================================
/// \cond DO_NOT_DOCUMENT

#if !defined(RBD_PK_MASTER_MCU)
// Replacement for Rte-generated code
LOCAL_INLINE void PduR_rbdImcComRxIndication(PduIdType id, const PduInfoType* info)
{
    PduR_RxIndication(id, info);
}

LOCAL_INLINE void PduR_rbdImcComTxConfirmation(PduIdType id)
{
    PduR_TxConfirmation(id);
}
#endif

/// \endcond
//----------------------------------------------------------------------------
//! \brief  Simple (non-TP) message transmission
//!
//! This function has to translate from the ID used in PduR to the Imc message ID.
//!
//! \retval  E_OK      Transmission accepted
//! \retval  E_NOT_OK  Transmission rejected
//----------------------------------------------------------------------------
Std_ReturnType rbdImcCom_Transmit(PduIdType TxPduId, PduInfoType const * PduInfoPtr)
{
    Std_ReturnType result = E_NOT_OK;

    if( NULL == PduInfoPtr )    /*AXIVION Line MisraC2012-14.3 MisraC2012-2.2: defensive input parameter checking */
    {
        (void)Det_ReportError( DET_MODULEID_IMC, 0, RBDIMCPDUR_IF_TX, RBDIMCPDUR_ERR_NULL ); /*AXIVION Line MisraC2012-2.2: could be configured as no function */
    }
    else if((0u==rbdImc_PduRCfg.NumTxMsg) || (NULL_PTR==rbdImc_PduRCfg.TxCfg))
    {
        // Nothing to do
    }
    else
    {
        uint32 i;

        // Search for a matching PDU
        for( i=0; i<rbdImc_PduRCfg.NumTxMsg; i++ )
        {
            if(TxPduId == rbdImc_PduRCfg.TxCfg[i].UpperTxFromPduR)
            {
                if( TRUE == ImcIf_Tx(rbdImc_PduRCfg.TxCfg[i].Imc_PduId, PduInfoPtr->SduLength, PduInfoPtr->SduDataPtr))
                {
                    result = E_OK;
                }
            }
        }
    }

    return result;
}

//----------------------------------------------------------------------------
//! \brief  Simple (non-TP) message cancellation
//!
//! \retval  E_OK      Transmission accepted
//! \retval  E_NOT_OK  Transmission rejected
//----------------------------------------------------------------------------
Std_ReturnType rbdImcCom_CancelTransmit(PduIdType CanTxPduId)
{
    RBD_UNUSED(CanTxPduId);

    return E_NOT_OK;        // Cancellation not supported by Imc
}


//----------------------------------------------------------------------------
//! \brief  Interrupt-based transmit confirmation
//!
//! This function has to translate from the Imc message ID to the PduR transmit callback (confirmation) ID.
//----------------------------------------------------------------------------
void rbdImc_IfTxInterrupt( ImcIf_MsgType_et const MsgId )
{
#if defined(RBD_PK_APP) && (defined(RBD_PK_CPU_CC26xx) || defined(RBD_PK_CPU_S32))
    if((0u==rbdImc_PduRCfg.NumTxMsg) || (NULL_PTR==rbdImc_PduRCfg.TxCfg))
    {
        // Nothing to do
    }
    else
    {
        uint32 i;

        // Search for a matching PDU
        for( i=0; i<rbdImc_PduRCfg.NumTxMsg; i++ )
        {
            if(MsgId == rbdImc_PduRCfg.TxCfg[i].Imc_PduId)
            {
                PduR_TxInterrupt( rbdImc_PduRCfg.TxCfg[i].UpperTxToPduR );
            }
        }
    }
#endif
}


//----------------------------------------------------------------------------
//! \brief  Tx Confirmation
//!
//! This function translates the PduId from Imc to PduR numbers.
//----------------------------------------------------------------------------
void rbdImc_IfTxConfirmation( ImcIf_MsgType_et const MsgId )
{
    if((0u==rbdImc_PduRCfg.NumTxMsg) || (NULL_PTR==rbdImc_PduRCfg.TxCfg))
    {
        // Nothing to do
    }
    else
    {
        uint32 i;

        // Search for a matching PDU
        for( i=0; i<rbdImc_PduRCfg.NumTxMsg; i++ )
        {
            if(MsgId == rbdImc_PduRCfg.TxCfg[i].Imc_PduId)
            {
                PduR_rbdImcComTxConfirmation( rbdImc_PduRCfg.TxCfg[i].UpperTxToPduR );
            }
        }
    }
}

//----------------------------------------------------------------------------
//! \brief  Rx Indication
//!
//! This function translates the PduId from Imc to PduR numbers.
//----------------------------------------------------------------------------
void rbdImc_IfRxIndication(
        ImcIf_MsgType_et const   MsgId,
        rbdImc_Length_t  const   Length,
        uint8            const * Msg,
        rbd_ElTime_Value_t const timestamp
        )
{
    if((0u==rbdImc_PduRCfg.NumRxMsg) || (NULL_PTR==rbdImc_PduRCfg.RxCfg))
    {
        // Nothing to do
    }
    else
    {
        uint32 i;

        // Search for a matching PDU
        for( i=0; i<rbdImc_PduRCfg.NumRxMsg; i++ )
        {
            if(MsgId == rbdImc_PduRCfg.RxCfg[i].Imc_PduId)
            {
                PduInfoType Info;
                Info.SduLength = Length;
                Info.SduDataPtr = (SduDataPtrType)Msg; /*AXIVION Line MisraC2012-11.8: cast is needed to meet function parameter requirement*/
                Info.SduTimeStamp = (SduTimeStampType)(RBD_ELTIME_MHZ_TICK_TO_US(rbd_ElTime_GetValueSinceStart(RBD_ELTIME_CHANNEL_MHZ, timestamp)) * 1000u);

                PduR_rbdImcComRxIndication( rbdImc_PduRCfg.RxCfg[i].UpperRxToPduR, &Info );
            }
        }
    }
}



//-------------------- End of File -------------------------------------------
