//----------------------------------------------------------------------------
// COPYRIGHT RESERVED, 2015 Robert Bosch (Australia) Pty Ltd.
// All rights reserved.  The reproduction, distribution and utilisation of
// this document as well as the communication of its contents to others
// without explicit authorisation is prohibited.  Offenders will be held liable
// for the payment of damages.  All rights reserved in the event of the grant
// of a patent, utility model or design.
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//! \file
//! \brief Configures cores service for the UDS Module.
//!
//! Module Description see according C file
//----------------------------------------------------------------------------

#ifndef RB_UDS_CORE_CFG_H
#define RB_UDS_CORE_CFG_H

//----------------------------------------------------------------------------
//  Includes
//----------------------------------------------------------------------------
#include "rbd_Pk.h"
#include "rbUds_ApiCfg.h"

//----------------------------------------------------------------------------
// Global variable declarations
//----------------------------------------------------------------------------

extern uint8 rbUds_PwrReqID;

//----------------------------------------------------------------------------
// Macros
//----------------------------------------------------------------------------

//! Enable services by setting to '1'.
#define VECTOR_CANDESC                      0

#define UDS_CFG_CORE_READ_DTC_ENABLE        1
#define UDS_CFG_CORE_WRITE_DID_ENABLE       1
#define UDS_CFG_CORE_IOCTRL_ENABLE          0

#if defined (RBD_PK_FBL)
#define UDS_CFG_CORE_UPLOAD_DOWNLOAD_ENABLE 1
#else
#define UDS_CFG_CORE_UPLOAD_DOWNLOAD_ENABLE 0
#endif

#define UDS_CFG_CORE_SAVE_RESTORE_ENABLE    1
#define UDS_CFG_CORE_CLEAR_DTC_ENABLE       1
#define UDS_CFG_CORE_ECU_RESET_ENABLE       1

#define FUNCTION_ADDRESSING                 1     // Allow Tester Present while other commands are executing

// Response timeout [ms]
#define UDS_P2_SERVER_MAX_MS                50u
// Response timeout [ms]
#define UDS_P2_STAR_SERVER_MAX_MS           5000u
// Session timeout [ms]
#define UDS_S3_SERVER_MAX_MS                5000u
//! Time between response pendings.
#define UDS_RCRRP_INTERVAL_MS               1000u
//! Duration of maximum number of Pending Responses.
#define UDS_RCRRP_LIMIT_MS                  (10u * UDS_RCRRP_INTERVAL_MS)

//! Duration of maximum number of Pending Responses for Security access.
#define UDS_SECUIRTY_RCRRP_LIMIT_MS                  (60u * UDS_RCRRP_INTERVAL_MS)

#define UDS_CFG_CORE_ECU_RESET_PERMITTED_SESSIONS       UDS_SESSION_ANY



#define UDS_UPDOWN_PRODUCT_HEADER_CORE_LENGTH   22u      // Prog Id (2) + Active and Update FW Image Addr (8) + Burst CMDA/B value (8) Type Flash (2) + Config Length (2)
#define UDS_UPDOWN_PRODUCT_HEADER_CHKSUM_LENGTH 2u
#define UDS_UPDOWN_PRODUCT_HEADER_COPY_SIZE     64u    // 22 + FLASH Write Config Length.

#endif // RB_UDS_CORE_CFG_H

//-------------------- End of File -------------------------------------------
