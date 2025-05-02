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
//! \brief  Configurtion Header File of the Imc Module
//!
//! This file is private and must not be directly included by any other
//! module.
//----------------------------------------------------------------------------

#ifndef RBD_IMC_H
#define RBD_IMC_H

#include "rbd_Pk.h"
#include "ComStack_Types.h"

//============================================================================
// Public Macros
//============================================================================

// Macros to say if a channel is enabled, and which end is active
#define IMC_MASTER    1     //!< Channel exists, this is the master end
#define IMC_SLAVE     2     //!< Channel exists, this is the slave end


// The following macros define both (1) which channels are used, and (2) the direction
//
// If a channel exists, the corresponding macro is defined.
// If a channel does not exist, the macro is not defined.
//    IMC_MCUBLE_EN     -- This is from MasterMcu to CC2642
//    IMC_BLEUWB_EN     -- This is from CC22642/S32 to UWB chip
//
// If a channel exists, the define also gives the direction or role in the current
// compilation: MASTER or SLAVE
//
// The macros can be used like:
//     #ifdef IMC_MCUBLE_EN
// or
//     #if defined(IMC_MCUBLE_EN) && (IMC_MASTER==IMC_MCUBLE_EN)
//
#if defined(RBD_PK_CPU_SPC58ECx)
    #define IMC_MCUBLE_EN  IMC_MASTER
#elif defined(RBD_PK_CPU_CC26xx)
    #define IMC_MCUBLE_EN  IMC_SLAVE
    #define IMC_BLEUWB_EN  IMC_MASTER
#elif defined(RBD_PK_CPU_S32)
    #define IMC_BLEUWB_EN  IMC_MASTER
#elif defined(RBD_PK_CPU_NCJ29D5)
    #define IMC_BLEUWB_EN  IMC_SLAVE
#endif

//! \brief  Maximum length of data which can be transferred
//!
//! This is defined at the top level to avoid an impossible chain of \#includes.
//!
//! 255 = SPI transfer to Ranger4 is 256 bytes, but first byte unusable.
//! 5 = ImcTl overhead
//! 2 = ImcIf overhead
#define IMC_CFG_MAX_DL_PAYLOAD_LEN  255u
#define IMC_CFG_MAX_TL_PAYLOAD_LEN  (IMC_CFG_MAX_DL_PAYLOAD_LEN-5u)
#define IMC_CFG_MAX_IF_PAYLOAD_LEN  (IMC_CFG_MAX_TL_PAYLOAD_LEN-1u)


//============================================================================
// Public Types
//============================================================================


//----------------------------------------------------------------------------
//! \brief  Imc length type
//!
//! This type is used for lengths throughout the Imc stack.
//----------------------------------------------------------------------------
typedef uint16 rbdImc_Length_t;



//----------------------------------------------------------------------------
//! \brief  Imc channels
//!
//! This enumeration specifies the channels available for communication
//----------------------------------------------------------------------------
typedef enum
{
#ifdef IMC_MCUBLE_EN
    IMC_CH_MCUBLE,  //!< Master Mcu-Ble communication
#endif

#ifdef IMC_BLEUWB_EN
    IMC_CH_BLEUWB,  //!< Master Ble-Ranger 4 communication
#endif

    IMC_CH_COUNT
}rbdImc_Channel_et;


//----------------------------------------------------------------------------
//! \brief  Imc Channel Enable Level
//!
//! This enumeration specifies the level of activity the IMC channel can have
//----------------------------------------------------------------------------
typedef enum
{
    IMC_CH_EN_OFF,          //!< IMC Channel is Disabled for all layers
    IMC_CH_EN_DL_ONLY,      //!< IMC Channel is Enabled but on the DL layer only, Disabled in upper layers ( used in RCI mode)
    IMC_CH_EN_FULL,         //!< IMC Channel is Enabled for all layers
}rbdImc_ChannelEn_et;

//----------------------------------------------------------------------------
//! \brief  Is the data-link layer ready to transmit
//!
//! This is the function type.
//----------------------------------------------------------------------------
typedef boolean (*ImcDl_IsTxBufferFree_fpt)(const rbdImc_Channel_et ch);

//----------------------------------------------------------------------------
//! \brief  Transmit a message on the datalink layer
//!
//! This is the function type.
//----------------------------------------------------------------------------
typedef boolean (*ImcDl_Tx_fpt)(const rbdImc_Channel_et Ch, const rbdImc_Length_t Length, const uint8 *const Msg,
        uint8 msgId);

//----------------------------------------------------------------------------
//! \brief  Imc channel configuration
//!
//! This structure specifies the parameters for each channel
//----------------------------------------------------------------------------
typedef struct
{
    uint32                   ImcTl_SyncTimeout_ms;  //!< Timeout for a SYNC.
    uint32                   ImcTl_ReadyTimeout_ms; //!< Timeout for a SYNC.
    uint32                   ImcTl_DataTimeout_ms;  //!< Timeout for a READY.

    ImcDl_IsTxBufferFree_fpt ImcDl_IsTxBufferFree;  //!< Is the datalink layer free?
    ImcDl_Tx_fpt             ImcDl_Tx;              //!< Transmit using the datalink layer

} Imc_ChannelCfg_st;



//============================================================================
// Public Variables
//============================================================================

#ifdef CEEDLING_TEST
// No const, so we can change timings in the test
extern       Imc_ChannelCfg_st Imc_ChannelCfg[IMC_CH_COUNT];
#else
extern const Imc_ChannelCfg_st Imc_ChannelCfg[IMC_CH_COUNT];
#endif
extern rbdImc_ChannelEn_et Imc_ChannelEn[IMC_CH_COUNT];


//============================================================================
// Public Functions
//============================================================================
/*AXIVION Disable MisraC2012-8.6: no problem to define prototype always */
#ifndef RBD_PK_MASTER_MCU
// these functions have prototype in Autosar\Appl\GenData\Components\Rte_rbdImc.h
void rbdImc_Init( void );
void rbdImc_CyclicFast( void );
void rbdImc_CyclicMedium( void );
#endif
void rbdImc_SetChannelEn(const rbdImc_Channel_et ch, const rbdImc_ChannelEn_et enable);
void rbdImc_ResetChannel(const rbdImc_Channel_et ch);

uint8 rbdImc_PingStart(const uint8 *const In_ReqData);
uint8 rbdImc_PingResult( uint8 * const buf, uint16 * const len );

#endif // Include guard

//-------------------- End of File -------------------------------------------
