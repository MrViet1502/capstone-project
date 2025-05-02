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
//! \brief  AUTOSAR CAN Interface Type Definitions
//----------------------------------------------------------------------------


#ifndef CANIF_TYPES_H
#define CANIF_TYPES_H


//============================================================================
// Include Files
//============================================================================
#include "ComStack_Types.h"
#include "Can_GeneralTypes.h"


//============================================================================
// Public Macros
//============================================================================
#define CANIF_VENDOR_ID                     (6u) /*AXIVION Line MisraC2012-2.5 :Can be used for future purpose. */
#define CANIF_MODULE_ID                     (60u) /*AXIVION Line MisraC2012-2.5 :Can be used for future purpose. */
                                                 
#define CANIF_AR_RELEASE_MAJOR_VERSION      (4u)  /*AXIVION Line MisraC2012-2.5 :Can be used for future purpose. */
#define CANIF_AR_RELEASE_MINOR_VERSION      (2u)  /*AXIVION Line MisraC2012-2.5 :Can be used for future purpose. */
#define CANIF_AR_RELEASE_REVISION_VERSION   (1u)  /*AXIVION Line MisraC2012-2.5 :Can be used for future purpose. */
                                                 
#define CANIF_SW_MAJOR_VERSION_TYPES_H      (1u)  /*AXIVION Line MisraC2012-2.5 :Can be used for future purpose. */
#define CANIF_SW_MINOR_VERSION_TYPES_H      (0u)  /*AXIVION Line MisraC2012-2.5 :Can be used for future purpose. */
#define CANIF_SW_PATCH_VERSION_TYPES_H      (0u)  /*AXIVION Line MisraC2012-2.5 :Can be used for future purpose. */


//============================================================================
// Public Types
//============================================================================
/* [SWS_CANIF_00136] */
typedef enum
{
    CANIF_CS_UNINIT,
    CANIF_CS_SLEEP,
    CANIF_CS_STARTED,
    CANIF_CS_STOPPED,
    CANIF_CS_WAKEUP
}CanIf_ControllerModeType;


/* [SWS_CANIF_00xxx] */
typedef enum
{
    CANIF_PDU_MODE
}CanIf_PduModeType;


typedef struct
{
    Can_HwHandleType    LoHwHandle;     //!< CAN hardware object handle
    PduIdType           UpPduId;        //!< Upper-layer PDU Id
} CanIf_PduHandleType;


typedef enum
{
    CANIF_CAN_TP,		//!< CAN TP
    CANIF_CAN_TSYN,		//!< CAN TSYN
    CANIF_PDUR,         //!< Route via PduR
    CANIF_CDD			//!< Complex Driver
} CanIf_PduUserCallbackULType;


typedef struct
{
    //! Total number of PDU ids
    uint32 CanIfPduMax;

    //! Total number of controllers (usually 1)
    uint32 CanIfNumController;

    //! Total number of channels (usually 1)
    uint32 CanIfNumChannels;

    //! CAN Controller Id
    uint8 CanIfCtrlId;

    //! CAN Transceiver Id
    uint8 CanIfTrcvId;

    //! CAN transceiver timestamp clock frequency.  If this is zero then no timestamp functions will be called.
    uint32 CanIfTimestampFrequency;

    //! Indicates if there is a pending transmit confirmation
    boolean *CanIfPendingTxConfirm;

    //! List of controller modes.  Length is equal to CanIfNumController.
    CanIf_ControllerModeType *CanIfControllerMode;

    //! List of transceiver modes.  Length is equal to CanIfNumChannels.
    CanTrcv_TrcvModeType *CanIfTrcvMode;

    //! Pointer to CAN Hardware Object Handle/PDU Id Configuration
    const CanIf_PduHandleType *CanIfPduHandleCfg;

    //! Pointer to PDU CAN Id Mapping Configuration
    const Can_IdType *CanIfPduCanIdMappingCfg;

    //! Pointer to CAN Pdu User Callback value, CanIf_PduUserCallbackULType
    const CanIf_PduUserCallbackULType *CanIfPduUserCallbackULCfg;

    //! Pointer to CAN Pdu User Callback Name Configuration
    const struct CanIf_PduUserCallback *CanIfPduUserCallbackNameCfg;

    //-- functions --
    //! Called during init to configure the CAN messages based on the Anchor ID.
    void (* CanIf_Cfg_SetAnchorCanId)( void );

    ///! Passes a frame to the CAN transceiver for sending.
    Can_ReturnType (* CanTrcv_Write)( Can_HwHandleType Hth, const Can_PduType *PduInfo );

    Can_ReturnType (* CanTrcv_SetControllerMode)( uint8 Controller, Can_StateTransitionType Transition );

    Std_ReturnType (* CanTrcv_SetOpMode)( uint8 Transceiver, CanTrcv_TrcvModeType OpMode );

    Std_ReturnType (* CanTrcv_GetOpMode)( uint8 Transceiver, CanTrcv_TrcvModeType *OpMode );

    //! Disable CAN peripheral interrupts.  Optional function, may be NULL.
    void (* CanTrcv_DisableAllInterrupts)( uint8 Controller );

    //! Stop any pending transmission.  Optional function, may be NULL.
    void (* CanTrcv_ClearTXBRP)( uint8 Controller );

    uint16 (* CanTrcv_GetCurrentTimeStampCounterValue)( uint8 Id );

    uint16 (* CanTrcv_GetCurrentTxTimeStamp)( uint8 Id);
} CanIf_ConfigType;


#endif /* CANIF_TYPES_H */


//-------------------- End of File -------------------------------------------
