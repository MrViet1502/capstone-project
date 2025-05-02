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
#include "ImcDlSpi_Api.h"
#include "ImcIf_Api.h"
#include "ImcTl.h"
#include "rbdElTime_Api.h"


#ifndef RBD_PK_CPU_NCJ29D5
#include "rbdPlantData_Api.h"
#endif

#ifdef RBDIMC_WITH_RTE_APP
#include "Rte_rbdImc.h"
#include "CanIf.h"
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
// Global Variables
//============================================================================



const Imc_ChannelCfg_st Imc_ChannelCfg[IMC_CH_COUNT] =
{
#ifdef IMC_MCUBLE_EN
    [IMC_CH_MCUBLE] =
    {
        .ImcTl_SyncTimeout_ms  = 100,
        .ImcTl_ReadyTimeout_ms = 100,
        .ImcTl_DataTimeout_ms  = 300,
        .ImcDl_IsTxBufferFree  = ImcDlSpi_IsFree,
        .ImcDl_Tx              = ImcDlSpi_Tx
    },
#endif

#ifdef IMC_BLEUWB_EN
    [IMC_CH_BLEUWB] =
    {
        .ImcTl_SyncTimeout_ms  = 100,
        .ImcTl_ReadyTimeout_ms = 100,
        .ImcTl_DataTimeout_ms  = 300,
        .ImcDl_IsTxBufferFree  = ImcDlSpi_IsFree,
        .ImcDl_Tx              = ImcDlSpi_Tx
    }
#endif
};



rbdImc_ChannelEn_et Imc_ChannelEn[IMC_CH_COUNT];


//============================================================================
// Module Variables
//============================================================================



//============================================================================
// Function declarations.
//============================================================================





//----------------------------------------------------------------------------
//! \brief  Initialise the IMC stack
//!
//! This function decides which channels should be enabled, then initialises
//! the stack.
//----------------------------------------------------------------------------
void rbdImc_Init( void )
{
#ifdef IMC_MCUBLE_EN
#if IMC_MASTER==IMC_MCUBLE_EN
    //TODO link to plant data
    Imc_ChannelEn[IMC_CH_MCUBLE] = IMC_CH_EN_FULL;

#else
    if( rbdPCID_keNone==rbdPlantData_GetCanLoading(0) ) /*AXIVION Line CertC-STR34 : comparison is of same types*/
    {
        Imc_ChannelEn[IMC_CH_MCUBLE] = IMC_CH_EN_FULL;
    }
    else
    {
        Imc_ChannelEn[IMC_CH_MCUBLE] = IMC_CH_EN_OFF;
    }
#endif
#endif

#ifdef IMC_BLEUWB_EN
#if IMC_MASTER==IMC_BLEUWB_EN
    if( rbdPCID_keNone==rbdPlantData_GetUwbLoading() ) /*AXIVION Line CertC-STR34 : comparison is of same types*/
    {
        Imc_ChannelEn[IMC_CH_BLEUWB] = IMC_CH_EN_OFF;
    }
    else
    {
        Imc_ChannelEn[IMC_CH_BLEUWB] = IMC_CH_EN_FULL;
    }
#else
    Imc_ChannelEn[IMC_CH_BLEUWB] = IMC_CH_EN_FULL;
#endif
#endif

#if defined(IMC_MCUBLE_EN) || defined(IMC_BLEUWB_EN)
    /* Initalise call of the transport layer of the SPI */
    ImcDlSpi_Initialise();

    /* Initalise the transport layer */
    ImcTl_Initialise();

    /* Initalise the interface layer */
    ImcIf_Initialise();
#endif

#if defined( RBD_PK_MASTER_MCU )
    // Start the CAN channel
    (void)CanIf_SetControllerMode(CanIfConf_CanIfCtrlCfg_Can_ImcController, CANIF_CS_STARTED);
    (void)CanIf_SetPduMode(       CanIfConf_CanIfCtrlCfg_Can_ImcController, CANIF_SET_ONLINE);
#endif
}


//----------------------------------------------------------------------------
//! \brief  Fast cyclic call of the Imc stack.
//!
//! This function should be called 1ms of faster, although the exact timing
//! is not important.
//----------------------------------------------------------------------------
void rbdImc_CyclicFast( void )  /*AXIVION Line MisraC2012-8.7: Can be static in some compilations, not others */
{
    ImcDlSpi_Cyclic();
}



//----------------------------------------------------------------------------
//! \brief  Slow cyclic call of the Imc stack.
//!
//! Ideally, this should be be called about 3x slower than the fast cyclic task.
//!
//! Note: Functions are called starting with the lower layer, working to the
//! top. This enables buffers freed by a completed transmission/reception
//! to be reused for the next transmission.
//----------------------------------------------------------------------------
void rbdImc_CyclicMedium( void )
{
    ImcIf_MainFunction();
    ImcTl_Cyclic();

#if defined(RBD_PK_KEYFOB)
    // No TP here
#elif defined(RBD_PK_FBL) && (defined(RBD_PK_CPU_NCJ29D5) || defined(RBD_PK_CPU_S32))
    // or here
#else
    rbdImc_TpCyclic();
#endif

#if defined(RBD_PK_APP) && defined(RBD_PK_VEHICLE)
    rbdImc_PingCyclic(); /*AXIVION Line MisraC2012-2.2: could be configured as no function */
#endif
}



//----------------------------------------------------------------------------
//! \brief  API to Enable/Disable the specified channel
//!
//! Note: Functions are called starting with the lower layer, working to the
//! top. This enables buffers freed by a completed transmission/reception
//! to be reused for the next transmission.
//----------------------------------------------------------------------------
void rbdImc_SetChannelEn( rbdImc_Channel_et const ch, rbdImc_ChannelEn_et const enable )
{
    if( (IMC_CH_COUNT > ch ) && ( IMC_CH_EN_FULL >= enable) ) /*AXIVION Line MisraC2012-2.2,MisraC2012-14.3: range checking */ /*AXIVION Line CertC-STR34 : comparison is of same types*/
    {
        Imc_ChannelEn[ch] = enable;
    }
}


//----------------------------------------------------------------------------
//! \brief  Reset (clear) an Imc channel
//!
//! This clears all rx and tx buffers and state variables for this channel.
//!
//! \pre  Channel must not be running when this is being executed.
//----------------------------------------------------------------------------
void rbdImc_ResetChannel( rbdImc_Channel_et const ch )
{
    ImcIf_ResetChannel( ch );
    ImcTl_ResetChannel( ch );
    ImcDlSpi_ResetChannel( ch );
}


#ifdef RBDIMC_WITH_RTE_APP

# ifdef RTE_PTR2ARRAYBASETYPE_PASSING
FUNC(Std_ReturnType, rbdImc_CODE) rbdImcTx_Tx(rbdImc_RteMsgType MsgType, rbdImc_RteMsgLength Length, P2CONST(uint8, AUTOMATIC, RTE_RBDIMC_APPL_DATA) Msg) /* PRQA S 0786, 3449, 0624 */ /* MD_Rte_0786, MD_Rte_3449, MD_Rte_0624 */
# else
FUNC(Std_ReturnType, rbdImc_CODE) rbdImcTx_Tx(rbdImc_RteMsgType MsgType, rbdImc_RteMsgLength Length, P2CONST(uint8, AUTOMATIC, RTE_RBDIMC_APPL_DATA) Msg) /* PRQA S 0786, 3449, 0624 */ /* MD_Rte_0786, MD_Rte_3449, MD_Rte_0624 */
# endif
{
    Std_ReturnType result;

    if( TRUE == ImcIf_Tx( (ImcIf_MsgType_et)MsgType, (uint32)Length, (uint8 *)Msg )) /*AXIVION Line MisraC2012-10.5 MisraC2012-11.8: cast ok */
    {
        result = E_OK;
    }
    else
    {
        result = E_NOT_OK;
    }
    return result;
}


FUNC(void, rbdImc_CODE) rbdImc_CyclicFast2(void)
{
    rbdImc_CyclicFast();
}
#endif

//-------------------- End of File -------------------------------------------
