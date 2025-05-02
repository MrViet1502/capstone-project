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
//! \brief UDS core service processing
//!
//! \par Module Description
//! Initial filtering and processing of a UDS message.
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//  Includes
//----------------------------------------------------------------------------
#include <stdint.h>
#include <stddef.h>
#include "Mcu.h"
#include "rbUds_CoreCfg.h"
#include "rbUds_Core.h"
#include "rbdElTime_Api.h"

#include "rbUds_Api.h"
#include "rbUds_Did.h"
#include "rbUds_Routine.h"
#if (1 == UDS_CFG_CORE_READ_DTC_ENABLE) || (1 == UDS_CFG_CORE_CLEAR_DTC_ENABLE)
#if (defined(RBD_PK_APP))
#include "rbUds_Dtc.h"
#endif
#endif
#include "rbUds_Session.h"  // PRQA S 0839
// QAC Msg(1:0839) File has already been included indirectly from this file. AE Action:  If defect: code must be changed (see AE-addition). If warning: code must be changed if better solution available. Otherwise, ignore warning.
// wy12cl QAC 7.1.1 bug. COmpiler disagrees.

#if (1 == UDS_CFG_CORE_IOCTRL_ENABLE)
#include "rbUds_IoCtrl.h"
#endif  // UDS_CFG_CORE_IOCTRL_ENABLE

#include "rbUds_Security.h"

#if (1 == UDS_CFG_CORE_UPLOAD_DOWNLOAD_ENABLE)
#include "rbUds_UpDown.h"
#endif  // UDS_CFG_CORE_UPLOAD_DOWNLOAD_ENABLE

#if (1 == UDS_CFG_CORE_ECU_RESET_ENABLE)
#include "rbUds_EcuReset.h"
#endif  // UDS_CFG_CORE_ECU_RESET_ENABLE

#include "rbUds_Pack.h"
#include "rbUds_Iso14229.h"

#include "Det.h"

#include "DimOf.h"
#if defined(RBD_PK_CPU_CC26xx) || defined(RBD_PK_MINI)
#include "rbdElTime_Api.h"
#endif

#include "rbdPwrReq.h"
//----------------------------------------------------------------------------
// Local Macros
//----------------------------------------------------------------------------

//! Length of sub-function code [bytes].
#define UDS_SUBFN_LEN                   1
//! Length of service ID code [bytes].
#define UDS_SID_LEN                     1
//! Value to signal no pending operation.
#define UDS_CORE_INVALID_RCRRP_INDEX    (~0u)/*AXIVION Line MisraC2012-1.1 :Defensive coding */
//! Mask for suppress positive response bit.
#define UDS_CORE_SPRMIB_MASK            ((uint8_t) BIT_07)
//! Mask for sub-function value.
#define UDS_CORE_SUBFN_MASK             ((uint8_t) (    BIT_06 \
                                                    |   BIT_05 \
                                                    |   BIT_04 \
                                                    |   BIT_03 \
                                                    |   BIT_02 \
                                                    |   BIT_01 \
                                                    |   BIT_00 \
                                                    ))
//! Length of negative response [bytes]
#define UDS_CORE_NEG_RESP_LEN           3

//! Negative response SID.
#define SID_NR                 ((uint8_t) 0x7Fu)
//! Positive response SID adjustment.
#define SID_PR                 ((uint8_t) 0x40u)

//----------------------------------------------------------------------------
//! \brief  Clear DTC message header and data offset from ISO-14229.
//----------------------------------------------------------------------------
enum rbUds_ClearDtcHeader_en
{
    RB_UDS_CDTCI_GODTC_HB_OFFSET,   //!< Index of group of DTC high byte.
    RB_UDS_CDTCI_GODTC_MB_OFFSET,   //!< Index of group of DTC mid byte.
    RB_UDS_CDTCI_GODTC_LB_OFFSET,   //!< Index of group of DTC low byte.

    RB_UDS_CDTCI_HDR_LEN            //!< Minimum length of header data.
};

//! Index of data following routine header.


//----------------------------------------------------------------------------
//! \brief  Read DTC message header and data offset from ISO-14229.
//----------------------------------------------------------------------------
enum rbUds_SubFnOnlyHeader_en
{
    RB_UDS_SUB_FN_OFFSET,           //!< Index of routine sub-function.

    RB_UDS_SUBFN_ONLY_HDR_LEN       //!< Minimum length of header data.
};

//! Index of data following routine header.
#define RB_UDS_SUBFN_ONLY_DATA_OFFSET      RB_UDS_SUBFN_ONLY_HDR_LEN

//----------------------------------------------------------------------------
//! \brief  Request download message header and data offset from ISO-14229.
//----------------------------------------------------------------------------
enum rbUds_RequestDownloadHeader_en
{
    RB_UDS_DATA_FORMAT_OFFSET,      //!< Data format ID [byte].
    RB_UDS_ADDR_LEN_FORMAT_OFFSET,  //!< Address and length format ID [byte].
    RB_UDS_MEM_ADDR_OFFSET,         //!< Memory address [minimum byte].
    RB_UDS_MEM_SIZE_MIN_OFFSET,     //!< Memory size [minimum byte].

    RB_UDS_REQ_DOWNLOAD_HDR_LEN     //!< Minimum length of header data.
};

//----------------------------------------------------------------------------
//! \brief  Transfer data message header and data offset from ISO-14229.
//----------------------------------------------------------------------------
enum rbUds_TransferDataHeader_en
{
    RB_UDS_BLOCK_SEQ_COUNTER_OFFSET,    //!< Block sequence counter [byte].

    RB_UDS_TRANSFER_DATA_HDR_LEN        //!< Minimum length of header data.
};

//----------------------------------------------------------------------------
//! \brief  Request transfer exit message header and data offset from ISO-14229.
//----------------------------------------------------------------------------
enum rbUds_RequestTransferExitHeader_en
{
    RB_UDS_REQ_TRANSFER_EXIT_HDR_LEN    //!< Minimum length of header data.
};

//----------------------------------------------------------------------------
//! \brief  Negative response Request data offset from ISO-14229.
//----------------------------------------------------------------------------
enum rbUds_NegativeResponse_en
{
    RB_UDS_NR_SID_NR,
    RB_UDS_NR_SID,
    RB_UDS_NR_CODE,

    RB_UDS_NR_LEN   //!< Minimum length of header data.
};

//----------------------------------------------------------------------------
// Local Types
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//! \brief Service handler function pointer
//!
//! Each function handles the common parametes and message packing for a given SID.
//! A further function handles sub-functions and parameters.
//----------------------------------------------------------------------------

typedef FUNCPTR_ATTR uint8_t (*Uds_CoreService_fn)(
        uint8_t     const   * const Req_a,          //!< Pointer to the request buffer.
        uint16_t    const           ReqLen          //!< The request length [bytes].
        );

//----------------------------------------------------------------------------
//! \brief The Module Name control block.
//!
//! It contains the current status of the Module
//----------------------------------------------------------------------------
typedef struct Uds_ControlBlock
{
    bool        isBusy;                             //!< This flag gets set when core is busy processing a uds command and cleared once this is finished
    //!< Server processing a message.
    bool        suppressPosResp;                    //!< Suppress postive response indicated by sub-function.
    bool        rcrrpTxConfirm;                     //!< Pending message transmission confirmation.
    uint8_t     rcrrpSidIndex;                      //!< SID requesting pending processing.
    rbd_ElTime_rbTimer_st rcrrpTimer;
    rbd_ElTime_rbTimer_st s3Timer;

    uint8_t  const * req;                                //!< Pointer to the request buffer
    uint16_t         reqLen;                             //!< Request length
    uint16_t         respLen;                            //!< UDS response length.
    uint8_t          resp_a[UDS_BUFFER_SIZE];            //!< UDS response buffer.
    uint8_t          negResp_a[UDS_CORE_NEG_RESP_LEN];   //!< UDS response buffer.
    rbd_ElTime_rbTimer_st rcrrpPendingTimer;             //!< Max duration for number of RCCRP
} Uds_ControlBlock_st;

//----------------------------------------------------------------------------
//! \brief Configuration block for a read DID.
//----------------------------------------------------------------------------
typedef struct Uds_CoreServiceElement
{
    uint8_t                     const sid;              //!< Service ID for this record.
    uint8_t                     const permittedSession; //!< Permitted session bit mask.
    uint8_t                     const minLength;        //!< minimum number request bytes.
    Uds_CoreService_fn          const Sid_fp;           //!< Service filtering function pointer.
    Uds_CoreServicePending_fn   const SidPending_fp;    //!< Service pending function pointer.
} Uds_CoreServiceElement_st;

//----------------------------------------------------------------------------
// Local Function ProtoTypes
//----------------------------------------------------------------------------


static void Uds_CoreResponsePendingSubtask(void);

static void Uds_CoreS3TimerSubtask(void);

static uint8_t Uds_CoreExtractSubFn(
    uint8_t const Byte
    );

static void Uds_CoreSendPositiveResponse(
    uint8_t const Sid
    );

static void Uds_CoreSendPendingResponse(
    uint8_t const Sid
    );

static void Uds_CoreSendNegativeResponse(
    uint8_t const Sid,
    uint8_t const ResponseCode
    );

static void Uds_CoreFillNegativeResponse(
    uint8_t const Sid,
    uint8_t const ResponseCode
    );

static void Uds_CoreSetIdle(void);

static uint8_t Uds_CoreRdbi(
    uint8_t     const   * const Req_a,
    uint16_t    const           ReqLen
    );

#if (1 == UDS_CFG_CORE_WRITE_DID_ENABLE)
static uint8_t Uds_CoreWdbi(
    uint8_t     const   * const Req_a,
    uint16_t    const           ReqLen
    );
#endif

static uint8_t Uds_CoreRc(
    uint8_t     const   * const Req_a,
    uint16_t    const           ReqLen
    );

#if defined(RBD_PK_APP)
#if (1 == UDS_CFG_CORE_READ_DTC_ENABLE)
static uint8_t Uds_CoreRdtci(
    uint8_t     const   * const Req_a,
    uint16_t    const           ReqLen
    );
#endif // UDS_CFG_CORE_READ_DTC_ENABLE
#endif

#if defined(RBD_PK_APP)
#if (1 == UDS_CFG_CORE_CLEAR_DTC_ENABLE)
static uint8_t Uds_CoreCdtci(
    uint8_t     const   * const Req_a,
    uint16_t    const           ReqLen
    );
#endif  // UDS_CFG_CORE_CLEAR_DTC_ENABLE
#endif

static uint8_t Uds_CoreDsc(
    uint8_t     const   * const Req_a,
    uint16_t    const           ReqLen
    );

static uint8_t Uds_CoreTp(
    uint8_t     const   * const Req_a,
    uint16_t    const           ReqLen
    );

static uint8_t Uds_CoreSa(
    uint8_t     const   * const Req_a,
    uint16_t    const           ReqLen
    );

#if (1 == UDS_CFG_CORE_UPLOAD_DOWNLOAD_ENABLE)
static uint8_t Uds_CoreRd(
    uint8_t     const   * const Req_a,
    uint16_t    const           ReqLen
    );

static uint8_t Uds_CoreTd(
    uint8_t     const   * const Req_a,
    uint16_t    const           ReqLen
    );

static uint8_t Uds_CoreRte(
    uint8_t     const   * const Req_a,
    uint16_t    const           ReqLen
    );
#endif  // UDS_CFG_CORE_UPLOAD_DOWNLOAD_ENABLE

#if (1 == UDS_CFG_CORE_ECU_RESET_ENABLE)
static uint8_t Uds_CoreEr(
    uint8_t     const   * const Req_a,
    uint16_t    const           ReqLen
    );
#endif  // UDS_CFG_CORE_ECU_RESET_ENABLE

static void UdsResponse_function(uint8_t *pResponseCode, uint8_t const *psid, uint32_t ii);
static void Uds_CoreResponsePendingTask(uint8_t *pResponseCode, uint8_t const *psid, uint16_t respLen);

//----------------------------------------------------------------------------
// Local Variables
//----------------------------------------------------------------------------

static Uds_CoreServiceElement_st const serviceCfg_a[] = // PRQA S 3674
// QAC Msg(2:3674) Array size defined implicitly with an initaliser. AE Action: If defect: code must be changed (see AE-addition). If warning: code must be changed if better solution available. Otherwise, Code must be commented, afterwards, ignore warning..
// wy12cl Deliberate. Configuration is variable dimension depending on selected options.
{
    {
        (uint8_t)RB_UDS_14229_SID_DSC,
        UDS_SESSION_ANY,
        1 + UDS_SUBFN_LEN,
        &Uds_CoreDsc,
        &rbUds_SessionCfgPendingCallback
    },

#if (1 == UDS_CFG_CORE_ECU_RESET_ENABLE)
    {
        (uint8_t)RB_UDS_14229_SID_ER,
        UDS_CFG_CORE_ECU_RESET_PERMITTED_SESSIONS,
        1 + UDS_SUBFN_LEN,
        &Uds_CoreEr,
        NULL
    },
#endif  // UDS_CFG_CORE_ECU_RESET_ENABLE

#if defined(RBD_PK_APP)
#if (1 == UDS_CFG_CORE_CLEAR_DTC_ENABLE)
    {
        (uint8_t)RB_UDS_14229_SID_CDTCI,
        (uint8_t)UDS_SESSION_MASK_EXTDS,
        1u + (uint8_t)RB_UDS_CDTCI_HDR_LEN,
        &Uds_CoreCdtci,
        &rbUds_DtcClearCfgClearIsComplete
    },
#endif  // UDS_CFG_CORE_CLEAR_DTC_ENABLE
#endif

#if defined(RBD_PK_APP)
#if (1 == UDS_CFG_CORE_READ_DTC_ENABLE)
    {
        (uint8_t)RB_UDS_14229_SID_RDTCI,
        ((uint8_t)UDS_SESSION_MASK_DS | (uint8_t)UDS_SESSION_MASK_EXTDS),/*AXIVION Line MisraC2012-10.1 : Typecast done properly */
        1 + UDS_SUBFN_LEN,      // minimum length is SID + SubFn
        &Uds_CoreRdtci,
        NULL
    },
#endif  // UDS_CFG_CORE_READ_DTC_ENABLE
#endif
    {
        (uint8_t)RB_UDS_14229_SID_RDBI,
        UDS_SESSION_ANY,
        1u + (uint8_t)RB_UDS_DID_HDR_LEN,
        &Uds_CoreRdbi,
        &rbUds_DidReadPendingCallback
    },
    {
        (uint8_t)RB_UDS_14229_SID_SA,
        ((uint8_t)UDS_SESSION_MASK_PRGS | (uint8_t)UDS_SESSION_MASK_EXTDS),/*AXIVION Line MisraC2012-10.1 : Typecast done properly */ // EBRCM02290519 - Work around for Security Access not allowed from non-DLC bus
        1 + UDS_SUBFN_LEN,
        &Uds_CoreSa,
        &rbUds_SecurityPendingCallback
    },

#if (1 == UDS_CFG_CORE_WRITE_DID_ENABLE)
    {
        (uint8_t)RB_UDS_14229_SID_WDBI,
        (uint8_t)UDS_SESSION_MASK_EXTDS,
        1u + (uint8_t)RB_UDS_DID_HDR_LEN + 1u,
        &Uds_CoreWdbi,
        &rbUds_DidWritePendingCallback
    },
#endif  // UDS_CFG_CORE_WRITE_DID_ENABLE

    {
        (uint8_t)RB_UDS_14229_SID_RC,
        (uint8_t)(UDS_SESSION_MASK_EXTDS | UDS_SESSION_MASK_PRGS),/*AXIVION Line MisraC2012-10.1 : Typecast done properly */
        1u + (uint8_t)UDS_SUBFN_LEN + (uint8_t)RB_UDS_DID_HDR_LEN,
        &Uds_CoreRc,
        &rbUds_RoutinePendingCallback
    },

#if (1 == UDS_CFG_CORE_UPLOAD_DOWNLOAD_ENABLE)
    {
        (uint8_t)RB_UDS_14229_SID_RD,
        (uint8_t)UDS_SESSION_MASK_PRGS,
        1u + (uint8_t)RB_UDS_REQ_DOWNLOAD_HDR_LEN,
        &Uds_CoreRd,
        &rbUds_RequestDownloadCfgCallback
    },
    {
        (uint8_t)RB_UDS_14229_SID_TD,
        (uint8_t)UDS_SESSION_MASK_PRGS,
        1u + (uint8_t)RB_UDS_TRANSFER_DATA_HDR_LEN,
        &Uds_CoreTd,
        &rbUds_TransferDataCfgCallback
    },
    {
        (uint8_t)RB_UDS_14229_SID_RTE,
        (uint8_t)UDS_SESSION_MASK_PRGS,
        1u + (uint8_t)RB_UDS_REQ_TRANSFER_EXIT_HDR_LEN,
        &Uds_CoreRte,
        &rbUds_TransferExitCfgCallback
    },
#endif  // UDS_CFG_CORE_UPLOAD_DOWNLOAD_ENABLE

    {
        (uint8_t)RB_UDS_14229_SID_TP,
        UDS_SESSION_ANY,
        1 + UDS_SUBFN_LEN,
        &Uds_CoreTp,
        NULL
    }
};

//----------------------------------------------------------------------------
//! \brief The UDS control block.
//!
//! It tracks the status of the module.
//!
//! Note: Initialise before use.
//----------------------------------------------------------------------------
static Uds_ControlBlock_st Uds_CtrlBlk;


static uint8 Uds_PwrReqID;
//----------------------------------------------------------------------------
// Function declarations.
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//! \brief REset core control block to idle
//!
//! Initialise the control structure on:
//! - Power-on reset.
//! - S3 timer expiration causing return to default session.
//!
//! \pre
//!     None.
//!
//! \post
//!     COre control block in idle.
//!
//! \return
//!     None.
//----------------------------------------------------------------------------
void Uds_CoreReset(void)
{
    Uds_CoreSetIdle();
    rbd_ElTime_rbTimerInit(&Uds_CtrlBlk.s3Timer);
}

//----------------------------------------------------------------------------
//! \brief Initialize Power Request ID
//!
//! \pre
//!     None.
//!
//! \post
//!     COre control block in idle.
//!
//! \return
//!     None.
//----------------------------------------------------------------------------
void Uds_CorePwrReqIDInit(void)
{
    Uds_PwrReqID = rbdPwrReq_GetID();
}

//----------------------------------------------------------------------------
//! \brief Process data indication (from TP).
//!
//! Checks the parameters and attempts to find a service for the requested SID.
//!
//! \pre
//!     None.
//!
//! \post
//!     Some services may leave UDS in the "pending" state.
//----------------------------------------------------------------------------
void Uds_ApiDataIndication(
    uint8_t     const   * const Req_a,
    uint16_t    const           ReqLen
    )
{


    // Any reception extends the session
    rbUds_CoreStartS3();

    if( (uint8_t *)NULL == Req_a)/*AXIVION Line MisraC2012-2.2, MisraC2012-14.3  : Defensive coding */
    {
        Det_ReportError( RB_UDS_MODULEID, 0, RB_UDS_S_APIDATAINDICATION, RB_UDS_E_NULL );
        Uds_CoreSendNegativeResponse(0, RB_UDS_14229_GR);
    }
    else if( ReqLen < (uint16_t)1u)/*AXIVION Line MisraC2012-2.2, MisraC2012-14.3  : Defensive coding */
    {
        // Shouldn't get here.
        Det_ReportError( RB_UDS_MODULEID, 0, RB_UDS_S_APIDATAINDICATION, RB_UDS_E_VALUE );
        Uds_CoreSendNegativeResponse(0, RB_UDS_14229_GR);
    }

#ifdef FUNCTION_ADDRESSING
    else if (true == Uds_CtrlBlk.isBusy)
    {
        // Allow only functional tester present during other service.

        uint8_t const sid = Req_a[0];

        if( (2u == ReqLen) && ((uint8_t)RB_UDS_14229_SID_TP==sid) && (0u==(Req_a[1] & 0x7Fu)) )
        {
            // TesterPresent extends the session. No response is sent.
        }
        else
        {
            // Ignore the request silently
        }
    }
#endif //FUNCTION_ADDRESSING
    else
    {
        uint8_t const   sid     = Req_a[0];
        bool            isFound = false;
        uint32_t        ii;

        Uds_CtrlBlk.req     = Req_a;
        Uds_CtrlBlk.reqLen  = ReqLen;
        Uds_CtrlBlk.respLen = 0;
        // Search for matching SID.
        for ( ii = 0; (ii < DIMOF(serviceCfg_a)) &&  (false == isFound); ii++ )/*AXIVION Line MisraC2012-2.2, MisraC2012-14.3  : Defensive coding */
        {
            Uds_CoreServiceElement_st   const   * const sidCfg_p        = &serviceCfg_a[ii];
            uint8_t                     const           currentSession  = rbUds_SessionGetState();

            // Process matching SID.
            if (sidCfg_p->sid == sid)
            {
                if (NULL == sidCfg_p->Sid_fp)
                {
                    // Programming error.
                    Det_ReportError( RB_UDS_MODULEID, 1, RB_UDS_S_APIDATAINDICATION, RB_UDS_E_NULL );
                    Uds_CoreSendNegativeResponse(sid, RB_UDS_14229_GR);
                }
                else if (0u == (currentSession & sidCfg_p->permittedSession))
                {
                    // Not recognised in current session.
                    Uds_CoreSendNegativeResponse(sid, RB_UDS_14229_SNSIAS);
                }
                else if (ReqLen < sidCfg_p->minLength)
                {
                    // Request too short.
                    Uds_CoreSendNegativeResponse(sid, RB_UDS_14229_IMLOIF);
                }
                else
                {
                    uint8_t responseCode;
                    Uds_CtrlBlk.isBusy = true;

                    // Specific processing of the remaining request.
                    responseCode = sidCfg_p->Sid_fp(            // PRQA S 3671
                                                    &Req_a[1],
                                                    (uint16_t) (ReqLen - 1u)
                                                    );
                    // QAC Msg(2:3671) Function called via pointer to function. AE Action: No action (informational only)..
                    // wy12cl Intentional call to configured function.
                    UdsResponse_function(&responseCode, &sid, ii);
                }

                // SID found and processed.
                isFound = true;
                break;
            }
        }

        if( !isFound )
        {
            Uds_CoreSendNegativeResponse(sid, RB_UDS_14229_SNS);
        }
    }
}
//----------------------------------------------------------------------------
//! \brief Process data indication (from TP).
//!
//! Checks the parameters and attempts to find a service for the requested SID.
//!
//! \pre
//!     None.
//!
//! \post
//!     Some services may leave UDS in the "pending" state.
//----------------------------------------------------------------------------
static void UdsResponse_function(uint8_t *pResponseCode, uint8_t const *psid, uint32_t ii)
{

    switch (*pResponseCode)
                    {
                    case RB_UDS_14229_PR:
                        {
                            Uds_CoreSendPositiveResponse(*psid);
                        }
                        break;

                    case RB_UDS_14229_RCRRP:
                        {
                            //Yes, start the rcrrp timer
                            rbd_ElTime_rbTimerStart(&Uds_CtrlBlk.rcrrpPendingTimer,UDS_RCRRP_LIMIT_MS);
                            // Check for pending support.
                            Uds_CoreServicePending_fn const Pending_fp = serviceCfg_a[ii].SidPending_fp;
                            if (NULL == Pending_fp)
                            {
                                // Shouldn't get here. Configuration error.
                                Uds_CoreSendNegativeResponse(*psid, RB_UDS_14229_GR);
                            }
                            else
                            {
                                // Save control information.
                                Uds_CtrlBlk.rcrrpSidIndex = (uint8_t)ii;

                                Uds_CoreSendPendingResponse(*psid);
                            }
                        }
                        break;

                    default:
                        {
                            Uds_CoreSendNegativeResponse(*psid, *pResponseCode);
                        }
                        break;
                    }

}
//----------------------------------------------------------------------------
//! \brief Receive request start indication.
//!
//! Called to indicate start of UDS request:
//! - Single frame.
//! - First frame.
//!
//! Activity implies a tester must be connected.
//! Stop the S3 timer whilst processing.
//!
//! \pre
//!     None.
//!
//! \post
//!     S3 timer stopped.
//!
//! \return
//!     None.
//----------------------------------------------------------------------------
void rbUds_ApiRxStarted(void)
{
    rbd_ElTime_rbTimerState_t const s3TimerState = rbd_ElTime_rbTimerUpdate(&Uds_CtrlBlk.s3Timer);

    switch (s3TimerState)
    {
    case RBD_ELTIME_RBTIMER_STATE_STOPPED:
        // Do nothing.
        break;

    case RBD_ELTIME_RBTIMER_STATE_IDLE:
        // Workaround restriction in STOPPING to allow S3 to reflect processing status.
        // Timer is IDLE until first request is seen.
        rbd_ElTime_rbTimerStart(&Uds_CtrlBlk.s3Timer, UDS_S3_SERVER_MAX_MS);
        rbd_ElTime_rbTimerStop(&Uds_CtrlBlk.s3Timer); //To Stop eltimer
        break;

    default:
        rbd_ElTime_rbTimerInit(&Uds_CtrlBlk.s3Timer); //To Stop eltimer
        break;
    }
}

//----------------------------------------------------------------------------
//! \brief Receive request aborted.
//!
//! Called to indicate abort of segmented UDS request
//!
//! Restart the S3 timer. Waiting for new request.
//!
//! \pre
//!     None.
//!
//! \post
//!     S3 timer started.
//!
//! \return
//!     None.
//----------------------------------------------------------------------------
void rbUds_ApiRxAborted(void)
{
    rbUds_CoreStartS3();
}

//----------------------------------------------------------------------------
//! \brief Transmit response aborted.
//!
//! Called to indicate abort of UDS response:
//! - Final frame.
//! - Final frame (RCRRP).
//! - Consecutive frame. Segmented response aborted.
//!
//! Restart the S3 timer unless RCRRP.
//! The latter implies UDS still processing.
//! Waiting for new request if started.
//!
//! \pre
//!     None.
//!
//! \post
//!     S3 timer started unless RCRRP.
//!
//! \return
//!     None.
//----------------------------------------------------------------------------
void Uds_CoreTxAborted(void)
{
    // Same as for TX confirmation.
    Uds_CoreTxConfirmation();
}

//----------------------------------------------------------------------------
//! \brief Transmit response success.
//!
//! Called to indicate successful transmit of UDS response:
//! - Final frame.
//! - Final frame (RCRRP).
//!
//! Restart the S3 timer unless RCRRP.
//! The latter implies UDS still processing.
//! Waiting for new request if started.
//!
//! \pre
//!     None.
//!
//! \post
//!     S3 timer started unless RCRRP.
//!
//! \return
//!     None.
//----------------------------------------------------------------------------
void Uds_CoreTxConfirmation(void)
{
    uint8_t const rcrrpSidIndex = Uds_CtrlBlk.rcrrpSidIndex;

    // Start S3 timer unless pending function flagged.
    if (rcrrpSidIndex >= DIMOF(serviceCfg_a))
    {
        rbUds_CoreStartS3();
    }
    else
    {
        Uds_CtrlBlk.rcrrpTxConfirm = true;
    }
}

//----------------------------------------------------------------------------
//! \brief Start the S3 timer
//!
//! Called to start the S3 (session) timer.
//!
//! \pre
//!     None.
//!
//! \post
//!     S3 timer started.
//!
//! \return
//!     None.
//----------------------------------------------------------------------------
void rbUds_CoreStartS3(void)
{
    rbd_ElTime_rbTimerStart(&Uds_CtrlBlk.s3Timer, UDS_S3_SERVER_MAX_MS);
    rbdPwrReq_RequestService(rbdPwrReq_Service_CPU,Uds_PwrReqID);
}

//----------------------------------------------------------------------------
//! \brief See if UDS is active.
//!
//! Reports whether a tester is actively communicating with UDS.
//!
//! \pre
//!     None.
//!
//! \post
//!     S3 timer updated.
//!
//! \return
//!     true    - Tester communication with S3 timeout.
//!     false   - No tester communication.
//----------------------------------------------------------------------------
bool rbUds_ApiIsS3Active(void)
{
    bool                    isActive;
    rbd_ElTime_rbTimerState_t const    s3TimerState = rbd_ElTime_rbTimerUpdate(&Uds_CtrlBlk.s3Timer);


    switch (s3TimerState)
    {
    case RBD_ELTIME_RBTIMER_STATE_RUNNING:
    case RBD_ELTIME_RBTIMER_STATE_STOPPED:
        {
            isActive = true;
        }
        break;

    default:
        {
            isActive = false;
        }
        break;
    }

    return isActive;
}

//----------------------------------------------------------------------------
//! \brief Check if there is pending sub-task
//!
//! \pre
//!     None.
//!
//! \post
//!     None
//!
//! \return
//!     true    - There is pending subtask
//!     false   - There is no pending subtask
//----------------------------------------------------------------------------
bool rbUds_ApiPendingSubtask(void)
{
    uint8_t const rcrrpSidIndex = Uds_CtrlBlk.rcrrpSidIndex;

    return rcrrpSidIndex < DIMOF(serviceCfg_a);
}

//----------------------------------------------------------------------------
//! \brief UDS cyclic task.
//!
//! Handle cyclic updates.
//!
//! This is a replacement for what Vector does with it's cyclic task and
//! response pending callback.
//!
//! \pre
//!     None.
//!
//! \post
//!     Pending may complete.
//!
//! \return
//!     None.
//----------------------------------------------------------------------------
void Uds_CoreCyclic(void)
{
    Uds_CoreResponsePendingSubtask();
    Uds_CoreS3TimerSubtask();
}

//----------------------------------------------------------------------------
//! \brief Response pending sub-taask.
//!
//! Handle response pending.
//!
//! \pre
//!     None.
//!
//! \post
//!     Pending may complete.
//!
//! \return
//!     None.
//----------------------------------------------------------------------------
static void Uds_CoreResponsePendingSubtask(void)
{
    uint8_t const rcrrpSidIndex = Uds_CtrlBlk.rcrrpSidIndex;

    // Pending function flagged?
    if (rcrrpSidIndex < DIMOF(serviceCfg_a))
    {
        Uds_CoreServicePending_fn   const Pending_fp    = serviceCfg_a[rcrrpSidIndex].SidPending_fp;
        uint8_t                     const sid           = serviceCfg_a[rcrrpSidIndex].sid;

        if (NULL == Pending_fp)
        {
            // Shouldn't get here.
            Det_ReportError( RB_UDS_MODULEID, 0, RB_UDS_S_CORERESPONSEPENDINGSUBTASK, RB_UDS_E_NULL );
            Uds_CoreSetIdle();
        }
        else
        {
            rbd_ElTime_rbTimerState_t const rcrrpTimerState = rbd_ElTime_rbTimerUpdate( &Uds_CtrlBlk.rcrrpTimer );
            // Has it taken too long?
            rbd_ElTime_rbTimerState_t const timerState = rbd_ElTime_rbTimerUpdate(&Uds_CtrlBlk.rcrrpPendingTimer);

            if (Uds_CtrlBlk.rcrrpTxConfirm == true)
            {
                // The final positive response to RCRRP may append data.
                // wy12cl FIXME:
                // - Application side SIDs have preemptively saved their DID/RID/sub-fn.
                // - PBL side SIDs currently assume all responsability for the positive response.
                // The latter is a legacy of hacking it together for Nissan B1.
                uint16_t       respLen         = 0;
                uint8_t        responseCode    = RB_UDS_14229_GR;
                if ((uint8_t)RB_UDS_14229_SID_RDBI == sid)
                {
                    responseCode = Pending_fp(   // PRQA S 3671
                            &Uds_CtrlBlk.resp_a[(uint8_t)UDS_SID_LEN + (uint8_t)RB_UDS_DID_HDR_LEN],
                            &respLen,
                            Uds_CtrlBlk.req,
                            Uds_CtrlBlk.reqLen
                    );
                }
                else
                {
                    responseCode = Pending_fp(   // PRQA S 3671
                            &Uds_CtrlBlk.resp_a[(uint16_t)UDS_SID_LEN + Uds_CtrlBlk.respLen],
                            &respLen,
                            Uds_CtrlBlk.req,
                            Uds_CtrlBlk.reqLen

                    );
                }

                if (RBD_ELTIME_RBTIMER_STATE_ELAPSED == timerState)/*AXIVION Line MisraC2012-10.4 : Different enums for different chip but of same value*/
                {
                    if(((uint8_t)RB_UDS_14229_SID_SA == sid)|| ((uint8_t)RB_UDS_14229_SID_DSC == sid)
                            || ((uint8_t)RB_UDS_14229_SID_ER == sid))/*AXIVION Line MisraC2012-10.4 : Different enums for different chip but of same value*/
                    {
                        /* Timed out. */
                        Mcu_PerformReset();
                    }
                    else
                    {
                        switch(sid)
                        {
                            case RB_UDS_14229_SID_RC: /*AXIVION Line MisraC2012-10.3:checked*/
                                rbUds_RoutineClearStateinfo();
                                break;
                            case RB_UDS_14229_SID_RDBI:/*AXIVION Line MisraC2012-10.3:checked*/
                                rbUds_DidReadClearStateinfo();
                                break;
                            case RB_UDS_14229_SID_WDBI:/*AXIVION Line MisraC2012-10.3:checked*/
                                rbUds_WriteDidClearStateinfo();
                                break;
                            default:/*AXIVION Line MisraC2012-16.1, MisraC2012-16.4 :checked*/
                                //Do nothing
                                break;
                        }
                        /* Timed out. */
                        responseCode = RB_UDS_14229_GR;
                    }

                }
                Uds_CoreResponsePendingTask(&responseCode, &sid, respLen);

            }
            else if (RBD_ELTIME_RBTIMER_STATE_ELAPSED == rcrrpTimerState)//TODO:RBD_ELTIME_RBTIMER_STATE_ELAPSED
            {
                // Tx confirmation timeout of previous pending message
                Det_ReportError( RB_UDS_MODULEID, 0, RB_UDS_S_CORERESPONSEPENDINGSUBTASK, RB_UDS_E_TIMEOUT );
                Uds_CoreSetIdle();
            }
            else
            {
                //Do nothing
            }
        }
    }
}
//----------------------------------------------------------------------------
//! \brief Response pending task.
//!
//! Send response pending.
//!
//! \pre
//!     None.
//!
//! \post
//!     Pending may complete.
//!
//! \return
//!     None.
//----------------------------------------------------------------------------
static void Uds_CoreResponsePendingTask(uint8_t *pResponseCode, uint8_t const *psid, uint16_t respLen )
{

    rbd_ElTime_rbTimerState_t const rcrrpTimerState = rbd_ElTime_rbTimerUpdate( &Uds_CtrlBlk.rcrrpTimer );

    switch (*pResponseCode)
                {
                case RB_UDS_14229_PR:
                    {
                        // Response length is the DID/RID/sub-fn plus any
                        // additional data that may have been appended the callback function.
                        Uds_CtrlBlk.respLen += respLen;/*AXIVION Line MisraC2012Directive-4.1 :Defensive coding*/
                        Uds_CoreSendPositiveResponse(*psid);
                    }
                    break;

                case RB_UDS_14229_RCRRP:
                    {
                        // Is another pending response required?
                        if (RBD_ELTIME_RBTIMER_STATE_ELAPSED == rcrrpTimerState)//TODO:RBD_ELTIME_RBTIMER_STATE_ELAPSED
                        {
                            rbUds_CoreStartS3();
                            Uds_CoreSendPendingResponse(*psid);
                        }
                    }
                    break;

                default:
                    {
                        Uds_CoreSendNegativeResponse(*psid, *pResponseCode);
                    }
                    break;
                }
}

//----------------------------------------------------------------------------
//! \brief Check tester activity timer.
//!
//! Evaluate the S3 timer. Change to default session if expired.
//!
//! Note: The timer check is in core as it replaces what Vector supplies.
//!
//! \pre
//!     None.
//!
//! \post
//!     UDS idle.
//!
//! \return
//!     None.
//----------------------------------------------------------------------------
static void Uds_CoreS3TimerSubtask(void)
{
    uint8_t                    const currentSession    = rbUds_SessionGetState();
    rbd_ElTime_rbTimerState_t  const s3TimerState      = rbd_ElTime_rbTimerUpdate(&Uds_CtrlBlk.s3Timer);

    // S3 time out action is only relevent in non-default sessions.
    if ((uint8_t) UDS_SESSION_MASK_DS != currentSession)
    {
        if (RBD_ELTIME_RBTIMER_STATE_ELAPSED == s3TimerState)
        {
            rbUds_SessionS3TimerExpiredReset();
            Uds_CoreReset();
        }
    }
    //S3 Timer elapsed release CPU power request.
    if (RBD_ELTIME_RBTIMER_STATE_ELAPSED == s3TimerState)
    {
        rbdPwrReq_ReleaseService(rbdPwrReq_Service_CPU,Uds_PwrReqID);
    }
}

//----------------------------------------------------------------------------
//! \brief Sub-function byte processing.
//!
//! Extract the sub-function code and suppress poositive response flag.
//!
//! \pre
//!     None.
//!
//! \post
//!     UDS idle.
//!
//! \return
//!     None.
//----------------------------------------------------------------------------
static uint8_t Uds_CoreExtractSubFn(
    uint8_t const Byte
    )
{
    if (UDS_CORE_SPRMIB_MASK == (uint8_t) (Byte & UDS_CORE_SPRMIB_MASK))
    {
        Uds_CtrlBlk.suppressPosResp = true;
    }

    return (uint8_t )(Byte & UDS_CORE_SUBFN_MASK);
}
//----------------------------------------------------------------------------
//! \brief Send UDS positive response.
//!
//! Send the final response to TP and go idle.
//!
//! \pre
//!     None.
//!
//! \post
//!     UDS idle.
//!
//! \return
//!     None.
//----------------------------------------------------------------------------
static void Uds_CoreSendPositiveResponse(
    uint8_t const Sid
    )
{
    // Prepend SID with positive response increment.
    Uds_CtrlBlk.resp_a[0]   =  (uint8_t) (Sid + SID_PR);/*AXIVION Line MisraC2012Directive-4.1 :Defensive coding*/
    Uds_CtrlBlk.respLen     += (uint16_t) 1u;/*AXIVION Line MisraC2012Directive-4.1 :Defensive coding*/

    Uds_CoreCfgSendResponse(
            &Uds_CtrlBlk.resp_a[0],
            Uds_CtrlBlk.respLen,
            Uds_CtrlBlk.suppressPosResp
            );
    // Fake a confirmation when response suppressed.
    if (true == Uds_CtrlBlk.suppressPosResp)
    {
        Uds_CoreTxConfirmation();
    }

    Uds_CoreSetIdle();
}

//----------------------------------------------------------------------------
//! \brief Send UDS pending response.
//!
//! Starts the repeat timer and sends a pending response to TP.
//!
//! \pre
//!     None.
//!
//! \post
//!     UDS pending timer (re)started.
//!
//! \return
//!     None.
//----------------------------------------------------------------------------
static void Uds_CoreSendPendingResponse(
    uint8_t const Sid
    )
{
    Uds_CoreFillNegativeResponse(Sid, RB_UDS_14229_RCRRP);
    Uds_CoreCfgSendResponse(
            &Uds_CtrlBlk.negResp_a[0],
            UDS_CORE_NEG_RESP_LEN,
            false
            );

    //zho1cl, double the timer due to slow IMC communication, need to be fixed
    rbd_ElTime_rbTimerStart(&Uds_CtrlBlk.rcrrpTimer, 3u*UDS_RCRRP_INTERVAL_MS);

    Uds_CtrlBlk.suppressPosResp = false;
    Uds_CtrlBlk.rcrrpTxConfirm = false;
    // Response pending processing now in play.
}

//----------------------------------------------------------------------------
//! \brief Send UDS negative response.
//!
//! Send the final response to TP and go idle.
//!
//! \pre
//!     None.
//!
//! \post
//!     UDS idle.
//!
//! \return
//!     None.
//----------------------------------------------------------------------------
static void Uds_CoreSendNegativeResponse(
    uint8_t const Sid,
    uint8_t const ResponseCode
    )
{
    Uds_CoreFillNegativeResponse(Sid, ResponseCode);
    Uds_CoreCfgSendResponse(
            &Uds_CtrlBlk.negResp_a[0],
            UDS_CORE_NEG_RESP_LEN,
            false
            );
    Uds_CoreSetIdle();
}

//----------------------------------------------------------------------------
//! \brief Set UDS negative response values.
//!
//! \pre
//!     None.
//!
//! \post
//!     Response fields set.
//!
//! \return
//!     None.
//----------------------------------------------------------------------------
static void Uds_CoreFillNegativeResponse(
    uint8_t const Sid,
    uint8_t const ResponseCode
    )
{
    // Form response.
    Uds_CtrlBlk.negResp_a[RB_UDS_NR_SID_NR] = SID_NR;
    Uds_CtrlBlk.negResp_a[RB_UDS_NR_SID]    = Sid;
    Uds_CtrlBlk.negResp_a[RB_UDS_NR_CODE]   = ResponseCode;
}

//----------------------------------------------------------------------------
//! \brief Set control block to idle.
//!
//! \pre
//!     None.
//!
//! \post
//!     UDS is not processing a request.
//!
//! \return
//!     None.
//----------------------------------------------------------------------------
static void Uds_CoreSetIdle(void)
{
    Uds_CtrlBlk.isBusy          = false;
    Uds_CtrlBlk.rcrrpTxConfirm  = false;
    Uds_CtrlBlk.rcrrpSidIndex   = (uint8_t)UDS_CORE_INVALID_RCRRP_INDEX;/*AXIVION Line MisraC2012-10.3,MisraC2012-12.4 :Implicit conversion is not done */
    Uds_CtrlBlk.respLen         = 0;
    Uds_CtrlBlk.suppressPosResp = false;

    rbd_ElTime_rbTimerInit(&Uds_CtrlBlk.rcrrpTimer); //Cancel Eltime
}

//----------------------------------------------------------------------------
//! \brief Check the session and security permissions.
//!
//! \pre
//!     None.
//!
//! \post
//!     None.
//!
//! \return
//!     UDS response code.
//----------------------------------------------------------------------------
uint8_t Uds_CoreCheckSessionSecurity(
    uint8_t     const           PermittedSession,
    uint8_t     const           PermittedSecurity
    )
{
    uint8_t         responseCode;
    uint8_t const   currentSession    = rbUds_SessionGetState();
    uint8_t const   currentSecurity   = rbUds_SecurityGetLevel();

    if (    ((uint8_t) UDS_SESSION_MASK_DS      == currentSession)
        &&  ((uint8_t) RB_UDS_SECURITY_MASK_ANY != PermittedSecurity)
        )
    {
        // Not recognised as security other than NONE requires non-default session.
        responseCode = RB_UDS_14229_GR;
    }
    else if (0u == (currentSession & PermittedSession))
    {
        // Not recognised in current session.
        responseCode = RB_UDS_14229_SNSIAS;
    }
    else if (0u == (currentSecurity & PermittedSecurity))
    {
        // Incorrect security level.
        responseCode = RB_UDS_14229_SAD;
    }
    else
    {
        responseCode = RB_UDS_14229_PR;
    }

    return responseCode;
}

//----------------------------------------------------------------------------
//! \brief Handle read data by identifier.
//!
//! Common handling for all DIDs
//! - Extract DID.
//! - Call specific handler.
//! - Pack common response.
//!
//! \pre
//!     None.
//!
//! \post
//!     Control block contains response data.
//!
//! \return
//!     None.
//----------------------------------------------------------------------------
static uint8_t Uds_CoreRdbi(
    uint8_t     const   * const Req_a,
    uint16_t    const           ReqLen
    )
{
    uint8_t responseCode;

    if ((uint16_t) RB_UDS_DID_HDR_LEN != ReqLen)
    {
        // Exactly one DID is supported.
        Uds_CtrlBlk.respLen = 0;
        responseCode        = RB_UDS_14229_IMLOIF;
    }
    else
    {
        uint16_t        respLen = 0;
        uint16_t const  did     = rbUds_Make16Bit(
                                        Req_a[RB_UDS_DID_MSB_OFFSET],   // PRQA S 0506
                                        Req_a[RB_UDS_DID_LSB_OFFSET]
                                        // QAC Msg(2:0506) Dereferencing pointer value that is possibly NULL. AE Action: (mandatory) If defect: code must be changed (see AE-addition). If warning: code must be changed if better solution available. Otherwise, do not deactivate warning.
                                        // wy12cl Library will not cannot pass NULL.
                                        );
        // Specific handler returns only its data.
        responseCode = rbUds_DidRead(
                            &Uds_CtrlBlk.resp_a[(uint8_t)UDS_SID_LEN + (uint8_t)RB_UDS_DID_HDR_LEN],
                            &respLen,
                            did
                            );
        if (((uint8_t) RB_UDS_14229_PR == responseCode) ||  ((uint8_t) RB_UDS_14229_RCRRP == responseCode))
        {
            // Copy DID.
            Uds_CtrlBlk.resp_a[(uint8_t)UDS_SID_LEN + (uint8_t)RB_UDS_DID_MSB_OFFSET] = Req_a[RB_UDS_DID_MSB_OFFSET];/*AXIVION Line MisraC2012-2.2 :no issue here. Misra complaint is the (uint8_t) added before a enum 0(RB_UDS_DID_MSB_OFFSET). However, if not added, Misra warnings is also created due to arithmatic adding enums*/
            Uds_CtrlBlk.resp_a[(uint8_t)UDS_SID_LEN + (uint8_t)RB_UDS_DID_LSB_OFFSET] = Req_a[RB_UDS_DID_LSB_OFFSET];

            //is the data pending?
            if( RB_UDS_14229_RCRRP == responseCode )
            {
                //yes, then zero the length here -- the actual data length will be set in the pending response (but fall thru and set the header length)
                respLen = 0;
            }

            // Set return data length.
            Uds_CtrlBlk.respLen                                     = (uint16_t) ((uint16_t)RB_UDS_DID_HDR_LEN + respLen);/*AXIVION Line MisraC2012Directive-4.1 :Defensive coding*/
        }
        else
        {
            Uds_CtrlBlk.respLen = 0;
        }
    }

    return responseCode;
}

//----------------------------------------------------------------------------
//! \brief Handle write data by identifier.
//!
//! Common handling for all DIDs
//! - Extract DID.
//! - Call specific handler.
//! - Pack common response.
//!
//! \pre
//!     None.
//!
//! \post
//!     Control block contains response data.
//!
//! \return
//!     None.
//----------------------------------------------------------------------------

#if (1 == UDS_CFG_CORE_WRITE_DID_ENABLE)

static uint8_t Uds_CoreWdbi(
    uint8_t     const   * const Req_a,
    uint16_t    const           ReqLen
    )
{
    uint16_t            respLen         = 0;
    uint16_t    const   did             = rbUds_Make16Bit(
                                                Req_a[RB_UDS_DID_MSB_OFFSET],   // PRQA S 0506
                                                Req_a[RB_UDS_DID_LSB_OFFSET]
                                                // QAC Msg(2:0506) Dereferencing pointer value that is possibly NULL. AE Action: (mandatory) If defect: code must be changed (see AE-addition). If warning: code must be changed if better solution available. Otherwise, do not deactivate warning.
                                                // wy12cl Library will not cannot pass NULL.
                                                );
    // Specific handler returns only its data.
    uint8_t     const   responseCode    = rbUds_DidWrite(
                                                &Uds_CtrlBlk.resp_a[(uint8_t)UDS_SID_LEN + (uint8_t)RB_UDS_DID_HDR_LEN],
                                                &respLen,
                                                &Req_a[RB_UDS_DID_HDR_LEN],/*AXIVION Line MisraC2012-18.1 :Defensive Coding */
                                                ReqLen - (uint8_t)RB_UDS_DID_HDR_LEN,/*AXIVION Line MisraC2012Directive-4.1 :Defensive coding*/
                                                did
                                                );
    // DID required for PR now or after pending.
    if (    ((uint8_t) RB_UDS_14229_PR    == responseCode)/*AXIVION Line MisraC2012-2.2, MisraC2012-14.3  : Defensive coding */
        ||  ((uint8_t) RB_UDS_14229_RCRRP == responseCode)/*AXIVION Line MisraC2012-2.2, MisraC2012-14.3  : Defensive coding */
        )
    {
        // Copy DID and set return data length.
        // Note: Some unusual DIDs may append further data after pending.
        Uds_CtrlBlk.resp_a[(uint8_t)UDS_SID_LEN + (uint8_t)RB_UDS_DID_MSB_OFFSET] = Req_a[RB_UDS_DID_MSB_OFFSET];/*AXIVION Line MisraC2012-2.2 :no issue here. Misra complaint is the (uint8_t) added before a enum 0(RB_UDS_DID_MSB_OFFSET). However, if not added, Misra warnings is also created due to arithmatic adding enums*/
        Uds_CtrlBlk.resp_a[(uint8_t)UDS_SID_LEN + (uint8_t)RB_UDS_DID_LSB_OFFSET] = Req_a[RB_UDS_DID_LSB_OFFSET];
        Uds_CtrlBlk.respLen                                     = (uint16_t) ((uint16_t)RB_UDS_DID_HDR_LEN + respLen);
    }
    else
    {
        Uds_CtrlBlk.respLen = 0;
    }

    return responseCode;
}

#endif // UDS_CFG_CORE_WRITE_DID_ENABLE

//----------------------------------------------------------------------------
//! \brief Handle routine control.
//!
//! Common handling for all RIDs
//! - Extract RID.
//! - Call handler for RID.
//! - Pack common response.
//!
//! \pre
//!     None.
//!
//! \post
//!     Control block contains response data.
//!
//! \return
//!     None.
//----------------------------------------------------------------------------
static uint8_t Uds_CoreRc(
    uint8_t     const   * const Req_a,
    uint16_t    const           ReqLen
    )
{
    uint16_t    respLen         = 0;
    uint8_t     responseCode;

    uint8_t     const   subFn   = Uds_CoreExtractSubFn(Req_a[RB_UDS_ROUTINE_SUB_FN_OFFSET]);    // PRQA S 0506
    // QAC Msg(2:0506) Dereferencing pointer value that is possibly NULL. AE Action: (mandatory) If defect: code must be changed (see AE-addition). If warning: code must be changed if better solution available. Otherwise, do not deactivate warning.
    // wy12cl Library will not cannot pass NULL.
    uint16_t    const   rid     = rbUds_Make16Bit(
                                        Req_a[RB_UDS_ROUTINE_RID_MSB_OFFSET],
                                        Req_a[RB_UDS_ROUTINE_RID_LSB_OFFSET]/*AXIVION Line MisraC2012-18.1 :Defensive Coding */
                                        );
    // Adjust lengths to remove the header
    Uds_CtrlBlk.req = &Req_a[RB_UDS_ROUTINE_HDR_LEN];/*AXIVION Line MisraC2012-18.1 :Defensive Coding */
    Uds_CtrlBlk.reqLen = ReqLen - (uint16_t)RB_UDS_ROUTINE_HDR_LEN;//AXIVION Line MisraC2012Directive-4.1 : checked
    // Specific handler returns only its data.
    responseCode = rbUds_Routine(
                        &Uds_CtrlBlk.resp_a[(uint8_t)UDS_SID_LEN + (uint8_t)RB_UDS_ROUTINE_HDR_LEN],
                        &respLen,
                        Uds_CtrlBlk.req,
                        Uds_CtrlBlk.reqLen,
                        rid,
                        subFn
                        );
    if (    ((uint8_t) RB_UDS_14229_PR    == responseCode)
        ||  ((uint8_t) RB_UDS_14229_RCRRP == responseCode)
        )
    {
        // Copy sub-function and RID.
        Uds_CtrlBlk.resp_a[(uint8_t)UDS_SID_LEN + (uint8_t)RB_UDS_ROUTINE_SUB_FN_OFFSET]  = subFn;/*AXIVION Line MisraC2012-2.2 :no issue here. Misra complaint is the (uint8_t) added before a enum 0(RB_UDS_ROUTINE_SUB_FN_OFFSET). However, if not added, Misra warnings is also created due to arithmatic adding enums*/
        Uds_CtrlBlk.resp_a[(uint8_t)UDS_SID_LEN + (uint8_t)RB_UDS_ROUTINE_RID_MSB_OFFSET] = Req_a[RB_UDS_ROUTINE_RID_MSB_OFFSET];
        Uds_CtrlBlk.resp_a[(uint8_t)UDS_SID_LEN + (uint8_t)RB_UDS_ROUTINE_RID_LSB_OFFSET] = Req_a[RB_UDS_ROUTINE_RID_LSB_OFFSET];/*AXIVION Line MisraC2012-18.1 :Defensive Coding */
        // Set return data length.
        Uds_CtrlBlk.respLen = (uint16_t) ((uint16_t)RB_UDS_ROUTINE_HDR_LEN + respLen);/*AXIVION Line MisraC2012Directive-4.1 :Defensive coding*/
    }
    else
    {
        Uds_CtrlBlk.respLen = 0;
    }

    return responseCode;
}

//----------------------------------------------------------------------------
//! \brief Handle read DTC information.
//!
//! Common handling for all read DTCs
//! - Extract sub-function.
//! - Call handler for sub-function.
//! - Pack common response.
//!
//! \pre
//!     None.
//!
//! \post
//!     Control block contains response data.
//!
//! \return
//!     None.
//----------------------------------------------------------------------------
#if defined(RBD_PK_APP)
#if (1 == UDS_CFG_CORE_READ_DTC_ENABLE)

static uint8_t Uds_CoreRdtci(
    uint8_t     const   * const Req_a,
    uint16_t    const           ReqLen
    )
{
    uint16_t            respLen         = 0;
    uint8_t     const   subFn           = Uds_CoreExtractSubFn(Req_a[RB_UDS_SUB_FN_OFFSET]);    // PRQA S 0506
    // QAC Msg(2:0506) Dereferencing pointer value that is possibly NULL. AE Action: (mandatory) If defect: code must be changed (see AE-addition). If warning: code must be changed if better solution available. Otherwise, do not deactivate warning.
    // wy12cl Library will not cannot pass NULL.

    // Specific handler returns only its data.
    uint8_t     const   responseCode    = rbUds_DtcRead(
                                                &Uds_CtrlBlk.resp_a[(uint8_t)UDS_SID_LEN + (uint8_t)RB_UDS_SUBFN_ONLY_DATA_OFFSET],
                                                &respLen,
                                                &Req_a[RB_UDS_SUBFN_ONLY_DATA_OFFSET],
                                                (uint16_t) (ReqLen - (uint16_t)RB_UDS_SUBFN_ONLY_HDR_LEN),/*AXIVION Line MisraC2012Directive-4.1 :Defensive coding*/
                                                subFn
                                                );
    if ((uint8_t) RB_UDS_14229_PR == responseCode)
    {
        // Copy sub-function and set return data length.
        Uds_CtrlBlk.resp_a[(uint8_t)UDS_SID_LEN + (uint8_t)RB_UDS_SUB_FN_OFFSET]  = subFn;/*AXIVION Line MisraC2012-2.2 :no issue here. Misra complaint is the (uint8_t) added before a enum 0(RB_UDS_SUB_FN_OFFSET). However, if not added, Misra warnings is also created due to arithmatic adding enums*/
        Uds_CtrlBlk.respLen = (uint16_t) ((uint16_t)RB_UDS_SUBFN_ONLY_HDR_LEN + respLen);/*AXIVION Line MisraC2012Directive-4.1 :Defensive coding*/
    }
    else
    {
        Uds_CtrlBlk.respLen = 0;
    }

    return responseCode;
}

#endif // UDS_CFG_CORE_READ_DTC_ENABLE
#endif

//----------------------------------------------------------------------------
//! \brief Handle clear DTC information.
//!
//! Common handling for clear DTC.
//! - Extract group of DTC.
//! - Call handler.
//! - Pack common response.
//!
//! \pre
//!     None.
//!
//! \post
//!     Control block contains response data.
//!
//! \return
//!     None.
//----------------------------------------------------------------------------
#if defined(RBD_PK_APP)
#if (1 == UDS_CFG_CORE_CLEAR_DTC_ENABLE)

static uint8_t Uds_CoreCdtci(
    uint8_t     const   * const Req_a,
    uint16_t    const           ReqLen  // PRQA S 3206
    // QAC Msg(3:3206) The parameter 'ReqLen' is not used in this function. AE Action: Code must be changed.
    // wy12cl Not used but required for consistent interface.
    )
{
    uint8_t responseCode;

    if ((uint16_t) RB_UDS_CDTCI_HDR_LEN != ReqLen)
    {
        responseCode = RB_UDS_14229_IMLOIF;
    }
    else
    {
        uint32_t const groupOfDtc = rbUds_Make32Bit(
                                            0u,
                                            Req_a[RB_UDS_CDTCI_GODTC_HB_OFFSET],    // PRQA S 0506
                                            Req_a[RB_UDS_CDTCI_GODTC_MB_OFFSET],
                                            Req_a[RB_UDS_CDTCI_GODTC_LB_OFFSET]/*AXIVION Line MisraC2012-18.1 :Defensive Coding */
                                            // QAC Msg(2:0506) Dereferencing pointer value that is possibly NULL. AE Action: (mandatory) If defect: code must be changed (see AE-addition). If warning: code must be changed if better solution available. Otherwise, do not deactivate warning.
                                            // wy12cl Library will not cannot pass NULL.
                                            );
        #ifdef RBD_PK_APP
            responseCode = rbUds_DtcClearCfgClear(groupOfDtc);
        #endif
    }

    // No data ever returned in the response.
    Uds_CtrlBlk.respLen = 0;

    return responseCode;
}

#endif  // UDS_CFG_CORE_CLEAR_DTC_ENABLE
#endif
//----------------------------------------------------------------------------
//! \brief Handle diagnostic session control.
//!
//! Common handling for diagnostic session control.
//! - Extract session sub-fn.
//! - Call handler.
//! - Pack common response.
//!
//! \pre
//!     None.
//!
//! \post
//!     Control block contains response data.
//!
//! \return
//!     None.
//----------------------------------------------------------------------------
static uint8_t Uds_CoreDsc(
    uint8_t     const   * const Req_a,
    uint16_t    const           ReqLen  // PRQA S 3206
    // QAC Msg(3:3206) The parameter 'ReqLen' is not used in this function. AE Action: Code must be changed.
    // wy12cl Not used but required for consistent interface.
    )
{
    uint8_t responseCode;

    if ((uint16_t) 1 != ReqLen)
    {
        Uds_CtrlBlk.respLen = 0;
        responseCode        = RB_UDS_14229_IMLOIF;
    }
    else
    {
        uint16_t            respLen = (uint16_t) 0;
        uint8_t     const   subFn   = Uds_CoreExtractSubFn(Req_a[RB_UDS_SUB_FN_OFFSET]);    // PRQA S 0506
        // QAC Msg(2:0506) Dereferencing pointer value that is possibly NULL. AE Action: (mandatory) If defect: code must be changed (see AE-addition). If warning: code must be changed if better solution available. Otherwise, do not deactivate warning.
        // wy12cl Library will not cannot pass NULL.

        responseCode = rbUds_Session(
                            &Uds_CtrlBlk.resp_a[(uint8_t)UDS_SID_LEN + (uint8_t)RB_UDS_SUBFN_ONLY_DATA_OFFSET],
                            &respLen,
                            subFn
                            );
        // Sub-fn required for PR now or after pending.
        if (    ((uint8_t) RB_UDS_14229_PR    == responseCode)
            ||  ((uint8_t) RB_UDS_14229_RCRRP == responseCode)
            )
        {
            // Copy sub-function and set return data length.
            /*AXIVION Next Line MisraC2012-2.2 :no issue here. Misra complaint is the (uint8_t) added before a enum 0(RB_UDS_SUB_FN_OFFSET). However, if not added, Misra warnings is also created due to arithmatic adding enums*/
            Uds_CtrlBlk.resp_a[(uint8_t)UDS_SID_LEN + (uint8_t)RB_UDS_SUB_FN_OFFSET]  = subFn;/*AXIVION Line MisraC2012Directive-4.1 :Defensive coding*/
            Uds_CtrlBlk.respLen = (uint16_t) ((uint16_t)RB_UDS_SUBFN_ONLY_HDR_LEN + respLen);/*AXIVION Line MisraC2012Directive-4.1 :Defensive coding*//*AXIVION Line MisraC2012Directive-4.1 :Defensive coding*/
        }
        else
        {
            Uds_CtrlBlk.respLen = 0;
        }
    }

    return responseCode;
}

//----------------------------------------------------------------------------
//! \brief Tester Present.
//!
//! Common handling:
//! - Extract sub-fn.
//! - Handle sub-fn.
//! - Pack common response.
//!
//! \pre
//!     None.
//!
//! \post
//!     Control block contains response data.
//!
//! \return
//!     UDS response code.
//----------------------------------------------------------------------------
static uint8_t Uds_CoreTp(
    uint8_t     const   * const Req_a,
    uint16_t    const           ReqLen  // PRQA S 3206
    // QAC Msg(3:3206) The parameter 'ReqLen' is not used in this function. AE Action: Code must be changed.
    // wy12cl Not used but required for consistent interface.
    )
{
    uint8_t             responseCode;
    uint8_t     const   subFn           = Uds_CoreExtractSubFn(Req_a[RB_UDS_SUB_FN_OFFSET]);  // PRQA S 0506
    // QAC Msg(2:0506) Dereferencing pointer value that is possibly NULL. AE Action: (mandatory) If defect: code must be changed (see AE-addition). If warning: code must be changed if better solution available. Otherwise, do not deactivate warning.
    // wy12cl Library will not cannot pass NULL.

    if ((uint16_t) 1u != ReqLen)
    {
        Uds_CtrlBlk.respLen = 0;
        responseCode        = RB_UDS_14229_IMLOIF;
    }
    else if (0u != subFn)
    {
        Uds_CtrlBlk.respLen = 0;
        responseCode        = RB_UDS_14229_SFNS;
    }
    else
    {
        // Copy sub-fn and set return data length.
        Uds_CtrlBlk.resp_a[(uint8_t)UDS_SID_LEN + (uint8_t)RB_UDS_SUB_FN_OFFSET]  = subFn;/*AXIVION Line MisraC2012-2.2 :no issue here. Misra complaint is the (uint8_t) added before a enum 0(RB_UDS_SUB_FN_OFFSET). However, if not added, Misra warnings is also created due to arithmatic adding enums*/
        Uds_CtrlBlk.respLen                                     = (uint16_t) (RB_UDS_SUBFN_ONLY_HDR_LEN);
        responseCode = RB_UDS_14229_PR;
    }

    return responseCode;
}

//----------------------------------------------------------------------------
//! \brief Handle security access.
//!
//! Common handling for security.
//! - Extract session sub-fn.
//! - Call handler.
//! - Pack common response.
//!
//! \pre
//!     None.
//!
//! \post
//!     Control block contains response data.
//!
//! \return
//!     None.
//----------------------------------------------------------------------------
static uint8_t Uds_CoreSa(
    uint8_t     const   * const Req_a,
    uint16_t    const           ReqLen
    )
{
    uint16_t            respLen         = 0;
    uint8_t     const   subFn           = Uds_CoreExtractSubFn(Req_a[RB_UDS_SUB_FN_OFFSET]);    // PRQA S 0506
    // QAC Msg(2:0506) Dereferencing pointer value that is possibly NULL. AE Action: (mandatory) If defect: code must be changed (see AE-addition). If warning: code must be changed if better solution available. Otherwise, do not deactivate warning.
    // wy12cl Library will not cannot pass NULL.
    uint8_t     const   responseCode    = rbUds_Security(
                                                &Uds_CtrlBlk.resp_a[(uint8_t)UDS_SID_LEN + (uint8_t)RB_UDS_SUBFN_ONLY_DATA_OFFSET],
                                                &respLen,
                                                &Req_a[RB_UDS_SUBFN_ONLY_DATA_OFFSET],
                                                (uint16_t) (ReqLen - (uint16_t)RB_UDS_SUBFN_ONLY_HDR_LEN),/*AXIVION Line MisraC2012Directive-4.1 :Defensive coding*/
                                                subFn
                                                );
    // Sub-fn required for PR now or after pending.
    if (    ((uint8_t) RB_UDS_14229_PR    == responseCode)
        ||  ((uint8_t) RB_UDS_14229_RCRRP == responseCode)
        )
    {
        // Copy sub-function and set return data length.
        Uds_CtrlBlk.resp_a[(uint8_t)UDS_SID_LEN + (uint8_t)RB_UDS_SUB_FN_OFFSET]  = subFn;/*AXIVION Line MisraC2012-2.2 :no issue here. Misra complaint is the (uint8_t) added before a enum 0(RB_UDS_SUB_FN_OFFSET). However, if not added, Misra warnings is also created due to arithmatic adding enums*/
        Uds_CtrlBlk.respLen = (uint16_t) ((uint16_t)RB_UDS_SUBFN_ONLY_HDR_LEN + respLen);/*AXIVION Line MisraC2012Directive-4.1 :Defensive coding*/
    }
    else
    {
        Uds_CtrlBlk.respLen = 0;
    }

    return responseCode;
}

#if (1 == UDS_CFG_CORE_UPLOAD_DOWNLOAD_ENABLE)

//----------------------------------------------------------------------------
//! \brief Handle request download.
//!
//! Common handling request download.
//! - Extract mandatory parameters.
//! - Call handler.
//! - Pack common response.
//!
//! \pre
//!     None.
//!
//! \post
//!     None.
//!
//! \return
//!     None.
//----------------------------------------------------------------------------
static uint8_t Uds_CoreRd(
    uint8_t     const   * const Req_a,
    uint16_t    const           ReqLen
    )
{
    uint8_t const   dataFormatId        = Req_a[RB_UDS_DATA_FORMAT_OFFSET]; // PRQA S 0506
    // QAC Msg(2:0506) Dereferencing pointer value that is possibly NULL. AE Action: (mandatory) If defect: code must be changed (see AE-addition). If warning: code must be changed if better solution available. Otherwise, do not deactivate warning.
    // wy12cl Library will not cannot pass NULL.
    uint8_t const   addrAndLenFormatId  = Req_a[RB_UDS_ADDR_LEN_FORMAT_OFFSET];
    uint8_t         responseCode        = rbUds_RequestDownloadCfg(
                                                &Req_a[RB_UDS_MEM_ADDR_OFFSET],/*AXIVION Line MisraC2012-18.1 :Defensive Coding */
                                                ReqLen - (uint16_t)RB_UDS_MEM_ADDR_OFFSET,/*AXIVION Line MisraC2012Directive-4.1 :Defensive coding*/
                                                dataFormatId,
                                                addrAndLenFormatId
                                                );
    if ((uint8_t) RB_UDS_14229_PR == responseCode)/*AXIVION Line MisraC2012-2.2, MisraC2012-14.3  : Defensive coding */
    {
        // Programming error for DV5162 Nissan 3ch Gateway.
        // This should only return RCRRP (e.g. erase in progress) or NRC.
        responseCode = RB_UDS_14229_GR;
    }

    Uds_CtrlBlk.respLen = 0;

    return responseCode;
}

//----------------------------------------------------------------------------
//! \brief Handle transfer data.
//!
//! Common handling transfer data.
//! - Extract mandatory parameters.
//! - Call handler.
//! - Pack common response.
//!
//! \pre
//!     None.
//!
//! \post
//!     None.
//!
//! \return
//!     None.
//----------------------------------------------------------------------------
static uint8_t Uds_CoreTd(
    uint8_t     const   * const Req_a,
    uint16_t    const           ReqLen
    )
{
    uint16_t            respLen         = 0;
    uint8_t     const   blockSequence   = Req_a[RB_UDS_BLOCK_SEQ_COUNTER_OFFSET];   // PRQA S 0506
    // QAC Msg(2:0506) Dereferencing pointer value that is possibly NULL. AE Action: (mandatory) If defect: code must be changed (see AE-addition). If warning: code must be changed if better solution available. Otherwise, do not deactivate warning.
    // wy12cl Library will not cannot pass NULL.
    uint8_t     const   responseCode    = rbUds_TransferDataCfg(
                                                &Uds_CtrlBlk.resp_a[UDS_SID_LEN],
                                                &respLen,
                                                &Req_a[RB_UDS_TRANSFER_DATA_HDR_LEN],
                                                ReqLen - (uint16_t)RB_UDS_TRANSFER_DATA_HDR_LEN,/*AXIVION Line MisraC2012Directive-4.1 :Defensive coding*/
                                                blockSequence
                                                );
    if ((uint8_t) RB_UDS_14229_PR == responseCode)
    {
        // Transfer data can respond immediately or RCRRP..
        // Note: It needs to push back the block sequence counter. (Limitation of pending handlign).
        Uds_CtrlBlk.respLen = respLen;
    }
    else
    {
        Uds_CtrlBlk.respLen = 0;
    }

    return responseCode;
}

//----------------------------------------------------------------------------
//! \brief Handle request transfer exit.
//!
//! Common handling transfer data.
//! - Call handler.
//! - Pack common response.
//!
//! \pre
//!     None.
//!
//! \post
//!     None.
//!
//! \return
//!     None.
//----------------------------------------------------------------------------
static uint8_t Uds_CoreRte(
    uint8_t     const   * const Req_a,
    uint16_t    const           ReqLen
    )
{
    uint16_t            respLen         = 0;
    uint8_t     const   responseCode    = rbUds_TransferExitCfg(
                                                &Uds_CtrlBlk.resp_a[UDS_SID_LEN],
                                                &respLen,
                                                &Req_a[RB_UDS_REQ_TRANSFER_EXIT_HDR_LEN],   // PRQA S 0506
                                                ReqLen - (uint16_t)RB_UDS_REQ_TRANSFER_EXIT_HDR_LEN
                                                // QAC Msg(2:0506) Dereferencing pointer value that is possibly NULL. AE Action: (mandatory) If defect: code must be changed (see AE-addition). If warning: code must be changed if better solution available. Otherwise, do not deactivate warning.
                                                // wy12cl Library will not cannot pass NULL.
                                                );
    if ((uint8_t) RB_UDS_14229_PR == responseCode)/*AXIVION Line MisraC2012-2.2, MisraC2012-14.3  : Defensive coding */
    {
        // Transfer exit can respond immediately or RCRRP.
        Uds_CtrlBlk.respLen = respLen;
    }
    else
    {
        Uds_CtrlBlk.respLen = 0;
    }

    return responseCode;
}

#endif // UDS_CFG_CORE_UPLOAD_DOWNLOAD_ENABLE

//----------------------------------------------------------------------------
//! \brief Handle ECU Reset.
//!
//! Common ECU reset handling.
//! - Extract sub-function.
//! - Call handler.
//! - Pack common response.
//!
//! \pre
//!     None.
//!
//! \post
//!     None.
//!
//! \return
//!     None.
//----------------------------------------------------------------------------
#if (1 == UDS_CFG_CORE_ECU_RESET_ENABLE)
static uint8_t Uds_CoreEr(
    uint8_t     const   * const Req_a,
    uint16_t    const           ReqLen  // PRQA S 3206
    // QAC Msg(3:3206) The parameter 'ReqLen' is not used in this function. AE Action: Code must be changed.
    // wy12cl Not used but required for consistent interface.
    )
{
    uint8_t             responseCode;
    uint8_t     const   subFn           = Uds_CoreExtractSubFn(Req_a[RB_UDS_SUB_FN_OFFSET]);    // PRQA S 0506
    // QAC Msg(2:0506) Dereferencing pointer value that is possibly NULL. AE Action: (mandatory) If defect: code must be changed (see AE-addition). If warning: code must be changed if better solution available. Otherwise, do not deactivate warning.
    // wy12cl Library will not pass NULL.

    if ((uint16_t) 1u != ReqLen)
    {
        Uds_CtrlBlk.respLen = 0u;
        responseCode        = RB_UDS_14229_IMLOIF;
    }
    else
    {
        responseCode = rbUds_EcuReset(
                            subFn
                            );
        if ((uint8_t) RB_UDS_14229_PR == responseCode)
        {
            // Copy sub-function and set return data length.
            Uds_CtrlBlk.resp_a[(uint8_t)UDS_SID_LEN + (uint8_t)RB_UDS_SUB_FN_OFFSET]  = subFn;/*AXIVION Line MisraC2012-2.2 :no issue here. Misra complaint is the (uint8_t) added before a enum 0(RB_UDS_SUB_FN_OFFSET). However, if not added, Misra warnings is also created due to arithmatic adding enums*/
            Uds_CtrlBlk.respLen                                     = (uint16_t) RB_UDS_SUBFN_ONLY_HDR_LEN;
        }
        else
        {
            Uds_CtrlBlk.respLen = 0;
        }
    }

    return responseCode;
}
#endif  // UDS_CFG_CORE_ECU_RESET_ENABLE

//----------------------------------------------------------------------------
//! \brief Get status of isBusy flag
//!
//!  This function is used to query if the uds core is currently processing a
//!  uds command
//!
//! \pre
//!     None.
//!
//! \post
//!     None
//!
//! \return
//!     isBusy status.
//----------------------------------------------------------------------------
bool Uds_CoreIsBusy(void)
{
    return Uds_CtrlBlk.isBusy;
}
//-------------------- End of File -------------------------------------------
