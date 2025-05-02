//----------------------------------------------------------------------------
// COPYRIGHT RESERVED, 2015 Robert Bosch (Australia) Pty Ltd.
// All rights reserved.  The reproduction, distribution and utilisation of
// this document as well as the communication of its contents to others
// without explicit authorisation is prohibited.  Offenders will be held liable
// for the payment of damages.  All rights reserved in the event of the grant
// of a patent, utility model or design.
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//! \file rbUds_Core.h
//! \brief Private decalarations for the UDS Module.
//!
//! Module Description see corresponding C file
//----------------------------------------------------------------------------

#ifndef RB_UDS_CORE_H
#define RB_UDS_CORE_H

//----------------------------------------------------------------------------
//  Includes
//----------------------------------------------------------------------------

#include <stdint.h>
#include <stdbool.h>


#include "Std_Types.h"
#include "rbUds_CoreCfg.h"


//----------------------------------------------------------------------------
// Macros
//----------------------------------------------------------------------------

#define RB_UDS_MODULEID 53      //< Autosar module ID for Dcm

// Det error codes
#define RB_UDS_E_VALUE          1
#define RB_UDS_E_LENGTH         2
#define RB_UDS_E_NULL           3
#define RB_UDS_E_STATE_SEQ      4
#define RB_UDS_E_TIMER_ERR      5
#define RB_UDS_E_TIMEOUT        6
#define RB_UDS_E_ERASE_ERR      7
#define RB_UDS_E_SEQ_CTR        8
#define RB_UDS_E_WRITE_ERR      9
#define RB_UDS_E_MEMORY_SIZE    10
// Det services
#define RB_UDS_S_APIDATAINDICATION          1
#define RB_UDS_S_CORERESPONSEPENDINGSUBTASK 2

#define RB_UDS_UPDOWN_REQUEST_DOWNLOAD      10
#define RB_UDS_UPDOWN_REQUEST_DOWNLOAD_CBK  11
#define RB_UDS_UPDOWN_TRANSFER_DATA         12
#define RB_UDS_UPDOWN_TRANSFER_DATA_CBK     13
#define RB_UDS_UPDOWN_TRANSFER_EXIT         14
#define RB_UDS_UPDOWN_TRANSFER_EXIT_CBK     15

//----------------------------------------------------------------------------
// Types
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//! \brief Pending service handler function pointer
//!
//! Each function provides the pending completion handling for a given SID.
//----------------------------------------------------------------------------

typedef FUNCPTR_ATTR uint8_t (*Uds_CoreServicePending_fn)(
    uint8_t             * const Rsp_a,
    uint16_t            * const RspLen_p,
    uint8_t       const * const Req,
    uint16_t      const         ReqLen
    );


//----------------------------------------------------------------------------
// Private Variables
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Private Functions
//----------------------------------------------------------------------------

void Uds_CoreReset(void);

void Uds_CoreCyclic(void);

void Uds_CoreTxAborted(void);
void Uds_CoreTxConfirmation(void);
void rbUds_CoreStartS3(void);

void Uds_CoreCfgSendResponse(
    uint8_t     const Resp_a[UDS_BUFFER_SIZE],
    uint16_t    const RespLen,
    bool        const SuppressPosResp
    );

uint8_t Uds_CoreCheckSessionSecurity(
    uint8_t     const PermittedSession,
    uint8_t     const PermittedSecurity
    );

void Uds_CoreCfgRestoreSessionAndSecurityContext(void);/*AXIVION Line MisraC2012-8.6 :Definition exists in rbuds_CoreCfg.c*/

void Uds_CoreCfgCompleteSessionChange(void);/*AXIVION Line MisraC2012-8.6 :Definition exists in rbuds_CoreCfg.c*/

bool Uds_CoreIsBusy(void);

void Uds_CorePwrReqIDInit(void);

#endif // RB_UDS_CORE_H

//-------------------- End of File -------------------------------------------
