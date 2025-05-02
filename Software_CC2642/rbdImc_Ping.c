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
//! \brief  Ping duration test for Imc
//----------------------------------------------------------------------------

#if defined(RBD_PK_CPU_SPC58ECx) && defined(RBD_PK_APP)
#define RBDIMC_WITH_RTE_APP
#endif

//============================================================================
//  Includes
//============================================================================

#include <string.h>     // For memcpy()
#include <assert.h>

#include "rbd_Pk.h"
#include "rbdImc.h"
#include "rbdImc_Private.h"
#include "ImcIf_Api.h"
#include "rbdMacros.h"

#include "rbdElTime_Api.h"
#include "rbUds_Iso14229.h"
#ifdef RBDIMC_WITH_RTE_APP
#include "Rte_rbdImc.h"
#endif
//============================================================================
// Local Macros
//============================================================================


// Define which channels to use

#if defined(RBD_PK_APP)
#if defined(RBD_PK_CPU_SPC58ECx)
    // MasterMcu can do a ping with the BLE chip
    #define RBDIMC_PING_TXMSG IMCIF_MSG_PING_BLE
#elif defined(RBD_PK_CPU_CC26xx)
    // BLE chip can respond to ping from MasterMcu, and initiate with UWB chip'
    #define RBDIMC_PING_RXMSG IMCIF_MSG_PING_BLE
    #define RBDIMC_PING_TXMSG IMCIF_MSG_PING_UWB
#elif defined(RBD_PK_CPU_S32)
    // S32 can initiate a ping with the UWB chip
    #define RBDIMC_PING_TXMSG IMCIF_MSG_PING_UWB
#elif defined(RBD_PK_CPU_NCJ29D5)
    // UWB chip only responds to a ping
    #define RBDIMC_PING_RXMSG IMCIF_MSG_PING_UWB
#else
    #error
#endif
#endif


//============================================================================
// Local Types
//============================================================================




//============================================================================
// Local Function Prototypes
//============================================================================




//============================================================================
// Global Variables
//============================================================================

// Union, to allow packing a timestamp into the message buffer
typedef union
{
    uint8              buf[4];
    rbd_ElTime_Value_t timestamp;
}
rbdImc_PingMsg_ut;


//============================================================================
// Module Variables
//============================================================================
#if defined RBDIMC_PING_TXMSG && (RBD_BUILD_TYPE == RBD_DEVELOPMENT)
static struct
{
    uint32  SumRTT;                 //!< Sum of the round-trip-time of all packets
    uint16  NumPackets;             //!< Number of packets to send
    uint16  NumTx;                  //!< Number of packets sent
    uint16  NumRx;                  //!< Number of packets received
    boolean running;                //!< Is the test running?
    uint8   STmin_ms;               //!< Minimum separation time, ms
    rbd_ElTime_rbTimer_st timer;    //!< Timer for packet separation
} rbdImc_Ping_s =
{
    .running = FALSE
};
#endif //defined RBDIMC_PING_TXMSG && (RBD_BUILD_TYPE == RBD_DEVELOPMENT)

//============================================================================
// Function declarations.
//============================================================================






#if defined(RBDIMC_PING_TXMSG) && (RBD_BUILD_TYPE == RBD_DEVELOPMENT)
//----------------------------------------------------------------------------
//! \brief  Start a ping test
//!
//! This function starts a test.
//!
//! \retval  0    for success
//! \retval  BRR  if a test is already running
//----------------------------------------------------------------------------
uint8 rbdImc_PingStart( uint8 const * const In_ReqData )
{
    uint8 result;

    if( rbdImc_Ping_s.running )
    {
        result = RB_UDS_14229_BRR;
    }
    else
    {
        rbdImc_Ping_s.NumPackets = ((uint16)In_ReqData[0u] << 8) + In_ReqData[1u];
        rbdImc_Ping_s.NumTx      = 0u;
        rbdImc_Ping_s.NumRx      = 0u;
        rbdImc_Ping_s.SumRTT     = 0u;
        rbd_ElTime_rbTimerInit( &rbdImc_Ping_s.timer );
        rbdImc_Ping_s.running    = TRUE;

        result = RB_UDS_14229_PR;
    }
    return result;
}

#ifdef RBDIMC_WITH_RTE_APP
FUNC(Std_ReturnType, rbdImc_CODE) rbdImc_ImcPingStart_Start(P2CONST(uint8, AUTOMATIC, RTE_RBDIMC_APPL_DATA) DcmDspStartRoutineInSignal,
                                                            Dcm_OpStatusType OpStatus,
                                                            P2VAR(uint8, AUTOMATIC, RTE_RBDIMC_APPL_VAR) DcmDspStartRoutineOutSignal,
                                                            P2VAR(uint16, AUTOMATIC, RTE_RBDIMC_APPL_VAR) DataLength,
                                                            P2VAR(Dcm_NegativeResponseCodeType, AUTOMATIC, RTE_RBDIMC_APPL_VAR) ErrorCode)
{
    uint8 ret = RTE_E_INVALID;
    RBD_UNUSED(OpStatus);
    RBD_UNUSED(DcmDspStartRoutineOutSignal);

    // Use the function above to parse the request
    *ErrorCode = rbdImc_PingStart(DcmDspStartRoutineInSignal);

    // Adjust the output to suit Dcm
    if( *ErrorCode == RB_UDS_14229_PR )
    {
        ret = RTE_E_OK;
    }
    *DataLength = 0u;
    return ret;
}
#endif // #ifndef RBDIMC_WITH_RTE_APP






//----------------------------------------------------------------------------
//! \brief  Store a uint16 into a buffer
//----------------------------------------------------------------------------
static void rbdImc_storeU16( uint8 * const buf, const uint16 value )
{
    buf[0] = (uint8)(value >>  8);
    buf[1] = (uint8)(value      );
}


//----------------------------------------------------------------------------
//! \brief  Return formatted results
//!
//! This function will write the results into buf, and set the length into len.
//!
//! \retval  0     OK
//! \retval  RSE   No results available (test was never started)
//----------------------------------------------------------------------------
uint8 rbdImc_PingResult( uint8 * const buf, uint16 * const len )
{
    uint8 result;

    if( 0u == rbdImc_Ping_s.NumPackets )
    {
        result = RB_UDS_14229_RSE;
    }
    else
    {
        uint16 RTT = 0u;

        if( rbdImc_Ping_s.NumRx > 0u )
        {
            RTT = RBD_ELTIME_KHZ_TICK_TO_MS(rbdImc_Ping_s.SumRTT / rbdImc_Ping_s.NumRx);
        }

        rbdImc_storeU16( &buf[0], rbdImc_Ping_s.NumTx );
        rbdImc_storeU16( &buf[2], rbdImc_Ping_s.NumRx );
        rbdImc_storeU16( &buf[4], RTT );
        *len = 6u;
        result = RB_UDS_14229_PR;
    }
    return result;
}

#ifdef RBDIMC_WITH_RTE_APP
FUNC(Std_ReturnType, rbdImc_CODE) rbdImc_ImcPingResult_Result(P2CONST(uint8, AUTOMATIC, RTE_RBDIMC_APPL_DATA) DcmDspRoutineRequestResInSignal,
                                                              Dcm_OpStatusType OpStatus,
                                                              P2VAR(uint8, AUTOMATIC, RTE_RBDIMC_APPL_VAR) DcmDspRoutineRequestResOutSignal,
                                                              P2VAR(uint16, AUTOMATIC, RTE_RBDIMC_APPL_VAR) DataLength,
                                                              P2VAR(Dcm_NegativeResponseCodeType, AUTOMATIC, RTE_RBDIMC_APPL_VAR) ErrorCode)
{
    uint8 ret = RTE_E_INVALID;
    uint16 len;
    RBD_UNUSED(OpStatus);
    RBD_UNUSED(DcmDspRoutineRequestResInSignal);

    // Use the function above to generate the response
    *ErrorCode = rbdImc_PingResult( DcmDspRoutineRequestResOutSignal, &len );

    // Adjust the output for Dcm
    if( *ErrorCode == RB_UDS_14229_PR )
    {
        ret = RTE_E_OK;
    }
    *DataLength = len;
    return ret;
}
#endif //#ifdef RBDIMC_WITH_RTE_APP
#endif


//----------------------------------------------------------------------------
//! \brief  Cyclic call of the Imc stack.
//!
//! Note: Functions are called starting with the lower layer, working to the
//! top. This enables buffers freed by a completed transmission/reception
//! to be reused for the next transmission.
//----------------------------------------------------------------------------
void rbdImc_PingCyclic( void )
{
#if defined RBDIMC_PING_TXMSG && (RBD_BUILD_TYPE == RBD_DEVELOPMENT)
    rbdImc_PingMsg_ut           msg;
    rbd_ElTime_rbTimerState_t   timerState;

    // Check the buffer size is correct
    static_assert( sizeof(msg.buf) == sizeof(msg.timestamp), "Timestamp must fit inside message buffer" );

    if( TRUE == rbdImc_Ping_s.running )
    {
        timerState = rbd_ElTime_rbTimerUpdate( &rbdImc_Ping_s.timer );

        if( rbdImc_Ping_s.NumTx >= rbdImc_Ping_s.NumPackets )
        {
            // Test finished
            rbdImc_Ping_s.running = FALSE;
        }
        else if( RBD_ELTIME_RBTIMER_STATE_RUNNING == timerState )
        {
            // Keep waiting
        }
        else
        {
            // Send the next packet
            // More packets to send
            // Send a message with the timestamp. Use memcpy() to avoid alignment issues on timestamp
            rbd_ElTime_GetCounterValue( RBD_ELTIME_CHANNEL_KHZ, &msg.timestamp);
            if( TRUE == ImcIf_Tx( RBDIMC_PING_TXMSG, sizeof(msg.buf), msg.buf ) )
            {
                rbdImc_Ping_s.NumTx++;

                // Start the timer for the next message
                if( rbdImc_Ping_s.STmin_ms > 0 )
                {
                    rbd_ElTime_rbTimerStart( &rbdImc_Ping_s.timer, rbdImc_Ping_s.STmin_ms );
                }
            }
        }
    }
#endif
}


//----------------------------------------------------------------------------
//! \brief  Message reception function
//!
//! As a ping responder, this function simply bounces the message back to sender.
//! As a ping initiator, this function has to count and time messages.
//----------------------------------------------------------------------------
void rbdImc_PingRxIndication(
        ImcIf_MsgType_et const   MsgId,
        rbdImc_Length_t  const   Length,
        uint8            const * const Msg,
        rbd_ElTime_Value_t const timestamp
        )
{
    RBD_UNUSED(timestamp);

#if (RBD_BUILD_TYPE == RBD_DEVELOPMENT)
#if defined(RBDIMC_PING_RXMSG)
    if( MsgId == RBDIMC_PING_RXMSG )
    {
        (void)ImcIf_Tx( RBDIMC_PING_RXMSG, Length, Msg );
    }
#else
    RBD_UNUSED(Length);
    RBD_UNUSED(Msg);
#endif


#if defined(RBDIMC_PING_TXMSG)
    if( MsgId == RBDIMC_PING_TXMSG )
    {
        rbdImc_PingMsg_ut  tempBuf;
        rbd_ElTime_Value_t now;

        rbd_ElTime_GetCounterValue( RBD_ELTIME_CHANNEL_KHZ, &now);

        // Use memcpy to avoid alignment issues
        memcpy( tempBuf.buf, Msg, sizeof(tempBuf.buf) );
        rbdImc_Ping_s.SumRTT += (now - tempBuf.timestamp);
        rbdImc_Ping_s.NumRx++;
    }
#else
#if !defined(RBDIMC_PING_RXMSG)
    RBD_UNUSED(MsgId);
#endif
#endif
#else
    RBD_UNUSED(Length);
    RBD_UNUSED(Msg);
    RBD_UNUSED(MsgId);
#endif
}

//-------------------- End of File -------------------------------------------
