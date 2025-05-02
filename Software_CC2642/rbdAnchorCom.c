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
//! \brief
//----------------------------------------------------------------------------


//============================================================================
//  Includes
//============================================================================

#include <assert.h>

#include "Appl_NmCbk.h"
#include "rbd_Pk.h"
#include "DimOf.h"

#include "rbdAnchorCom_Api.h"
#include "rbdAnchorCom_Private.h"

#ifdef RBD_PK_MASTER_MCU
#include "Rte_rbdAnchorCom.h"        // RTE-generated file, doesn't exist on other ECUs
#include "rbdAnchorCom_CanIf.h"
#include "CanNm_Cbk.h"
#include "Csm_cfg.h"
#else
#include "StbM.h"
#endif


#include "Csm.h"


//============================================================================
// Local Macros
//============================================================================
#define DEFAULT_KEY_LENGTH (16U)

//============================================================================
//Enumeration
//============================================================================

//============================================================================
// Local Types
//============================================================================


//============================================================================
// Global Variables
//============================================================================

//============================================================================
// Local Variables
//============================================================================

//============================================================================
// Local Function Prototypes
//============================================================================

//============================================================================
// Function declarations.
//============================================================================


//------------------------------------------------------------------------------
//! \brief  Initialisation function
//------------------------------------------------------------------------------
void rbdAnchorCom_Init( void )
{
    rbdAnchorComTp_Init();

#if STD_ON==RBDANCHORCOM_TEST
    rbdAnchorCom_TestInit();
#endif
}


void rbdAnchorCom_CyclicTx( void )
{
    rbdAnchorComTp_CyclicTx();

#if STD_ON==RBDANCHORCOM_TEST
    rbdAnchorCom_TestCyclic();
#endif
}


//------------------------------------------------------------------------------
/** Is transmission allowed?

 The conditions for allowing transmission are different for master and
 anchors.
 - Master has to wait until the receiving node is alive.
 - Anchors have to wait until the freshness timer for SecOC (RadioTime) is
   available.

@rst
..  impl::
    :id: rbdAnchorCom_NodeOkToTx
    :links_satisfies: rbdAnchorCom_NodeOkToTx_2
@endrst
*///--------------------------------------------------------------------------------
uint32 rbdAnchorCom_NodeOkToTx( void )
{
    uint32 result = 0;

#if defined(RBD_PK_MASTER_MCU)
    // MasterMcu can always transmit, because it originates the time signal used for SecOC freshness.
    // However, we wait until an anchor is awake, to prevent missed messages.
    AnchorDtcStatus_a LostCommStatus;
    uint8 nodeIdx;

    Rte_Read_RP_BackboneLostComms_Status(LostCommStatus);

    for(nodeIdx = 0; nodeIdx < RBD_PK_MAX_NODES; nodeIdx++)
    {
        if(LostCommStatus[nodeIdx] == DTC_PASS)
        {
            //Mark it as TRUE(1);
            result |= (1 << nodeIdx);
        }
    }
#else
    StbM_TimeStampType  timeStamp = {.nanoseconds=0, .seconds=0, .secondsHi=0, .timeBaseStatus=0};
    StbM_UserDataType   userData = {.userDataLength=0, .userByte0=0, .userByte1=0, .userByte2=0};

    // UserData from StbM shows if we have synchronised with the Master.
    /*AXIVION Next Line MisraC2012-2.2 MisraC2012-14.3: defensive return value checking */
    if( E_OK == StbM_GetCurrentTime(StbM_STBT_RadioTime, &timeStamp, &userData) ) /*AXIVION Line CertC-STR34 : comparison is of same types*/
    {
        if( (userData.userDataLength>0u) && (userData.userByte0!=0u) )
        {
            // Non-zero user data shows we've synced time with the Master, so we can send data
            result = 0xFFFFFFFFu;
        }
    }
#endif

    return result;
}


//-------------------- End of File -------------------------------------------
