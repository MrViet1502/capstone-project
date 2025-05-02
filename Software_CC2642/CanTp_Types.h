/**********************************************************************************************************************
 * COPYRIGHT RESERVED, Robert Bosch GmbH, 2019. All rights reserved.
 * The reproduction, distribution and utilization of this document as well as the communication of its contents to
 * others without explicit authorization is prohibited. Offenders will be held liable for the payment of damages.
 * All rights reserved in the event of the grant of a patent, utility model or design.
 *
 **********************************************************************************************************************/
/*AXIVION Disable Style:MisraC2012--Directive-4.8:Data abstarction is not required */
#ifndef CANTP_TYPES_H
#define CANTP_TYPES_H

//============================================================================
// Include Files
//============================================================================
/* TRACE[SWS_CanTp_00209] */
#include "Os.h"
#include "ComStack_Types.h"


//============================================================================
// Version Check
//============================================================================
#if (!defined(COMTYPE_AR_RELEASE_MAJOR_VERSION) || (COMTYPE_AR_RELEASE_MAJOR_VERSION != 4))
#error "AUTOSAR major version undefined or mismatched"
#endif
#if (!defined(COMTYPE_AR_RELEASE_MINOR_VERSION) || (COMTYPE_AR_RELEASE_MINOR_VERSION != 2))
#error "AUTOSAR minor version undefined or mismatched"
#endif


//============================================================================
// Public Types
//============================================================================
/* [SWS_CanTp_00002][SWS_CanTp_00296] */
typedef TickType CanTp_TickType;
typedef uint8 CanTp_SduIdType;
typedef uint8 CanTp_ChannelIdType;
typedef void (*CanTp_GetSduPairType)(CanTp_SduIdType *TxSduId, CanTp_SduIdType *RxSduId, uint8 Address);

typedef struct CanTp_SharedFcStructType
{
    const PduIdType RxFcPduId;             /* Rx FC N-Pdu Id */
    const CanTp_SduIdType TxSduId;         /* Index to Tx Connection within CanTp_Connection */
} CanTp_SharedFcType;

/* Rx N-PDU to Connection Configuration Mapping Structure */
typedef struct CanTp_RxPduStructType
{
    const CanTp_SduIdType RxSduId;         /* RxSdu for combination of N-PDU and N-Ta */
    const CanTp_SduIdType TxSduId;         /* TxSdu for combination of N-PDU and N-Sa */
    const CanTp_GetSduPairType GetSduPair;
} CanTp_RxPduType;

/* TxConnection Configuration Structure */
typedef struct CanTp_TxSduStructType
{
    const uint8 TX_DL;                     /* Max CAN_DL of Tx */
    const uint8 BitFields;                 /* 4:IsTcDisabled|3:IsPadingOn|2:IsFunctional|1:IsFcDisabled|0:IsFdEnabled */
    const uint8 Address;                   /* Extended/Mixed Address to be used during transmission */
    const uint8 AddressFormatId;           /* Addressing Format Identifier */
    const uint8 TimeOutId;                 /* Timeout Index for this Connection in CanTp_TimeOut */
    const CanTp_ChannelIdType ChannelId;   /* Index of RAM Channel used by this connection */
    const PduIdType TxConfirmationId;      /* Transmit Confirmation Id for SF/FF/CF Frame */
    const PduIdType TxPduId;               /* Transmit N-PDU Id for this Connection */
    const PduIdType PduRPduHandleId;       /* Identifier to be used for PduR Api calls */
} CanTp_TxSduType;

/* RxConnection Configuration Structure */
typedef struct CanTp_RxSduStructType
{
    const uint8 BitFields;                 /* 3:IsPadingOn|2:IsFunctional|1:IsFcDisabled|0:IsFdEnabled */
    const uint8 Address;                   /* Extended/Mixed Address to be used during reception */
    const uint8 AddressFormatId;           /* Addressing Format Identifier */
    const uint8 TimeOutId;                 /* Timeout Index for this Connection in CanTp_TimeOut */
    const uint8 ParamId;                   /* Rx Conn. STmin/BS/FcWaitMax Index for this connection. */
    const CanTp_ChannelIdType ChannelId;   /* Index of RAM Channel used by this connection */
    const PduIdType TxConfirmationId;      /* Transmit Confirmation Id for FC Frame */
    const PduIdType TxPduId;               /* Transmit FC relevant N-PDU Id for this Connection */
    const PduIdType PduRPduHandleId;       /* Identifier to be used for PduR Api calls */
} CanTp_RxSduType;

/* TimeOut Configuration Structure */
typedef struct CanTp_TimeOutStructType
{
    const CanTp_TickType AsArTicks;        /* Time for transmission of the CAN frame (any N_PDU) on the sender/receiver side */
    const CanTp_TickType BsBrTicks;        /* Time until reception/transmission of the next FlowControl N_PDU */
    const CanTp_TickType CsCrTicks;        /* Time until transmission/reception of the next Consecutive Frame N_PDU */
} CanTp_TimeOutType;

/* RxConnection Specific left overs Configuration Structure */
typedef struct CanTp_ParamStructType
{
    const uint8 Param[2];                  /* Param[0] is for STMin and Param[1] is for BS */
    const uint16 FcWaitMax;                /* Maximum number of FC.WAIT frame transmissions */
} CanTp_ParamType;

/* CanTp Configuration Structure */
typedef struct CanTp_ConfigStructType
{
    const uint8 NumberOfSharedFc;          /* Number of shared FlowControl */
    const CanTp_ChannelIdType NumberOfChannels; /* Number of channels for CanTp communication */
    const CanTp_SduIdType NumberOfRxPdus;  /* Number of N-PDU for reception */
    const CanTp_SduIdType NumberOfTxPdus;  /* Number of N-PDU for transmission */
    const CanTp_SduIdType NumberOfRxSdus;  /* Number of N-SDU for reception */
    const CanTp_SduIdType NumberOfTxSdus;  /* Number of N-SDU for transmission */
    const CanTp_SharedFcType *SharedFc;    /* Shared FlowControl configuration */
    const CanTp_RxPduType *RxPdu;          /* N-PDU configuration for reception */
    const CanTp_TxSduType *TxSdu;          /* N-SDU configuration for transmission */
    const CanTp_RxSduType *RxSdu;          /* N-SDU configuration for reception */
    const CanTp_TimeOutType *TimeOut;      /* Timeout configuration */
    const CanTp_ParamType *Param;          /* FlowControl parameters */
} CanTp_ConfigType;


#endif /* CANTP_TYPES_H */
