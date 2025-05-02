#ifndef RBDIMC_PRIVATE_H
#define RBDIMC_PRIVATE_H
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
//! \brief  Private include file for rbdImc
//----------------------------------------------------------------------------



//============================================================================
//  Includes
//============================================================================



//============================================================================
// Local Macros
//============================================================================



// Det service IDs
#define RBDIMCPDUR_IF_TX      1       //!< Classic transmit
#define RBDIMCPDUR_TP_TX      2       //!< TP transmit
#define RBDIMCPDUR_IF_RX      3       //!< Classic receive
#define RBDIMCPDUR_TP_RX      4       //!< TP receive
#define RBDIMCPDUR_IF_CANCEL  5       //!< Classic cancel
#define RBDIMCPDUR_TP_CANCEL  6       //!< TP cancel
#define RBDIMCCAN_SETMODE    10       //!< CAN driver set controller mode

// Det error codes
#define RBDIMCPDUR_ERR_NULL      1    //!< Null pointer
#define RBDIMCPDUR_ERR_PDU       2    //!< Invalid PduId
#define RBDIMCPDUR_ERR_LENGTH    3    //!< Invalid length
#define RBDIMCPDUR_ERR_SEQUENCE  4    //!< Invalid sequence number
#define RBDIMCPDUR_ERR_RESULT    5    //!< Unexpected return value
#define RBDIMCCAN_ERR_PARAM     10    //!< Parameter error

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


void rbdImc_TpCyclic( void ); /*AXIVION Line MisraC2012-8.6: correct prototype definition */
void rbdImc_PingCyclic( void ); /*AXIVION Line MisraC2012-8.6: correct prototype definition */

#endif // RBDIMC_PRIVATE_H
//-------------------- End of File -------------------------------------------
