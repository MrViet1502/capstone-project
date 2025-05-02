/*##############################################################################
#                                ROBERT BOSCH                                  #
#                            Melbourne,  AUSTRALIA                             #
###########################################################################*//**
@file
@brief CAN Interface.

 The CAN Interface module is located between the low level CAN device drivers
 and the upper communication service layers (i.e. CANSM, CANNM, CANTP, PDUR). It represents the
 interface to the services of the CAN Driver for the upper communication layers.

@rst
.. uml::

   component [PDU Router] as pdur
   component [CAN Transport Protocol] as cantp
   component [CAN State Manager] as cansm
   component [CAN Network Manager] as cannm
   component [CAN Driver] as candrv

   package "CAN IF" {
       component #GreenYellow [CAN Interface] as canif
       component #GreenYellow [CAN IF Callback] as canifcbk
       package "ECU Custom" {
           component [CAN IF Configuration] as canifcfg
       }
   }

   pdur ..> canif
   cantp ..> canif
   cansm ..> canif
   cannm ..> canif
   canifcfg <.left. canif
   canif ..> candrv
   canifcbk <.. candrv

@endrst

 The CAN Interface module provides a unique interface to manage different CAN
 hardware device types like CAN Controllers and CAN Transceivers used by the
 defined ECU hardware layout. Thus multiple underlying internal and external
 CAN Controllers/ CAN Transceivers can be controlled by the CAN State Managers
 module based on a physical CAN channel related view.



*//*############################################################################
# CONFIDENTIAL                                                                 #
#                                                                              #
# COPYRIGHT RESERVED, 2023 Robert Bosch (Australia) Pty Ltd. All rights        #
# reserved.  The reproduction, distribution and utilisation of this document   #
# as well as the communication of its contents to others without explicit      #
# authorisation is prohibited.  Offenders will be held liable for the payment  #
# of damages.  All rights reserved in the event of the grant of a patent,      #
# utility model or design.                                                     #
#                                                                              #
##############################################################################*/

/*==============================================================================

	INCLUDES

==============================================================================*/
#include <string.h>
#include "CanIf.h"
#include "CanIf_Cbk.h"
#include "CanIf_Private.h"
#include "CanSM_Cb.h"
#include "Det.h"
#include "rbdPwrVM.h"


//! @cond
/*==============================================================================

	DEFINES

==============================================================================*/


/*==============================================================================

	TYPES

==============================================================================*/


/*==============================================================================

	NATIONAL FUNCTION DECLARATIONS

==============================================================================*/


/*==============================================================================

	GLOBAL VARIABLE DEFINITIONS

==============================================================================*/


/*==============================================================================

	NATIONAL VARIABLE DEFINITIONS

==============================================================================*/


//! @endcond
/*==============================================================================

	GLOBAL FUNCTION DEFINITIONS

==============================================================================*/

/*-------------------------------------------------------------------------*//**
@brief Initialise CanIf Module.
 This service Initialises internal and external interfaces of the CAN Interface
 for the further processing.  The project must define a global variable
 CanIf_Config that holds the information and functions used by CanIf.

@returns Nil.
@rst
..  impl::
    :id: CanIf_Init
    :links_satisfies: CanIf_Init_1
@endrst
*//*--------------------------------------------------------------------------*/
void CanIf_Init( void )
{
    uint32 Id;

    for (Id = 0; Id < CanIf_Config.CanIfNumController; Id++)
    {
        CanIf_Config.CanIfControllerMode[Id] = CANIF_CS_UNINIT;
    }

    for( Id = 0; Id < CanIf_Config.CanIfNumChannels; Id++ )
    {
        CanIf_Config.CanIfTrcvMode[Id] = CANTRCV_TRCVMODE_SLEEP;        // There is no UNINIT value
    }

    CanIf_Config.CanIf_Cfg_SetAnchorCanId();

    // Clear pending-callback flags
    memset( CanIf_Config.CanIfPendingTxConfirm, FALSE, CanIf_Config.CanIfPduMax ); // AXIVION Line MisraC2012-21.18: checked
}

/*-------------------------------------------------------------------------*//**
@brief Update CAN IDs with new Anchor ID.
 This function is usually called during AnchorLearn when it needs to change the
 CAN ID it uses to broadcast this ECU's serial number.  Either
 CanId_vStoreAnchorId() or CanId_vStoreAnchorLearnId() should have been called
 before calling this function.  This function will then get the new IDs loaded
 into the mapping and filter tables.

@returns Nil.
@rst
..  impl::
    :id: CanIf_RefreshDynamicId
    :links_satisfies: CanIf_SetDynamicTxId_1
@endrst
*//*--------------------------------------------------------------------------*/
void CanIf_RefreshDynamicId( void )
{
    //update the mappng and filter tables
    CanIf_Config.CanIf_Cfg_SetAnchorCanId();
}


/*-------------------------------------------------------------------------*//**
@brief Transmit CAN PDU.
 This service requests transmission of a PDU, providing the supply voltage is
 within normal range.

@param[in] TxPduId    Identifier of the PDU to be transmitted.

@param[in] PduInfoPtr Length of and pointer to the PDU data and pointer to MetaData.

@retval  E_OK       Transmit request has been accepted.
@retval  E_NOT_OK   Transmit request has not been accepted.  Either the supply
 voltage is abnormal, or the CAN driver rejected the request.

 @rst
..  impl::
    :id: CanIf_Transmit
    :links_satisfies: CanIf_Transmit_1
@endrst
*//*--------------------------------------------------------------------------*/
Std_ReturnType CanIf_Transmit( PduIdType TxPduId, const PduInfoType *PduInfoPtr) /*AXIVION Line MisraC2012-8.13 :Coding has been done according AUTOSAR standard , it will impact the functionality if its changed. */
{
    Std_ReturnType RetValue = E_NOT_OK;

#if (CANIF_DEV_ERROR_DETECT == STD_ON)
    if ((PduIdType)CanIf_Config.CanIfPduMax <= TxPduId) /*Misra Fix : MisraC2012-10.4 - By typecasting*/
    {
        (void)Det_ReportError( DET_MODULEID_CANIF, 0, CANIF_API_TRANSMIT, CANIF_ERR_PARAM_PDUID ); /*Misra fix :MisraC2012Directive-4.7,MisraC2012-17.7 - By typecasting */
    }
    else if (NULL_PTR == PduInfoPtr)
    {
       (void)Det_ReportError( DET_MODULEID_CANIF, 0, CANIF_API_TRANSMIT, CANIF_ERR_PARAM_POINTER ); /*Misra fix :MisraC2012Directive-4.7,MisraC2012-17.7 - By typecasting */
    }
    else
#endif
    {
        // SysRS 7509/7498 there should be no CAN transmission error when voltage is under or over
        if (rbdPwrVM_ReadVoltageRange() != RBDPWRVM_NORMAL)/*AXIVION Line MisraC2012-2.2, MisraC2012-14.3 : safety code, no issue */
        {/*AXIVION Line MisraC2012-2.1 : safety code, no issue */
#if (CANIF_DEV_ERROR_DETECT == STD_ON)
            (void)Det_ReportError( DET_MODULEID_CANIF, 0, CANIF_API_TRANSMIT, CANIF_ERR_PARAM_VOLTAGE ); /*Misra fix :MisraC2012Directive-4.7,MisraC2012-17.7 - By typecasting */
#endif
        }
        else if (CANIF_CS_STARTED == CanIf_Config.CanIfControllerMode[CanIf_Config.CanIfCtrlId])
        {
            Can_ReturnType Ret;
            Can_PduType CanPdu;

            CanPdu.swPduHandle = TxPduId;
            CanPdu.id = CanIf_Config.CanIfPduCanIdMappingCfg[TxPduId];
            CanPdu.length = (uint8)PduInfoPtr->SduLength;
            CanPdu.sdu = PduInfoPtr->SduDataPtr;

            Ret = CanIf_Config.CanTrcv_Write( CanIf_Config.CanIfPduHandleCfg[TxPduId].LoHwHandle, &CanPdu );

            //busy?  Nothing to do.
            if (CAN_BUSY == Ret)  // AXIVION Line MisraC2012-2.2, MisraC2012-14.3: checked
            {
                ; /** Do nothing */
            }
            else if (CAN_NOT_OK == Ret)
            {
#if (CANIF_DEV_ERROR_DETECT == STD_ON)
               (void)Det_ReportError( DET_MODULEID_CANIF, 0, CANIF_API_TRANSMIT, CANIF_ERR_TRANSMIT_FAILED ); /*Misra fix :MisraC2012Directive-4.7,MisraC2012-17.7 - By typecasting */
#endif
            }
            else
            {
                // Flag that a confirmation is pending.
                // This is used to fix a sequencing error around TP messages
                // See ALM open point 410391
                CanIf_Config.CanIfPendingTxConfirm[TxPduId] = (boolean)TRUE;

                RetValue = E_OK;
            }
        }
        else
        {
            //do nothing
        }
    }

    return RetValue;
}


/*-------------------------------------------------------------------------*//**
@brief Set Controller Mode.
 This service calls the corresponding CAN Driver service for changing of the
 CAN controller mode.

@param[in] ControllerId   Abstracted CanIf ControllerId which is assigned to a
 CAN controller, which is requested for mode transition.

@param[in] ControllerMode Requested mode transition

@retval E_OK       Controller mode request has been accepted.
@retval E_NOT_OK   Controller mode request has not been accepted.

@rst
..  impl::
    :id: CanIf_SetControllerMode
    :links_satisfies: CanIf_SetControllerMode_1
@endrst
*//*--------------------------------------------------------------------------*/
Std_ReturnType CanIf_SetControllerMode( uint8 ControllerId, CanIf_ControllerModeType ControllerMode )
{
    Std_ReturnType RetValue = E_NOT_OK; /* MISRA Fix : MisraC2012-10.3 */

#if (CANIF_DEV_ERROR_DETECT == STD_ON)
    if (CanIf_Config.CanIfNumController <= ControllerId)
    {
        (void)Det_ReportError( DET_MODULEID_CANIF, 0, CANIF_API_SET_CTRL_MODE, CANIF_ERR_PARAM_CTRLID ); /*Misra fix :MisraC2012Directive-4.7,MisraC2012-17.7 - By typecasting */
    }
    else if ((ControllerMode != CANIF_CS_SLEEP) &&
             (ControllerMode != CANIF_CS_STARTED) &&
             (ControllerMode != CANIF_CS_WAKEUP) &&
             (ControllerMode != CANIF_CS_STOPPED))  /*Misra fix : MisraC2012-12.1 - Enclosed in Parentheses*/
    {
       (void)Det_ReportError( DET_MODULEID_CANIF, ControllerId, CANIF_API_SET_CTRL_MODE, CANIF_ERR_PARAM_CTRLMODE ); /*Misra fix :MisraC2012Directive-4.7,MisraC2012-17.7 - By typecasting */
    }
    else
#endif
    {
        Can_ReturnType Ret;
        Can_StateTransitionType Transition = CAN_T_STOP;

        if (CANIF_CS_STARTED == ControllerMode) // AXIVION Line MisraC2012-2.2, MisraC2012-14.3: checked
        {
            Transition = CAN_T_START;
        }
        else if (CANIF_CS_SLEEP == ControllerMode)
        {
            Transition = CAN_T_SLEEP;
        }
        else if (CANIF_CS_WAKEUP == ControllerMode)
        {
            Transition = CAN_T_WAKEUP;
        }
        else
        {
           ; //Do nothing
        }
        Ret = CanIf_Config.CanTrcv_SetControllerMode( ControllerId, Transition );

        if (CAN_OK == Ret)
        {
            RetValue = E_OK;
        }
        else if (CAN_BUSY == Ret) // AXIVION Line MisraC2012-2.2, MisraC2012-14.3: checked
        {
            ; // Do nothing
        }
        else
        {
#if (CANIF_DEV_ERROR_DETECT == STD_ON)
          (void)Det_ReportError( DET_MODULEID_CANIF, 0, CANIF_API_SET_CTRL_MODE, CANIF_ERR_MCAN_TRANSITION ); /*Misra fix :MisraC2012Directive-4.7,MisraC2012-17.7 - By typecasting */
#endif
        }
    }

    return RetValue;
}


/*-------------------------------------------------------------------------*//**
@brief Get Controller Mode.
 This service calls the corresponding CAN Driver service for obtaining the
 current status of the CAN controller.

@param[in] ControllerId   Abstracted CanIf ControllerId which is assigned to a
 CAN controller, which is requested for mode transition.

@param[out] ControllerModePtr  Pointer to a memory location, where the current
 mode of the CAN controller will be stored.

@retval E_OK Controller mode value is valid.
@retval E_NOT_OK Controller mode value is invalid.

@rst
..  impl::
    :id: CanIf_GetControllerMode
    :links_satisfies: CanIf_GetControllerMode_1
@endrst
*//*--------------------------------------------------------------------------*/
Std_ReturnType CanIf_GetControllerMode( uint8 ControllerId, CanIf_ControllerModeType *ControllerModePtr )
{
    Std_ReturnType RetValue = E_NOT_OK; /* MISRA Fix : MisraC2012-10.3 */

#if (CANIF_DEV_ERROR_DETECT == STD_ON)
    if (CanIf_Config.CanIfNumController <= ControllerId)
    {
        (void)Det_ReportError( DET_MODULEID_CANIF, 0, CANIF_API_GET_CTRL_MODE, CANIF_ERR_PARAM_CTRLID );/*Misra fix :MisraC2012Directive-4.7,MisraC2012-17.7 - By typecasting */
    }
    else if (NULL_PTR == ControllerModePtr)
    {
        (void)Det_ReportError( DET_MODULEID_CANIF, ControllerId, CANIF_API_GET_CTRL_MODE, CANIF_ERR_PARAM_POINTER );/*Misra fix :MisraC2012Directive-4.7,MisraC2012-17.7 - By typecasting */
    }
    else
#endif
    {
        *ControllerModePtr = CanIf_Config.CanIfControllerMode[ControllerId];
        RetValue = E_OK;
    }

    return RetValue;
}


/*-------------------------------------------------------------------------*//**
@brief Set Transceiver Mode.
 This service changes the operation mode of the tansceiver TransceiverId, via
 calling the corresponding CAN Transceiver Driver service.

@param[in] TransceiverId   Abstracted CanIf TransceiverId, which is assigned
 to a CAN transceiver, which is requested for mode transition.

@param[in] TransceiverMode Requested mode of requested network the Transceiver
 is connected to.

@retval E_OK       Transceiver mode request has been accepted.
@retval E_NOT_OK   Transceiver mode request has not been accepted.
@rst
..  impl::
    :id: CanIf_SetTrcvMode
    :links_satisfies: CanIf_SetTrcvMode_1
@endrst
*//*--------------------------------------------------------------------------*/
Std_ReturnType CanIf_SetTrcvMode(         uint8 TransceiverId, CanTrcv_TrcvModeType TransceiverMode )
{
    Std_ReturnType RetValue = E_NOT_OK;

#if (CANIF_DEV_ERROR_DETECT == STD_ON)
    if (CanIf_Config.CanIfNumChannels <= TransceiverId)
    {
        (void)Det_ReportError( DET_MODULEID_CANIF, 0, CANIF_API_SET_TRCV_MODE, CANIF_ERR_PARAM_TRCVID ); /*Misra fix :MisraC2012Directive-4.7,MisraC2012-17.7 - By typecasting */
    }
    else if ((TransceiverMode != CANTRCV_TRCVMODE_NORMAL) &&
             (TransceiverMode != CANTRCV_TRCVMODE_SLEEP) &&
             (TransceiverMode != CANTRCV_TRCVMODE_STANDBY))  /*Misra fix : MisraC2012-12.1 - Enclosed in Parentheses*/
    {
        (void)Det_ReportError( DET_MODULEID_CANIF, 0, CANIF_API_SET_TRCV_MODE, CANIF_ERR_PARAM_TRCVMODE ); /*Misra fix :MisraC2012Directive-4.7,MisraC2012-17.7 - By typecasting */
    }
    else
#endif
    {
        //set the new mode
        RetValue = CanIf_Config.CanTrcv_SetOpMode( TransceiverId, TransceiverMode );

        //was it set successfully?
        if (E_OK ==(Std_ReturnType) RetValue) /*Misra Fix : MisraC2012-10.4 -By typecasting*/
        {
            CanTrcv_TrcvModeType CanTrcv_Mode;

            //yes, so read it back
            RetValue = CanIf_Config.CanTrcv_GetOpMode( TransceiverId, &CanTrcv_Mode );

            //did we read it?
            if ( E_OK == (Std_ReturnType)RetValue) /* MISRA Fix : MisraC2012-10.4 - By typecasting*/
            {
                //does it not match what we requested?
                if (CanTrcv_Mode != TransceiverMode)
                {
                    //return error
                    RetValue = E_NOT_OK;
                }
            }
        }

#if (CANIF_DEV_ERROR_DETECT == STD_ON)
        if (E_NOT_OK == (Std_ReturnType) RetValue) /* MISRA Fix : MisraC2012-10.4 - By typecasting */
        {
            (void)Det_ReportError(DET_MODULEID_CANIF, 0, CANIF_API_SET_TRCV_MODE, CANIF_ERR_TRCV_TRANSITION); /*Misra fix :MisraC2012Directive-4.7,MisraC2012-17.7 - By typecasting */
        }
#endif
    }

    return RetValue;
}


/*-------------------------------------------------------------------------*//**
@brief Get Transceiver Mode.
 This function invokes CanTrcv_GetOpMode and updates the parameter
 TransceiverModePtr with the value OpMode provided by CanTrcv.

@param[in] TransceiverId  Abstracted CanIf TransceiverId, which is assigned to
 a CAN transceiver, which is requested for current operation mode.

@param[out] TransceiverModePtr  Requested mode of requested network the
 Transceiver is connected to.

@retval E_OK Transceiver mode value is valid.
@retval E_NOT_OK Transceiver mode value is invalid.

@rst
..  impl::
    :id: CanIf_GetTrcvMode
    :links_satisfies: CanIf_GetTrcvMode_1
@endrst
*//*--------------------------------------------------------------------------*/
Std_ReturnType CanIf_GetTrcvMode( uint8 TransceiverId, CanTrcv_TrcvModeType *TransceiverModePtr )
{
    Std_ReturnType RetValue = E_NOT_OK;

#if (CANIF_DEV_ERROR_DETECT == STD_ON)
    if (CanIf_Config.CanIfNumChannels <= TransceiverId)
    {
        (void)Det_ReportError( DET_MODULEID_CANIF, 0, CANIF_API_GET_TRCV_MODE, CANIF_ERR_PARAM_TRCVID ); /*Misra fix :MisraC2012Directive-4.7,MisraC2012-17.7 - By typecasting */
    }
    else if (NULL_PTR == TransceiverModePtr)
    {
        (void)Det_ReportError( DET_MODULEID_CANIF, 0, CANIF_API_GET_TRCV_MODE, CANIF_ERR_PARAM_POINTER ); /*Misra fix :MisraC2012Directive-4.7,MisraC2012-17.7 - By typecasting */
    }
    else
#endif
    {
        // This code assumes configured controller index matches transceiver index
        if( CANIF_CS_UNINIT != CanIf_Config.CanIfControllerMode[CanIf_Config.CanIfCtrlId] )
        {
            // Really, the controller mode is only valid if we have initialised the hardware
            RetValue = E_OK;
        }

        // We always return some kind of result, in case the caller doesn't check RetValue.
        *TransceiverModePtr = CanIf_Config.CanIfTrcvMode[TransceiverId];
    }

    return RetValue;
}


/*-------------------------------------------------------------------------*//**
@brief Indicate Controller Mode.
 This service indicates a controller state transition referring to the
 corresponding CAN controller with the abstract CanIf ControllerId.

@param[in] ControllerId    Abstract CanIf ControllerId which is assigned to a
 CAN controller, which state has been transitioned.

@param[in] ControllerMode  Mode to which the CAN controller transitioned

@returns Nil.
@rst
..  impl::
    :id: CanIf_ControllerModeIndication
    :links_satisfies: CanIf_ControllerModeIndication_1,AnchorBSW_Comms_CAN_1,AnchorBSW_Comms_Sleep_1
@endrst
*//*--------------------------------------------------------------------------*/
void CanIf_ControllerModeIndication( uint8 ControllerId, CanIf_ControllerModeType ControllerMode )
{
#if (CANIF_DEV_ERROR_DETECT == STD_ON)
    if (CanIf_Config.CanIfNumController <= ControllerId)
    {
        (void)Det_ReportError( DET_MODULEID_CANIF, 0, CANIF_API_CTRL_MODE_IND, CANIF_ERR_PARAM_CTRLID ); /*Misra fix :MisraC2012Directive-4.7,MisraC2012-17.7 - By typecasting */
    }
    else if ((ControllerMode != CANIF_CS_SLEEP) &&
             (ControllerMode != CANIF_CS_STARTED) &&
             (ControllerMode != CANIF_CS_STOPPED))   /*Misra fix : MisraC2012-12.1 */
    {
        (void)Det_ReportError( DET_MODULEID_CANIF, ControllerId, CANIF_API_CTRL_MODE_IND, CANIF_ERR_PARAM_CTRLMODE ); /*Misra fix :MisraC2012Directive-4.7,MisraC2012-17.7 - By typecasting */
    }
    else
#endif
    {
        CanIf_Config.CanIfControllerMode[ControllerId] = ControllerMode;
    }
}


/*-------------------------------------------------------------------------*//**
@brief Indicate Transceiver Mode.
 This service indicates a transceiver state transition referring to the
 corresponding CAN transceiver with the abstract CanIf TransceiverId.

@param[in] TransceiverId  Abstract CanIf TransceiverId, which is assigned to a
 CAN transceiver, which state has been transitioned.
@param[in] TransceiverMode  Mode to which the CAN transceiver transitioned

@returns Nil.
@rst
..  impl::
    :id: CanIf_TrcvModeIndication
    :links_satisfies: CanIf_TrcvModeIndication_1
@endrst
*//*--------------------------------------------------------------------------*/
void CanIf_TrcvModeIndication( uint8 TransceiverId, CanTrcv_TrcvModeType TransceiverMode )
{
#if (CANIF_DEV_ERROR_DETECT == STD_ON)
    if (TransceiverId >= CanIf_Config.CanIfNumChannels)
    {
        (void)Det_ReportError( DET_MODULEID_CANIF, 0, CANIF_API_TRCV_MODE_IND, CANIF_ERR_PARAM_TRCVID ); /*Misra fix :MisraC2012Directive-4.7,MisraC2012-17.7 - By typecasting */
    }
    else
#endif
    {
        CanIf_Config.CanIfTrcvMode[TransceiverId] = TransceiverMode;
    }
}


/*-------------------------------------------------------------------------*//**
@brief Indicate Controller BusOff Event.
 This service indicates a Controller BusOff event referring to the corresponding
 CAN Controller with the abstract CanIf ControllerId.

@param[in] ControllerId  Abstract CanIf ControllerId which is assigned to a
 CAN controller, which state has been transitioned.

@returns Nil.
@rst
..  impl::
    :id: CanIf_ControllerBusOff
    :links_satisfies: CanIf_ControllerBusOff_1
@endrst
*//*--------------------------------------------------------------------------*/
void CanIf_ControllerBusOff( uint8 ControllerId )
{
#if (CANIF_DEV_ERROR_DETECT == STD_ON)
    if (CanIf_Config.CanIfNumController <= ControllerId)
    {
       (void) Det_ReportError( DET_MODULEID_CANIF, 0, CANIF_API_CTRL_BUSOFF, CANIF_ERR_PARAM_CTRLID ); /*Misra fix :MisraC2012Directive-4.7,MisraC2012-17.7 - By typecasting */
    }
    else
#endif
    {
        /** Set CanIf controller mode to STOPPED */
        CanIf_Config.CanIfControllerMode[ControllerId] = CANIF_CS_STOPPED;

        /** Disable all the MCAN interrupts */
        if( CanIf_Config.CanTrcv_DisableAllInterrupts != NULL_PTR)
        {
            CanIf_Config.CanTrcv_DisableAllInterrupts( ControllerId );
        }

        /** Request transmit cancellation */
        if( CanIf_Config.CanTrcv_ClearTXBRP != NULL_PTR )
        {
            CanIf_Config.CanTrcv_ClearTXBRP( ControllerId );
        }

        /** Call User_ControllerBusOff() */
        CanSM_ControllerBusOff( ControllerId );
    }
}


/*==============================================================================

	NATIONAL FUNCTIONS

==============================================================================*/


