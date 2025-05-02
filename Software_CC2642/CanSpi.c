/*******************************************************************************
*                                ROBERT BOSCH                                  *
*                            Melbourne,  AUSTRALIA                             *
********************************************************************************
   DESCRIPTION:
   This module is responsible for controlling initialisation of the TCAN4550.
   Normally it just ensures the chip is correctly reset, then polls to find when
   it's ready to be initialised before kicking off that activity.

   But in the background, it keeps polling to determine if the TCAN has had any
   faults like a low voltage lockout or if it has shutdown.  It will then take
   corrective action.

   FYI, the TCAN4550 is a TI chip, but it uses a Bosch M_CAN CAN module
   internally for CAN & CAN-FD.


********************************************************************************
* CONFIDENTIAL                                                                 *
*                                                                              *
* COPYRIGHT Robert Bosch (Australia) Pty. Ltd. 2020.                           *
* All rights reserved, also regarding any disposal, exploitation,              *
* reproduction, editing, distribution, as well as in the event of              *
* applications for industrial property rights.                                 *
*                                                                              *
*******************************************************************************/

/*==============================================================================

   INCLUDES

==============================================================================*/
#include <string.h>
#include "CanSpi_Api.h"
#include "CanIf.h"

#include "Det.h"
#include "Det_rbApi.h"
#include "Det_rbModuleId.h"
#include "dio.h"
#include "Dio.h"
#include "rbdElTime_Api.h"
#include "Can_44_TCAN4x5x.h"
#include "CanTrcv_44_TCAN4x5x.h"
#include "pt.h"
#include "Spi.h"
#include "rbdMacros.h"
#include "rbdPwrReq.h"
#include "CanSM.h"
#include "arm_acle.h"
#include "rbd_HwAb_Api.h"
#include "rbdPwrVM.h"
#include "WdgMgr.h"

// TI SDK
#include <ti/drivers/dpl/ClockP.h>
#include <ti/devices/cc13x2_cc26x2/driverlib/gpio.h>
#include <ti/devices/cc13x2_cc26x2/driverlib/cpu.h>

/*==============================================================================

   MISRA SUPRESSIONS

==============================================================================*/
// AXIVION Disable MisraC2012Directive-4.7: checked
// AXIVION Disable MisraC2012Directive-4.14: checked
// AXIVION Disable MisraC2012-10.1: checked
// AXIVION Disable MisraC2012-10.3: checked
// AXIVION Disable MisraC2012-10.4: checked
// AXIVION Disable MisraC2012-12.1: checked
// AXIVION Disable MisraC2012-13.1: checked
// AXIVION Disable MisraC2012-14.4: checked
// AXIVION Disable MisraC2012-15.5: checked
// AXIVION Disable MisraC2012-15.6: checked
// AXIVION Disable MisraC2012-17.7: checked
// AXIVION Disable MisraC2012-21.15: checked
// AXIVION Disable MisraC2012-21.16: checked
// AXIVION Disable Metric Metric-HIS.GOTO: pt is an accepted user of goto provided they are embedded within it's macros
// AXIVION Disable Metric Metric-HIS.LEVEL: pt macros are safe but induce a higher level metric
// AXIVION Disable Metric Metric-HIS.PATH: pt macros are safe but induce a higher level metric
// AXIVION Disable Metric Metric-HIS.STMT: pt macros are safe but induce a higher level metric
// AXIVION Disable Metric Metric-HIS.VG: pt macros are safe but induce a higher level metric


/*==============================================================================

   DEFINES

==============================================================================*/
//number of allowed retries
#define CANSPI_DEVID_RETRIES_MAX    100               //it _might_ take a few tries before we get the device ID
#define CANSPI_INIT_RETRIES_MAX     2                 //no point trying more than a couple of times

/* Delay after reset before attempting communication.  From the reference manual:
 "After a RST has taken place, a wait time of = 700 ?s should be used before
 reading or writing to the TCAN4550". */
#define CANSPI_RESET_DELAY_MS       2

//Error codes: API
#define CANSPI_API_INIT                  (1u)       //!< CanSpi_Init()
#define CANSPI_API_CYCLIC                (2u)       //!< CanSpi_Cyclic()

//Error codes: Values
#define CANSPI_ERR_STATE                 (1u)       //!< Invalid state
#define CANSPI_ERR_INIT                  (2u)       //!< Failed initialisation


/*==============================================================================

   TYPES

==============================================================================*/
typedef enum
{
    CANSPI_INITSTATE_PWRON,
    CANSPI_INITSTATE_INIT,
    CANSPI_INITSTATE_FAULT,
    CANSPI_INITSTATE_READY
}nCANSPIteState;

//TCAN register addresses
typedef enum
{
   //TI: Device ID and Interrupt/Diagnostic Flag Registers
   TCANRAkeDevID0       = 0x0000, //reply = "TCAN"
   TCANRAkeDevID1       = 0x0004, //reply = "4550"
   TCANRAkeTCANVer      = 0x0008,
   TCANRAkeTCANStatus   = 0x000C,

   //TI: Device configuration registers (M_CAN support)
   TCANRAkeModePinsCfg  = 0x0800, //Modes of Operation and Pin Configurations 0xC8000468 R/W/U
   TCANRAkePrescale     = 0x0804, //Timestamp Prescalar 0x00000002 R/W
   TCANRAkeTestReg      = 0x0808, //Read and Write Test Registers 0x00000000 R/W
   TCANRAkeEccTdr       = 0x080C, //ECC and TDR Registers 0x00000000 R/W/U
   TCANRAkeIrqFlags     = 0x0820, //Interrupt Flags 0x00000000 R (ref nTCAN4550tuIrqFlags)
   TCANRAkeMCANIrqFlags = 0x0824, //MCAN Interrupt Flags 0x00000000 R
   TCANRAkeIrqEnable    = 0x0830, //Interrupt Enable 0xFFFFFFFF R/W

   //Bosch: CAN-FD MCAN registers
   TCANRAkeCREL         = 0x1000, //Core Release Register rrrd dddd R
   TCANRAkeENDN         = 0x1004, //Endian Register 8765 4321 R
   TCANRAkeCUST         = 0x1008, //Customer Register 0000 0000 R
   TCANRAkeDBTP         = 0x100C, //Data Bit Timing & Prescaler Register 0000 0A33 RP
   TCANRAkeTEST         = 0x1010, //Test Register 0000 0000 RP
   TCANRAkeRWD          = 0x1014, //RAM Watchdog 0000 0000 RP
   TCANRAkeCCCR         = 0x1018, //CC Control Register 0000 0019 RWPp
   TCANRAkeNBTP         = 0x101C, //Nominal Bit Timing & Prescaler Register 0600 0A03 RP
   TCANRAkeTSCC         = 0x1020, //Timestamp Counter Configuration 0000 0000 RP
   TCANRAkeTSCV         = 0x1024, //Timestamp Counter Value 0000 0000 RC
   TCANRAkeTOCC         = 0x1028, //Timeout Counter Configuration FFFF 0000 RP
   TCANRAkeTOCV         = 0x102C, //Timeout Counter Value 0000 FFFF RC
   TCANRAkeECR          = 0x1040, //Error Counter Register 0000 0000 RX
   TCANRAkePSR          = 0x1044, //Protocol Status Register 0000 0707 RXS
   TCANRAkeTDCR         = 0x1048, //Transmitter Delay Compensation Register 0000 0000 RP
   TCANRAkeIR           = 0x1050, //Interrupt Register 0000 0000 RW
   TCANRAkeIE           = 0x1054, //Interrupt Enable 0000 0000 RW
   TCANRAkeILS          = 0x1058, //Interrupt Line Select 0000 0000 RW
   TCANRAkeILE          = 0x105C, //Interrupt Line Enable 0000 0000 RW
   TCANRAkeGFC          = 0x1080, //Global Filter Configuration 0000 0000 RP
   TCANRAkeSIDFC        = 0x1084, //Standard ID Filter Configuration 0000 0000 RP
   TCANRAkeXIDFC        = 0x1088, //Extended ID Filter Configuration 0000 0000 RP
   TCANRAkeXIDAM        = 0x1090, //Extended ID and MASK 1FFF FFFF RP
   TCANRAkeHPMS         = 0x1094, //High Priority Message Status 0000 0000 R
   TCANRAkeNDAT1        = 0x1098, //New Data 1 0000 0000 RW
   TCANRAkeNDAT2        = 0x109C, //New Data 2 0000 0000 RW
   TCANRAkeRXF0C        = 0x10A0, //Rx FIFO 0 Configuration 0000 0000 RP
   TCANRAkeRXF0S        = 0x10A4, //Rx FIFO 0 Status 0000 0000 R
   TCANRAkeRXF0A        = 0x10A8, //Rx FIFO 0 Acknowledge 0000 0000 RW
   TCANRAkeRXBC         = 0x10AC, //Rx Buffer Configuration 0000 0000 RP
   TCANRAkeRXF1C        = 0x10B0, //Rx FIFO 1 Configuration 0000 0000 RP
   TCANRAkeRXF1S        = 0x10B4, //Rx FIFO 1 Status 0000 0000 R
   TCANRAkeRXF1A        = 0x10B8, //Rx FIFO 1 Acknowledge 0000 0000 RW
   TCANRAkeRXESC        = 0x10BC, //Rx Buffer/FIFO Element Size Configuration 0000 0000 RP
   TCANRAkeTXBC         = 0x10C0, //Tx Buffer Configuration 0000 0000 RP
   TCANRAkeTXFQS        = 0x10C4, //Tx FIFO/Queue Status 0000 0000 R
   TCANRAkeTXESC        = 0x10C8, //Tx Buffer Element Size Configuration 0000 0000 RP
   TCANRAkeTXBRP        = 0x10CC, //Tx Buffer Request Pending 0000 0000 R
   TCANRAkeTXBAR        = 0x10D0, //Tx Buffer Add Request 0000 0000 RW
   TCANRAkeTXBCR        = 0x10D4, //Tx Buffer Cancellation Request 0000 0000 RW
   TCANRAkeTXBTO        = 0x10D8, //Tx Buffer Transmission Occurred 0000 0000 R
   TCANRAkeTXBCF        = 0x10DC, //Tx Buffer Cancellation Finished 0000 0000 R
   TCANRAkeTXBTIE       = 0x10E0, //Tx Buffer Transmission Interrupt Enable 0000 0000 RW
   TCANRAkeTXBCIE       = 0x10E4, //Tx Buffer Cancellation Finished Interrupt Enable 0000 0000 RW
   TCANRAkeTXEFC        = 0x10F0, //Tx Event FIFO Configuration 0000 0000 RP
   TCANRAkeTXEFS        = 0x10F4, //Tx Event FIFO Status 0000 0000 R
   TCANRAkeTXEFA        = 0x10F8, //Tx Event FIFO Acknowledge 0000 0000 RW
}nTCAN4550teRegAddr;

//TCAN command opcode
typedef enum
{
   TCANOPkeRead  = 0x41,
   TCANOPkeWrite = 0x61
}TCAN4550teOpCode;

//every command starts with this header
typedef struct __attribute__((__packed__))
{
   uint8_t /* TCAN4550teOpCode */ bOpCode;
   uint16_t wAddrBE;
   uint8_t bNofWords;
}nTCAN4550tsCommand;

//every reply starts with this header
typedef struct
{
   uint8_t bStatus;
   uint8_t abPadding[3];
}nTCAN4550tsRxHeader;

typedef union
{
   struct
   {
      uint8_t VTWD      : 1; //Global Voltage, Temp or WDTO
      uint8_t M_CAN_INT : 1; //M_CAN global INT
      uint8_t           : 1;
      uint8_t SPIERR    : 1; //SPI Error
      uint8_t           : 1;
      uint8_t CANERR    : 1; //CAN Error
      uint8_t WKRQ      : 1; //Wake Request
      uint8_t GLOBALERR : 1; //Global Error (Any Fault)
      uint8_t CANDOM    : 1; //CAN Stuck Dominant
      uint8_t           : 1;
      uint8_t CANSLNT   : 1; //CAN Silent
      uint8_t           : 2;
      uint8_t WKERR     : 1; //Wake Error
      uint8_t LWU       : 1; //Local Wake Up
      uint8_t CANINT    : 1; //Can Bus Wake Up Interrupt
      uint8_t ECCERR    : 1; //Uncorrectable ECC error detected
      uint8_t           : 1;
      uint8_t WDTO      : 1; //Watchdog Time Out
      uint8_t TSD       : 1; //Thermal Shutdown
      uint8_t PWRON     : 1; //Power ON
      uint8_t UVIO      : 1; //Under Voltage VIO
      uint8_t UVSUP     : 1; //Under Voltage VSUP and UVCCOUT
      uint8_t SMS       : 1; //Sleep Mode Status (Flag & Not an interrupt) Only sets when sleep mode is entered by a WKERR, UVIO timeout, or UVIO+TSD fault
      uint8_t           : 7;
      uint8_t CANBUSNOM : 1; //CAN Bus normal (Flag and Not Interrupt) Will change to 1 when in normal mode after first Dom to Rec transition
   };
   uint32_t lw;
}nTCAN4550tuIrqFlags;

typedef union
{
   struct
   {
      uint8_t REV_ID_MINOR;
      uint8_t REV_ID_MAJOR;
      uint8_t RSVD;
      uint8_t SPI_2_REVISION;
   };
   uint32_t lw;
}nTCAN4550tuTCANVer;

typedef union
{
   struct
   {
      uint8_t Day;

      uint8_t Month;

      uint8_t Year    : 4;
      uint8_t SubStep : 4;

      uint8_t Step    : 4;
      uint8_t Rel     : 4;
   };
   uint32_t lw;
}nTCAN4550tuMCANVer;

typedef union
{
   struct
   {
      uint32_t DSJW   : 4;
      uint32_t DTSEG2 : 4;
      uint32_t DTSEG1 : 5;
      uint32_t        : 3;
      uint32_t DBRP   : 5;
      uint32_t        : 2;
      uint32_t TDC    : 1;
      uint32_t        : 8;
   };
   uint32_t lw;
}nTCAN4550tuDBTP;

typedef union
{
   struct
   {
      uint32_t TEST_MODE_CONFIG  : 1;
      uint32_t SWE_DIS           : 1;
      uint32_t DEVICE_RESET      : 1;
      uint32_t WD_EN             : 1;
      uint32_t                   : 2;
      uint32_t MODE_SEL          : 2;
      uint32_t nWKRQ_CONFIG      : 1;
      uint32_t INH_DIS           : 1;
      uint32_t GPIO1_GPO_CONFIG  : 2;
      uint32_t                   : 1;
      uint32_t FAIL_SAFE_EN      : 1;
      uint32_t GPIO1_CONFIG      : 2;
      uint32_t WD_ACTION         : 2;
      uint32_t WD_BIT_SET        : 1;
      uint32_t nWKRQ_VOLTAGE     : 1;
      uint32_t                   : 1;
      uint32_t TEST_MODE_EN      : 1;
      uint32_t GPO2_CONFIG       : 2;
      uint32_t                   : 3;
      uint32_t CLK_REF           : 1;
      uint32_t WD_TIMER          : 2;
      uint32_t WAKE_CONFIG       : 2;
   };
   uint32_t lw;
}nTCAN4550tuModePinsCfg;

typedef enum
{
   nCANSPIkeFS_None,
   nCANSPIkeFS_Request,
   nCANSPIkeFS_Complete
}nCANSPIteFastSleep;


/*==============================================================================

   NATIONAL FUNCTION DECLARATIONS

==============================================================================*/
static char nCANSPI_cPwrOnInit( PT *pt, boolean oPartial );
static boolean nCANSPI_oInitTCAN( void );
static boolean nCANSPI_oInitTrcv( void );
static boolean nCANSPI_oSetTrcvNorm( void );
static boolean nCANSPI_oStartCtrl( void );
static char nCANSPI_cChkHealth( PT *pt, boolean *poRestart );
static char nCANSPI_cResetTCAN( PT *pt );
static char nCANSPI_cFastSleep( PT *pt );
static void nCANSPI_vSetUnavailable( nCANSPIteState eState );
static char nCANSPI_cCommsTest( PT *pt, boolean *poOk );
static char nCANSPI_cChkFaultFlags( PT *pt, boolean *poPwrOn, boolean *poThermErr );
static char nCANSPI_cQueryVersion( PT *pt );
static char nCANSPI_cTransceive( PT *pt, nTCAN4550teRegAddr eRegAddr, uint32_t lwTXData, uint32_t *plwRXData );


/*==============================================================================

   GLOBAL VARIABLE DEFINITIONS

==============================================================================*/


/*==============================================================================

   NATIONAL VARIABLE DEFINITIONS

==============================================================================*/
static PT nCANSPIsPT;
#ifndef DOXYGEN
PT_DEFINE_STACK( 32, nCANSPIsPTStack );
#endif

static uint32_t nCANSPIlwTimer;
static nCANSPIteState nCANSPIeState;
static uint8 nCANSPIbPwrReqID;
static nCANSPIteFastSleep nCANSPIeFastSleep = nCANSPIkeFS_None;


/*==============================================================================

   GLOBAL FUNCTION DEFINITIONS

==============================================================================*/

/*------------------------------------------------------------------------------
DESCRIPTION:
 Called at power on to initialise the CANSPI module, this function removes the
 reset assertion from the TCAN so it can start it's internal init.  It inits
 the CANSPI instance.
PARAMETERS:
  -> nil
RETURNS:
 <-  nil
------------------------------------------------------------------------------*/
void CanSpi_Init(void)
{
   //the reset PIN is taken HIGH (reset) at power on by the DIO driver, so we now take take it LOW (run)
   Dio_WriteChannel( DIOkeP_CAN_Reset, STD_LOW );

   //flag we're initialising
   nCANSPIeState = CANSPI_INITSTATE_PWRON;

   //init the thread
   PT_INIT( &nCANSPIsPT, nCANSPIsPTStack );

   //start the timer
   PT_DELAY_START( nCANSPIlwTimer );

   //get our unique ID from the power manager
   nCANSPIbPwrReqID = rbdPwrReq_GetID();

   //prevent sleep until we've initialised
   rbdPwrReq_RequestService( rbdPwrReq_Service_CPU, nCANSPIbPwrReqID );
}


/*------------------------------------------------------------------------------
DESCRIPTION:
 Called by various modules to confirm the availability of the TCAN chip.
PARAMETERS:
  -> nil
RETURNS:
 <-  TRUE if it is ready
------------------------------------------------------------------------------*/
bool CanSpi_CanHwReady( void )
{
   return( nCANSPIeState == CANSPI_INITSTATE_READY );
}


/*------------------------------------------------------------------------------
DESCRIPTION:
 This function is called to request the TCAN chip be put to sleep as part of the
 FastSleep UDS command.  The companion function CanSpi_ChkFastSleepDone() will
 return TRUE when the fast sleep sequence has been completed.
PARAMETERS:
  -> nil
RETURNS:
 <-  nil
------------------------------------------------------------------------------*/
void CanSpi_ReqFastSleep( void )
{
   //note the request
   nCANSPIeFastSleep = nCANSPIkeFS_Request;

   //ensure the CPU doesn't sleep until we're done
   rbdPwrReq_RequestService( rbdPwrReq_Service_CPU, nCANSPIbPwrReqID );
}


/*------------------------------------------------------------------------------
DESCRIPTION:
 Called by the shutdown module to find when the fast sleep routine has finished
 putting the TCAN to sleep.
PARAMETERS:
  -> nil
RETURNS:
 <-  TRUE if fast sleep has been completed.
------------------------------------------------------------------------------*/
boolean CanSpi_QryFastSleep( void )
{
    return (nCANSPIeFastSleep != nCANSPIkeFS_Request);
}


/*------------------------------------------------------------------------------
DESCRIPTION:
 The cyclic runs the initialisation code and then interrupt and health check
 polling.  If this function "exits", it's because TCAN required re-initialising.
PARAMETERS:
  -> nil
RETURNS:
 <-  nil
------------------------------------------------------------------------------*/
void CanSpi_Cyclic( void )
{
   PT *pt = &nCANSPIsPT;
   boolean oRestart;

   PT_BEGIN( pt );

   //=========================
   //== power on initialise ==
   //=========================
   //do TCAN power on init
   PT_GOSUB( pt, nCANSPI_cPwrOnInit( PT_SUB, FALSE ) );

   //did init fail?
   if( nCANSPIeState != CANSPI_INITSTATE_READY )
   {
      //reset the TCAN
      PT_GOSUB( pt, nCANSPI_cResetTCAN( PT_SUB ) );

      //exit - we'll restart the power on sequence
      PT_EXIT( pt );
   }


   //==================
   //== running loop ==
   //==================
   RBD_FOREVER
   {
      //start the timer
      PT_DELAY_START( nCANSPIlwTimer );

      //run the interrupt service
      RBD_FOREVER
      {
         //if an interrupt is still pending, re-trigger the task, so it can be processed further in the next call.
         if( Can_44_TCAN4x5x_HasPendingISR() ) /* AXIVION Line MisraC2012-14.4: checked */
         {
            Can_44_TCAN4x5x_Irq_Handler( CanIf_Config.CanIfTrcvId );
         }

         //let other cyclics run
         PT_YIELD( pt );

         //has the timer expired? yes, then exit the ISR loop, owise loop over and service the ISR
         if( PT_DELAY_CHK_EXPIRED( nCANSPIlwTimer, PT_MS_TO_TICKS( 50 ) ) ) break;

         //fast sleep requested? yes, then abort
         if( nCANSPIeFastSleep != nCANSPIkeFS_None ) break;
      }

      //abort runningloop if fast sleep requested
      if( nCANSPIeFastSleep != nCANSPIkeFS_None ) break;

      //if we're here, it's time for a TCAN health check
      PT_GOSUB( pt, nCANSPI_cChkHealth( PT_SUB, &oRestart ) );

      //need to restart?
      if( oRestart ) /* AXIVION Line MisraC2012-14.4: checked */
      {
         //yes, so reset the TCAN
         PT_GOSUB( pt, nCANSPI_cResetTCAN( PT_SUB ) );

         //exit - we'll restart the power on sequence
         PT_EXIT( pt );
      }
   }

   //has fast sleep been requested?
   if( nCANSPIeFastSleep != nCANSPIkeFS_None )
   {  //AXIVION Line MisraC2012-2.1: code is reachable
      //yes, so go do it
      PT_GOSUB( pt, nCANSPI_cFastSleep( PT_SUB ) );

      //report sleep is complete
      nCANSPIeFastSleep = nCANSPIkeFS_Complete;

      //and we stay here forever - only a reset will wake us from fastsleep
      PT_WAIT_WHILE( pt, TRUE );
   }


   PT_END( pt );
   PT_CHK_STACK( pt, nCANSPIsPTStack );
   PT_VOID_RETURN();
}


/*==============================================================================

   NATIONAL FUNCTIONS

==============================================================================*/

/*------------------------------------------------------------------------------
DESCRIPTION:
 This task performs either a full init of TCAN and our internal modules, or just
 a partial init which is just the internal modules.  The fill init is required
 after resetting the TCAN, while the driver only init is when TCAN has auto
 recovered from a low voltage or thermal fault.
 This function will try each init step several times.  If it fails the main
 cyclic will reset and try again.
PARAMETERS:
  -> TRUE if only a internal driver init is required.
RETURNS:
 <-  PT_ENDED when done
------------------------------------------------------------------------------*/
static char nCANSPI_cPwrOnInit( PT *pt, boolean oPartial )
{
   typedef boolean(* tf_oInit)(void);

   //initialisation functions, listed in call order, return TRUE if successful
   static const tf_oInit kaf_oInit[] =
   {
      nCANSPI_oInitTCAN,
      nCANSPI_oInitTrcv,

      //partial runs from here down...
      nCANSPI_oSetTrcvNorm,
      nCANSPI_oStartCtrl
   };

   uint32_t lwTries = PT_RECALL( pt, uint32_t );
   uint32_t lwIdx = PT_RECALL( pt, uint32_t );
   boolean oOk;
   rbdPwrVM_VoltageRange_et eVoltageState;

   PT_BEGIN( pt );

   //prevent sleep while we're initialising
   rbdPwrReq_RequestService( rbdPwrReq_Service_CPU, nCANSPIbPwrReqID );

   Det_RbLog( DET_MODULEID_CANSPI, "CANSPI:Initialising\r\n" );

   //now start a 5ms polling for the device ID, but fail if we're undervoltage
   for( lwTries = 0; lwTries < CANSPI_DEVID_RETRIES_MAX; lwTries++ )
   {
      //get battery state
      eVoltageState = rbdPwrVM_ReadVoltageRange();

      //undervoltage?
      if( eVoltageState == RBDPWRVM_UNDER_VOLTAGE )
      {
         //yes - so abort and go await recovery
         Det_RbLog( DET_MODULEID_CANSPI, "CANSPI:Undervoltage\r\n" );
         PT_EXIT( pt );
      }

      //query the TCAN device ID
      PT_GOSUB( pt, nCANSPI_cCommsTest( PT_SUB, &oOk ) );

      //is it ready? yes, so stop polling
      if( oOk ) break; /* AXIVION Line MisraC2012-14.4: checked */

      //owise waf 5ms and try again
      PT_DELAY( pt, nCANSPIlwTimer, PT_MS_TO_TICKS( 5 ) );
   }

   //abort if unsuccessful (we'll reset it and try again)
   if( lwTries == CANSPI_DEVID_RETRIES_MAX )
   {
      Det_RbLog( DET_MODULEID_CANSPI, "CANSPI:DeviceIDFail\r\n" );
      PT_EXIT( pt );
   }

   //amuse the developer with some TCAN version info if this is the first init after power on
   if( nCANSPIeState == CANSPI_INITSTATE_PWRON ) PT_GOSUB( pt, nCANSPI_cQueryVersion( PT_SUB ) );

   //flag we're initialising (we may have been in the pwron state)
   nCANSPIeState = CANSPI_INITSTATE_INIT;

   //loop through the init functions
   for( lwIdx = oPartial? 2 : 0; lwIdx < DIMOF( kaf_oInit ); lwIdx++ )
   {
      //give the init function a few attempts to succeed
      for( lwTries = 0; lwTries < CANSPI_INIT_RETRIES_MAX; lwTries++ )
      {
         //call the init function, was init successful?  yes, then stop the try loop
         if( kaf_oInit[lwIdx]() ) break; /* AXIVION Line MisraC2012-14.4: checked */

         //waf 2ms
         PT_DELAY( pt, nCANSPIlwTimer, PT_MS_TO_TICKS( 2 ) );
      }

      //was this init unsuccessful?
      if( lwTries == CANSPI_INIT_RETRIES_MAX )
      {
         //report the error
         Det_ReportError( DET_MODULEID_CANSPI, 0, CANSPI_API_CYCLIC, CANSPI_ERR_INIT );

         Det_RbLog( DET_MODULEID_CANSPI, "CANSPI:InitFailed @%u\r\n", lwIdx );

         //exit - this will cause a pin reset and re-init
         PT_EXIT( pt );
      }
   }

   //resume the TCAN dog, nice puppy
   WdgMgr_Resume( WDGMGRkeN_External );

   //if we're here, init succeeded, flag we're good to run, yay!
   nCANSPIeState = CANSPI_INITSTATE_READY;
   Det_RbLog( DET_MODULEID_CANSPI, "CANSPI:InitComplete\r\n" );

   //allow sleep now we're running
   rbdPwrReq_ReleaseService( rbdPwrReq_Service_CPU, nCANSPIbPwrReqID );

   PT_END( pt );
   PT_STORE( pt, lwIdx, uint32_t );
   PT_STORE( pt, lwTries, uint32_t );
   PT_RETURN();
}


/*------------------------------------------------------------------------------
DESCRIPTION:
 Initialisation functions.  Expect to be called multiple times until each
 returns success.
PARAMETERS:
  -> nil
RETURNS:
 <-  TRUE if init succeeded
------------------------------------------------------------------------------*/
static boolean nCANSPI_oInitTCAN( void )
{
   //go init the TCAN
   Can_44_TCAN4x5x_Init( &Can_44_TCAN4x5x_Config );

   //TRUE if init succeed
   return( Can_44_TCAN4x5x_DriverState == CAN_44_TCAN4x5x_READY );
}

static boolean nCANSPI_oInitTrcv( void )
{
   //init
   CanTrcv_44_TCAN4x5x_Init( &CanTrcv_44_TCAN4x5x_Config );

   //TRUE if init succeed
   return( CanTrcv_44_TCAN4x5x_TrcvInitState == CANTRCV_44_TCAN4x5x_INIT );
}

static boolean nCANSPI_oSetTrcvNorm( void )
{
   return( CanIf_SetTrcvMode( CanIf_Config.CanIfCtrlId, CANTRCV_TRCVMODE_NORMAL ) == E_OK );
}

static boolean nCANSPI_oStartCtrl( void )
{
   //ignore the result as CANIF _may_ have already been started by another module
   CanIf_SetControllerMode( CanIf_Config.CanIfCtrlId, CANIF_CS_STARTED );
   return( TRUE );
}


/*------------------------------------------------------------------------------
DESCRIPTION:
 Performs the high level determination on whether the TCAN is temporarily in a
 fault state (ie low voltage / high temperature) or whether it's reset(ing) and
 we need to restart it.
PARAMETERS:
  -> thread instance
 <-  TRUE if the TCAN has reset and thus needs to be fully restarted, FALSE if
     the cyclic can continue.
RETURNS:
 <-  PT_ENDED when done
------------------------------------------------------------------------------*/
static char nCANSPI_cChkHealth( PT *pt, boolean *poRestart )
{
   boolean oDevIDOk = PT_RECALL( pt, boolean );
   boolean oPwrOn = PT_RECALL( pt, boolean );
   boolean oThermErr = PT_RECALL( pt, boolean ); //therm fault

   PT_BEGIN( pt );

   oDevIDOk = FALSE;
   oPwrOn = FALSE;
   oThermErr = FALSE;

   //wait until CAN is awake
   PT_WAIT_UNTIL( pt, CanSM_GetCanHwStatus() == CANSM_CANHW_IN_WAKE );

   //prevent sleep while we're polling
   rbdPwrReq_RequestService( rbdPwrReq_Service_CPU, nCANSPIbPwrReqID );

   //re-check we can read the TCAN device ID
   PT_GOSUB( pt, nCANSPI_cCommsTest( PT_SUB, &oDevIDOk) );

   //was the device ID ok?
   if( oDevIDOk ) /* AXIVION Line MisraC2012-14.4: checked */
   {
      //yes, so as the TCAN is alive, go check it's other registers
      PT_GOSUB( pt, nCANSPI_cChkFaultFlags( PT_SUB, &oPwrOn, &oThermErr ) );
   }
   else
   {
      Det_RbLog( DET_MODULEID_CANSPI, "CANSPI:DeviceIDFail\r\n" );
   }

   //failed device ID OR seeing a power on?
   if( !oDevIDOk || oPwrOn )
   {
      //exit and redo init
      *poRestart = TRUE;
      PT_EXIT( pt );
   }

   //allow sleep now we're done polling
   rbdPwrReq_ReleaseService( rbdPwrReq_Service_CPU, nCANSPIbPwrReqID );

   //seeing a fault AND we're currently READY?  (ie - we're now in fault)
   if( oThermErr && (nCANSPIeState == CANSPI_INITSTATE_READY) )
   {
      //flag we're now unavailable
      nCANSPI_vSetUnavailable( CANSPI_INITSTATE_FAULT );
      Det_RbLog( DET_MODULEID_CANSPI, "CANSPI:UnderVolt/Therm fault\r\n" );
   }

   //NOT seeing a fault AND we're currently NOT READY?  (ie - we've recovered)
   if( !oThermErr && (nCANSPIeState != CANSPI_INITSTATE_READY) )
   {
      //we just need to perform a init of CanIf
      PT_GOSUB( pt, nCANSPI_cPwrOnInit( PT_SUB, TRUE ) );
      Det_RbLog( DET_MODULEID_CANSPI, "CANSPI:UnderVolt/Therm cleared\r\n" );
   }

   //restart not required
   *poRestart = FALSE;

   PT_END( pt );
   PT_STORE( pt, oThermErr, boolean );
   PT_STORE( pt, oPwrOn, boolean );
   PT_STORE( pt, oDevIDOk, boolean );
   PT_RETURN();
}


/*------------------------------------------------------------------------------
DESCRIPTION:
 Operates the TCAN reset pin and blocks power saving.
PARAMETERS:
  -> thread instance
RETURNS:
 <-  PT_ENDED when done
------------------------------------------------------------------------------*/
static char nCANSPI_cResetTCAN( PT *pt )
{
   #define mResetPeriod_us 40 //time to hold reset line active, in us
   uint32_t lwDelay;
   ClockP_FreqHz Freq_s;
   rbdPwrVM_VoltageRange_et eVoltageState;

   PT_BEGIN( pt );

   //prevent sleep while we're initialising
   rbdPwrReq_RequestService( rbdPwrReq_Service_CPU, nCANSPIbPwrReqID );

   //set the driver back to an uninit state
   Can_44_TCAN4x5x_DriverState = CAN_44_TCAN4x5x_UNINIT;
   CanTrcv_44_TCAN4x5x_TrcvInitState = CANTRCV_44_TCAN4x5x_UNINIT;

   //we need to set the controller back to stopped to block CAN coms
   nCANSPI_vSetUnavailable( CANSPI_INITSTATE_INIT );

   //stop the TCAN doggie
   WdgMgr_Stop( WDGMGRkeN_External );

   Det_RbLog( DET_MODULEID_CANSPI, "CANSPI:VoltageChk\r\n" );

   //waf the supply to be sensible
   RBD_FOREVER
   {
      //get battery state
      eVoltageState = rbdPwrVM_ReadVoltageRange();

      //not undervoltage? no, so it's safe to reset
      if( eVoltageState != RBDPWRVM_UNDER_VOLTAGE ) break;

      //owise waf 20ms before trying again
      PT_DELAY( pt, nCANSPIlwTimer, PT_MS_TO_TICKS( 20 ) );
   }

   Det_RbLog( DET_MODULEID_CANSPI, "CANSPI:ResetTCAN\r\n" );

   //assert the TCAN reset line (30us minimum)
   Dio_WriteChannel( DIOkeP_CAN_Reset, STD_HIGH );

   //get the CPU clock frequency
   ClockP_getCpuFreq( &Freq_s );

   //figure the delay (/3 as delay function is 3 cycles per loop)
   lwDelay = Freq_s.lo * mResetPeriod_us / 1000000 / 3;

   //do the delay (3 cycles per loop)
   CPUdelay( lwDelay );

   //release the reset PIN
   Dio_WriteChannel( DIOkeP_CAN_Reset, STD_LOW );

   //wait a further 2ms for the TCAN internal init to complete
   PT_YIELD_UNTIL( pt, PT_DELAY_CHK_EXPIRED( nCANSPIlwTimer, PT_MS_TO_TICKS( CANSPI_RESET_DELAY_MS ) ) );

   PT_END( pt );
   PT_RETURN();
}


/*------------------------------------------------------------------------------
DESCRIPTION:
 This thread allows us to get the TCAN to sleep quicky for the FastSleep
 feature.  Note TCAN has a tSILENCE timer that usually prevents the TCAN from
 dropping it's current consumption for 800ms even though the M_CAN reports it's
 sleeping.  The only way we've found to circumvent is to reset the TCAN and put
 it to sleep after reset.
PARAMETERS:
  -> thread instance
RETURNS:
 <-  PT_ENDED when done
------------------------------------------------------------------------------*/
static char nCANSPI_cFastSleep( PT *pt )
{
   uint32_t lwTries = PT_RECALL( pt, uint32_t );
   boolean oOk;

   PT_BEGIN( pt );

   Det_RbLog( DET_MODULEID_CANSPI, "CANSPI:FastSleep\r\n" );

   // Set CAN controller to stopped mode
   (void)CanIf_SetControllerMode( CanIf_Config.CanIfCtrlId, CANIF_CS_STOPPED );

   //this is how we bypass the TCAN TI tSILENCE timer - swing the Mjolnir and HARD reset the TCAN
   PT_GOSUB( pt, nCANSPI_cResetTCAN( PT_SUB ) );

   //now start a 5ms polling for the device ID so we know when the TCAN has completed resetting
   for( lwTries = 0; lwTries < CANSPI_DEVID_RETRIES_MAX; lwTries++ )
   {
      //query the TCAN device ID
      PT_GOSUB( pt, nCANSPI_cCommsTest( PT_SUB, &oOk ) );

      //is it ready? yes, so stop polling
      if( oOk ) break;

      //owise waf 5ms and try again
      PT_DELAY( pt, nCANSPIlwTimer, PT_MS_TO_TICKS( 5 ) );
   }

   //set CAN transceiver to sleep mode
   (void)CanIf_SetTrcvMode( CanIf_Config.CanIfTrcvId, CANTRCV_TRCVMODE_SLEEP );

   //set CAN controller to sleep mode
   (void)CanIf_SetControllerMode( CanIf_Config.CanIfCtrlId, CANIF_CS_SLEEP );

   #if 0 //only required for debugging - enable if you need to see when TCAN has actually gone to sleep
   //loop to detect when TCAN has gone to sleep
   RBD_FOREVER
   {
      //re-check we can read the TCAN device ID
      PT_GOSUB( pt, nCANSPI_cCommsTest( PT_SUB, &oOk) );

      if( !oOk ) break;

      PT_YIELD( pt );
   }

   Det_RbLog( DET_MODULEID_CANSPI, "CANSPI:TCAN unresponsive\r\n" );
   #endif

   //we can now allow our CPU to sleep
   rbdPwrReq_ReleaseService( rbdPwrReq_Service_CPU, nCANSPIbPwrReqID );

   PT_END( pt );
   PT_STORE( pt, lwTries, uint32_t );
   PT_RETURN();
}


/*------------------------------------------------------------------------------
DESCRIPTION:
 Any TCAN error will call this function to stop our CanIf from doing any further
 CAN comms.
PARAMETERS:
  -> new state (usually INIT)
RETURNS:
 <-  nil
------------------------------------------------------------------------------*/
static void nCANSPI_vSetUnavailable( nCANSPIteState eState )
{
   //set the new state
   nCANSPIeState = eState;

   //block CAN comms
   CanIf_SetControllerMode( 0, CANIF_CS_STOPPED );
   CanIf_ControllerBusOff( 0 );
}


/*-----------------------------------------------------------------------------
DESCRIPTION:
 Callled to query the TCAN to see if it has finished resetting and it now
 accepting SPI traffic.  It simply tries to read the device ID.
PARAMETERS:
  -> thread instance
 <-  TRUE if the TCAN replied correctly
RETURNS:
 <- PT_ENDED when done
------------------------------------------------------------------------------*/
static char nCANSPI_cCommsTest( PT *pt, boolean *poOk )
{
   //expected device ID reply
   static const char kaczTCAN[] = "TCAN";
   uint32_t lwDevID;

   PT_BEGIN( pt );

   PT_GOSUB( pt, nCANSPI_cTransceive( PT_SUB, TCANRAkeDevID0, ~0, &lwDevID ) );

   //TRUE if the device ID matches - thus the TCAN is ready
   *poOk = !memcmp( &lwDevID, kaczTCAN, sizeof( lwDevID ) );

   PT_END( pt );
   PT_RETURN();
}


/*-----------------------------------------------------------------------------
DESCRIPTION:
 Callled to see if the TCAN may have performed a reset (maybe due to cranking
 power dip) or see if it's currently in low voltage or thermal lockout.
 - If the baud rate register is the default value, this means we've lost our
 baud rate value, ie, we've been reset. (0x11601000 = the Bosch value
 (reversed), ref Can_44_TCAN4x5x_MCAN_SetBaudrate())
 - If the endian register can't be read, this means the clock to the M_CAN has
 stopped.
PARAMETERS:
  -> thread instance
 <-  TRUE if the power on flag is set, will clear it
 <-  TRUE if either low voltage or thermal fault flags are set (will self clear
 when the condition clears)
RETURNS:
 <- PT_ENDED when done
------------------------------------------------------------------------------*/
static char nCANSPI_cChkFaultFlags( PT *pt, boolean *poPwrOn, boolean *poThermErr )
{
   //to clear the power on and UVT flags, we write a 1 to them
   static const nTCAN4550tuIrqFlags kuTxClrErr =
   {
      .TSD   = 1,
      .PWRON = 1,
      .UVIO  = 1,
      .UVSUP = 1
   };
   nTCAN4550tuIrqFlags *psIRQFlags = PT_MALLOC( pt, nTCAN4550tuIrqFlags );
   nTCAN4550tuDBTP *puDBTP = PT_MALLOC( pt, nTCAN4550tuDBTP );
   uint32_t lwEndianReg = PT_RECALL( pt, uint32_t );
   boolean oBadReg;

   PT_BEGIN( pt );

   //read the IRQ flags
   PT_GOSUB( pt, nCANSPI_cTransceive( PT_SUB, TCANRAkeIrqFlags, ~0, &psIRQFlags->lw ) );

   //read the baud rate register
   PT_GOSUB( pt, nCANSPI_cTransceive( PT_SUB, TCANRAkeDBTP, ~0, &puDBTP->lw ) );

   //read the endian register
   PT_GOSUB( pt, nCANSPI_cTransceive( PT_SUB, TCANRAkeENDN, ~0, &lwEndianReg ) );

   //TRUE if DBTP == 0x00000A33(default) _OR_ empty endian register
   oBadReg = (puDBTP->lw == 0x00000A33u) || (lwEndianReg != 0x87654321u);

   //TRUE if (power on _OR_ UnderVoltage _OR_ )
   *poPwrOn = psIRQFlags->PWRON || psIRQFlags->UVSUP || psIRQFlags->UVIO;

   //TRUE if thermal fault is present
   *poThermErr = psIRQFlags->TSD;

   #if DET_PRINTF == STD_ON
   //something to show?
   if( *poPwrOn || *poThermErr || oBadReg )
   {
      //display it
      if( Det_RbLog( DET_MODULEID_MAINTASK, "CANSPI:" ) )
      {
         if( psIRQFlags->PWRON ) Det_RbLogContinue( " PWRON" );
         if( oBadReg )           Det_RbLogContinue( " BadRegister" );
         if( psIRQFlags->UVSUP ) Det_RbLogContinue( " UVSUP" );
         if( psIRQFlags->UVIO )  Det_RbLogContinue( " UVIO" );
         if( psIRQFlags->TSD )   Det_RbLogContinue( " TSD" );
         Det_RbLogContinue( "\r\n" );
      }
   }
   #endif //DET_PRINTF == STD_ON

   //also signal power on if we had a bad register
   *poPwrOn |= oBadReg;

   //was the therm flag(s) set? (don't worry about any flags grouped into "poweron" as we'll be resetting the TCAN)
   if( *poThermErr ) /* AXIVION Line MisraC2012-14.4: checked */
   {
      //yes, so clear it
      PT_GOSUB( pt, nCANSPI_cTransceive( PT_SUB, TCANRAkeIrqFlags, kuTxClrErr.lw, NULL ) );
   }

   PT_END( pt );
   PT_STORE( pt, lwEndianReg, uint32_t );
   PT_FREE( pt, nTCAN4550tuDBTP );
   PT_FREE( pt, nTCAN4550tuIrqFlags );
   PT_RETURN();
}


/*------------------------------------------------------------------------------
DESCRIPTION:
 Displays the TCAN & M_CAN version information.
PARAMETERS:
  -> thread instance
RETURNS:
 <-  nil
------------------------------------------------------------------------------*/
static char nCANSPI_cQueryVersion( PT *pt )
{
   //only include this if printf is enabled
   #if DET_PRINTF == STD_ON

   nTCAN4550tuTCANVer sTCANVer;
   nTCAN4550tuMCANVer sMCANVer;

   PT_BEGIN( pt );

   //get the TCAN version
   PT_GOSUB( pt, nCANSPI_cTransceive( PT_SUB, TCANRAkeTCANVer, ~0, &sTCANVer.lw ) );

   //display it
   Det_RbLog( DET_MODULEID_MAINTASK, "TCAN4550 Version:\r\n SPI_2 = %d\r\n   REV = %d.%d\r\n  RSVD = %d\r\n", sTCANVer.SPI_2_REVISION, sTCANVer.REV_ID_MAJOR, sTCANVer.REV_ID_MINOR, sTCANVer.RSVD );

   //get the MCAN version
   PT_GOSUB( pt, nCANSPI_cTransceive( PT_SUB, TCANRAkeCREL, ~0, &sMCANVer.lw ) );

   //display it
   Det_RbLog( DET_MODULEID_MAINTASK, "Bosch M_CAN Version:%d.%d.%d\r\n  Yr=%d Mon=%02x Day=%02x\r\n", sMCANVer.Rel, sMCANVer.Step, sMCANVer.SubStep, sMCANVer.Year, sMCANVer.Month, sMCANVer.Day );

   PT_END( pt );
   PT_RETURN();

   #else

   RBD_UNUSED( pt );
   return( PT_ENDED );

   #endif
}


/*-----------------------------------------------------------------------------
DESCRIPTION:
 Reads or write a single word to the TCAN, yielding until SPI is available.
 Handles the endian swap for both tx/rx data & the address (ARM is little,
 TCAN is big).
PARAMETERS:
  -> thread instance
  -> TCAN register to read/write
  -> data to send
 <-  received data (or NULL if this is a write)
RETURNS:
 <- PT_ENDED when done
------------------------------------------------------------------------------*/
static char nCANSPI_cTransceive( PT *pt, nTCAN4550teRegAddr eRegAddr, uint32_t lwTXData, uint32_t *plwRXData )
{
   typedef struct
   {
      nTCAN4550tsCommand;
      uint32_t lwData;
   }tsTx;
   typedef struct
   {
      nTCAN4550tsRxHeader;
      uint32_t lwData;
   }tsRx;

   tsTx *psTx = PT_MALLOC( pt, tsTx );
   tsRx *psRx = PT_MALLOC( pt, tsRx );

   PT_BEGIN( pt );

   //load the request
   *psTx = (tsTx)
   {
      .bOpCode = (plwRXData != NULL) ? TCANOPkeRead : TCANOPkeWrite,
      .wAddrBE = __rev16( eRegAddr ),
      .bNofWords = 1,
      .lwData = __rev( lwTXData )
   };

   //yield until we get access to SPI (won't yield if SPI is available on the first try)
   PT_WAIT_UNTIL( pt, Spi_SetupEB( SPI_CH_CAN, (void *)psTx, (void *)psRx, sizeof(*psTx) ) == E_OK );

   //transfer
   Spi_SyncTransmit( SPI_SEQ_CAN );

   //does the caller want the reply data?
   if( plwRXData ) /* AXIVION Line MisraC2012-14.4: checked */
   {
      //yes, so give it them
      *plwRXData = __rev( psRx->lwData );
   }

   PT_END( pt );
   PT_FREE( pt, tsRx );
   PT_FREE( pt, tsTx );
   PT_RETURN();
}
