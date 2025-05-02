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
//! \brief  Public Header File of the CanNm Module
//!
//! Module Description
//!     See corresponding C file
//----------------------------------------------------------------------------

//! \addtogroup CanNm CanNm
//! CanNM interface
//! @{

#ifndef RBD_CANNM_H
#define RBD_CANNM_H

#include "ComStack_Types.h"
#include "Std_Types.h"

#include "rbdPwrMgmtAnc.h"

//----------------------------------------------------------------------------
// Public Macros
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Public Types
//----------------------------------------------------------------------------
typedef enum {
    /*! Bus-Sleep Mode */
    NM_MODE_BUS_SLEEP,
    /*! Prepare Bus-Sleep Mode */
    NM_MODE_PREPARE_BUS_SLEEP,
    /*! Network Mode */
    NM_MODE_NETWORK,
    /*! maximum NM State number */
    NM_MODE_MAX_NUMBER
} CanNm_ModeType;

typedef enum {
    /*! Bus Sleep State */
    NM_STATE_BUS_SLEEP,
    /*! Prepare Bus Sleep State */
    NM_STATE_PREPARE_BUS_SLEEP,
    /*! Ready Sleep State */
    NM_STATE_READY_SLEEP,
    /*! Normal Operation State */
    NM_STATE_NORMAL_OPERATION,
    /*! Repeat Message State */
    NM_STATE_REPEAT_MESSAGE,
    /*! maximum NM State number */
    NM_STATE_MAX_NUMBER
} CanNm_StateType;

typedef struct {
    uint8 RepeatMessageRequest          :1;
    uint8 Reserved_NMCoordinatorID_Low  :1;
    uint8 Reserved_NMCoordinatorID_High :1;
    uint8 NMCoordinatorSleepReady       :1;
    uint8 ActiveWakeup                  :1;
    uint8 Reversed_1                    :1;
    uint8 PartialNetworkInformation     :1;
    uint8 Reserved_2                    :1;
} CanNm_AutosarControlBitVector_st;

typedef struct {
    uint8                               NodeId;
    CanNm_AutosarControlBitVector_st    AutosarNMControl;
    uint8                               unused[6];
} CanNm_Frame_st;




#define CANNM_FRAME_LENGTH           sizeof(CanNm_Frame_st)    //!< NM frame length
typedef struct {
    PduIdType       Id;
    union
    {           /*AXIVION Line MisraC2012-19.2 : Union is used here for data conversion, which is allowed in our project. */
        CanNm_Frame_st  f;                          //!< Frame structure
        uint8           b[CANNM_FRAME_LENGTH];  //!< Bytes
    } data;
    PduLengthType   Length;
}CanNm_MsgInfo_St;

typedef enum {
    NM_NETWORK_STATE_REQUESTED,
    NM_NETWORK_STATE_RELEASED
} CanNm_AdditionalNetworkStateType;



//----------------------------------------------------------------------------
// Public Variables
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//! \brief  Module configuration
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Public Functions
//----------------------------------------------------------------------------

void           CanNm_Init ( void );
Std_ReturnType CanNm_PassiveStartUp ( void );
Std_ReturnType CanNm_NetworkRequest ( void );
Std_ReturnType CanNm_NetworkRelease ( void );
Std_ReturnType CanNm_GetState ( CanNm_StateType* nmStatePtr, CanNm_ModeType* nmModePtr );
void           CanNm_MainFunction ( void );
void           CanNm_SetCanIdx(uint8 CanIdx);
// Separate function names for Fbl version to prevent any linker confusion
void           CanNm_FblInit ( void );
void           CanNm_FblMainFunction ( void );


//! @}
#endif // include guard

//-------------------- End of File -------------------------------------------
