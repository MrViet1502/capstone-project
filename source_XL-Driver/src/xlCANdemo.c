/*------------------------------------------------------------------------------
| File:
|   xlCANdemo.C
| Project:
|   Sample for XL - Driver Library
|   Example application using 'vxlapi.dll'
|-------------------------------------------------------------------------------
| $Author: visjb $    $Locker: $   $Revision: 101442 $
|-------------------------------------------------------------------------------
| Copyright (c) 2014 by Vector Informatik GmbH.  All rights reserved.
 -----------------------------------------------------------------------------*/

#if defined(_Windows) || defined(_MSC_VER) || defined (__GNUC__)
 #define  STRICT
 #include <windows.h>
#endif

#include <stdio.h>

#define UNUSED_PARAM(a) { a=a; }

#define RECEIVE_EVENT_SIZE         1        // DO NOT EDIT! Currently 1 is supported only
#define RX_QUEUE_SIZE              4096     // internal driver queue size in CAN events
#define RX_QUEUE_SIZE_FD           16384    // driver queue size for CAN-FD Rx events
#define ENABLE_CAN_FD_MODE_NO_ISO  0        // switch to activate no iso mode on a CAN FD channel

#include "vxlapi.h"

/////////////////////////////////////////////////////////////////////////////
// globals

char            g_AppName[XL_MAX_APPNAME+1]  = "xlCANdemo";               //!< Application name which is displayed in VHWconf
XLportHandle    g_xlPortHandle              = XL_INVALID_PORTHANDLE;      //!< Global porthandle (we use only one!)
XLdriverConfig  g_xlDrvConfig;                                            //!< Contains the actual hardware configuration
XLaccess        g_xlChannelMask             = 0;                          //!< Global channelmask (includes all founded channels)
XLaccess        g_xlPermissionMask          = 0;                          //!< Global permissionmask (includes all founded channels)
unsigned int    g_BaudRate                  = 500000;                     //!< Default baudrate
int             g_silent                    = 0;                          //!< flag to visualize the message events (on/off)
unsigned int    g_TimerRate                 = 0;                          //!< Global timerrate (to toggel)
unsigned int    g_canFdSupport              = 0;                          //!< Global CAN FD support flag
unsigned int    g_canFdModeNoIso            = ENABLE_CAN_FD_MODE_NO_ISO;  //!< Global CAN FD ISO (default) / no ISO mode flag

// tread variables
XLhandle        g_hMsgEvent;                                          //!< notification handle for the receive queue
HANDLE          g_hRXThread;                                          //!< thread handle (RX)
HANDLE          g_hTXThread;                                          //!< thread handle (TX)
int             g_RXThreadRun;                                        //!< flag to start/stop the RX thread
int             g_TXThreadRun;                                        //!< flag to start/stop the TX thread (for the transmission burst)
int             g_RXCANThreadRun;                                     //!< flag to start/stop the RX thread
unsigned int    g_TXThreadCanId ;                                     //!< CAN-ID the TX thread transmits under
XLaccess        g_TXThreadTxMask;                                     //!< channel mask the TX thread uses for transmitting


////////////////////////////////////////////////////////////////////////////
// functions (Threads)

DWORD     WINAPI RxCanFdThread( PVOID par );
DWORD     WINAPI RxThread( PVOID par );
DWORD     WINAPI TxThread( LPVOID par );

////////////////////////////////////////////////////////////////////////////
// functions (prototypes)
void     demoHelp(void);
void     demoPrintConfig(void);
XLstatus demoCreateRxThread(void);

// #ifdef __GNUC__
// static void strncpy_s(char *strDest, size_t numberOfElements, const char *strSource, size_t count)
// {
//   UNUSED_PARAM(numberOfElements);
//   strncpy(strDest, strSource, count);	
// }

// static void sscanf_s(const char *buffer, const char *format, ...)
// {
//   va_list argList;
//   va_start(argList, format);
//   sscanf(buffer, format, argList);
// }
// #endif

////////////////////////////////////////////////////////////////////////////

//! demoHelp()

//! shows the program functionality
//!
////////////////////////////////////////////////////////////////////////////

void demoHelp(void) {

  printf("\n----------------------------------------------------------\n");
  printf("-                   xlCANdemo - HELP                     -\n");
  printf("----------------------------------------------------------\n");
  printf("- Keyboard commands:                                     -\n");
  printf("- 't'      Transmit a message                            -\n");
  printf("- 'b'      Transmit a message burst (toggle)             -\n");
  printf("- 'm'      Transmit a remote message                     -\n");
  printf("- 'g'      Request chipstate                             -\n");
  printf("- 's'      Start/Stop                                    -\n");
  printf("- 'r'      Reset clock                                   -\n");
  printf("- '+'      Select channel      (up)                      -\n");
  printf("- '-'      Select channel      (down)                    -\n");
  printf("- 'i'      Select transmit Id  (up)                      -\n");
  printf("- 'I'      Select transmit Id  (down)                    -\n");
  printf("- 'x'      Toggle extended/standard Id                   -\n");
  printf("- 'o'      Toggle output mode                            -\n");
  printf("- 'a'      Toggle timer                                  -\n");
  printf("- 'v'      Toggle logging to screen                      -\n");
  printf("- 'p'      Show hardware configuration                   -\n");
  printf("- 'y'      Trigger HW-Sync pulse                         -\n");
  printf("- 'h'      Help                                          -\n");
  printf("- 'ESC'    Exit                                          -\n");
  printf("----------------------------------------------------------\n");
  printf("- 'PH'->PortHandle; 'CM'->ChannelMask; 'PM'->Permission  -\n");
  printf("----------------------------------------------------------\n\n");

}

////////////////////////////////////////////////////////////////////////////

//! demoPrintConfig()

//! shows the actual hardware configuration
//!
////////////////////////////////////////////////////////////////////////////

void demoPrintConfig(void) {

  unsigned int i;
  char         str[100];

  printf("----------------------------------------------------------\n");
  printf("- %02d channels       Hardware Configuration               -\n", g_xlDrvConfig.channelCount);
  printf("----------------------------------------------------------\n");

  for (i=0; i < g_xlDrvConfig.channelCount; i++) {

    printf("- Ch:%02d, CM:0x%03I64x,", 
      g_xlDrvConfig.channel[i].channelIndex, g_xlDrvConfig.channel[i].channelMask);

    strncpy_s(str, 100, g_xlDrvConfig.channel[i].name, 23);
    printf(" %23s,", str);

    memset(str, 0, sizeof(str));
    
    if (g_xlDrvConfig.channel[i].transceiverType != XL_TRANSCEIVER_TYPE_NONE) {
      strncpy_s( str, 100, g_xlDrvConfig.channel[i].transceiverName, 13);
      printf("%13s -\n", str);
    }
    else printf("    no Cab!   -\n");
   
  
  }
  
  printf("----------------------------------------------------------\n\n");
 
}

////////////////////////////////////////////////////////////////////////////

//! demoTransmit

//! transmit a CAN message (depending on an ID, channel)
//!
////////////////////////////////////////////////////////////////////////////

XLstatus demoTransmit(unsigned int txID, XLaccess xlChanMaskTx)
{
  XLstatus             xlStatus;
  unsigned int         messageCount = 1;
  static int           cnt = 0;

  if(g_canFdSupport) {
    unsigned int  fl[3] = {
      
      0 , // CAN (no FD)
      XL_CAN_TXMSG_FLAG_EDL,
      XL_CAN_TXMSG_FLAG_EDL | XL_CAN_TXMSG_FLAG_BRS,
    };

    XLcanTxEvent canTxEvt;
    unsigned int cntSent;
    unsigned int i;

    memset(&canTxEvt, 0, sizeof(canTxEvt));
    canTxEvt.tag = XL_CAN_EV_TAG_TX_MSG;

    canTxEvt.tagData.canMsg.canId     = txID;
    canTxEvt.tagData.canMsg.msgFlags  = fl[cnt%(sizeof(fl)/sizeof(fl[0]))];
    canTxEvt.tagData.canMsg.dlc       = 8;

    // if EDL is set, demonstrate transmit with DLC=15 (64 bytes)
    if (canTxEvt.tagData.canMsg.msgFlags & XL_CAN_TXMSG_FLAG_EDL) {
      canTxEvt.tagData.canMsg.dlc = 15;
    }

    ++cnt;
    
    for(i=1; i<XL_CAN_MAX_DATA_LEN; ++i) {
      canTxEvt.tagData.canMsg.data[i] = (unsigned char)i-1;
    }
    canTxEvt.tagData.canMsg.data[0] = (unsigned char)cnt;
    xlStatus = xlCanTransmitEx(g_xlPortHandle, xlChanMaskTx, messageCount, &cntSent, &canTxEvt);
  }
  else {
    static XLevent       xlEvent;
   
    memset(&xlEvent, 0, sizeof(xlEvent));

    xlEvent.tag                 = XL_TRANSMIT_MSG;
    xlEvent.tagData.msg.id      = txID;
    xlEvent.tagData.msg.dlc     = 8;
    xlEvent.tagData.msg.flags   = 0;
    ++xlEvent.tagData.msg.data[0];
    xlEvent.tagData.msg.data[1] = 2;
    xlEvent.tagData.msg.data[2] = 3;
    xlEvent.tagData.msg.data[3] = 4;
    xlEvent.tagData.msg.data[4] = 5;
    xlEvent.tagData.msg.data[5] = 6;
    xlEvent.tagData.msg.data[6] = 7;
    xlEvent.tagData.msg.data[7] = 8;

    xlStatus = xlCanTransmit(g_xlPortHandle, xlChanMaskTx, &messageCount, &xlEvent);
   // printf("%s\n", xlChanMaskTx, xlGetErrorString(xlStatus));

  }

  printf("- Transmit         : CM(0x%I64x), %s\n", xlChanMaskTx, xlGetErrorString(xlStatus));

  return xlStatus;
}

////////////////////////////////////////////////////////////////////////////

//! demoStopTransmitBurst

//! Stop the TX thread if it is running.
//!
////////////////////////////////////////////////////////////////////////////

void demoStopTransmitBurst()
{
  if (g_hTXThread) {
    g_TXThreadRun = 0;
    WaitForSingleObject(g_hTXThread, 10);
    g_hTXThread = 0;
  }
}

////////////////////////////////////////////////////////////////////////////

//! demoTransmitBurst

//! transmit a message burst (also depending on an IC, channel).
//!
////////////////////////////////////////////////////////////////////////////

void demoTransmitBurst(unsigned int txID, XLaccess xlChanMaskTx) 
{
  // first collect old TX-Thread
  demoStopTransmitBurst();

  printf("- print txID: %d\n", txID);
  g_TXThreadCanId = txID;
  g_TXThreadTxMask = xlChanMaskTx;
  g_TXThreadRun = 1;
  g_hTXThread = CreateThread(0, 0x1000, TxThread, NULL, 0, NULL);
}

////////////////////////////////////////////////////////////////////////////

//! demoTransmitRemote

//! transmit a remote frame
//!
////////////////////////////////////////////////////////////////////////////

XLstatus demoTransmitRemote(unsigned int txID, XLaccess xlChanMaskTx)
{
  XLstatus      xlStatus;
  unsigned int  messageCount = 1;

  if(g_canFdSupport) {
    XLcanTxEvent canTxEvt;
    unsigned int cntSent;

    memset(&canTxEvt, 0, sizeof(canTxEvt));

    canTxEvt.tag = XL_CAN_EV_TAG_TX_MSG;

    canTxEvt.tagData.canMsg.canId     = txID;
    canTxEvt.tagData.canMsg.msgFlags  = XL_CAN_TXMSG_FLAG_RTR;
    canTxEvt.tagData.canMsg.dlc       = 8;

    xlStatus = xlCanTransmitEx(g_xlPortHandle, xlChanMaskTx, messageCount, &cntSent, &canTxEvt);
  }
  else {
    XLevent       xlEvent;

    memset(&xlEvent, 0, sizeof(xlEvent));

    xlEvent.tag               = XL_TRANSMIT_MSG;
    xlEvent.tagData.msg.id    = txID;
    xlEvent.tagData.msg.flags = XL_CAN_MSG_FLAG_REMOTE_FRAME;
    xlEvent.tagData.msg.dlc   = 8;

    xlStatus = xlCanTransmit(g_xlPortHandle, xlChanMaskTx, &messageCount, &xlEvent);
  }

  printf("- Transmit REMOTE  : CM(0x%I64x), %s\n", g_xlChannelMask, xlGetErrorString(xlStatus));
  
  return XL_SUCCESS;
}

////////////////////////////////////////////////////////////////////////////

//! demoStartStop

//! toggle the channel activate/deactivate
//!
////////////////////////////////////////////////////////////////////////////

XLstatus demoStartStop(int activated)
{
  XLstatus xlStatus;

  if (activated) {
    xlStatus = xlActivateChannel(g_xlPortHandle, g_xlChannelMask, XL_BUS_TYPE_CAN, XL_ACTIVATE_RESET_CLOCK);
    printf("- ActivateChannel : CM(0x%I64x), %s\n", g_xlChannelMask, xlGetErrorString(xlStatus));
  }
  else {
    demoStopTransmitBurst();
    xlStatus = xlDeactivateChannel(g_xlPortHandle, g_xlChannelMask);
    printf("- DeativateChannel: CM(0x%I64x), %s\n", g_xlChannelMask, xlGetErrorString(xlStatus));
  }

  return XL_SUCCESS;
}

////////////////////////////////////////////////////////////////////////////

//! demoSetOutput

//! toggle NORMAL/SILENT mode of a CAN channel
//!
////////////////////////////////////////////////////////////////////////////

XLstatus demoSetOutput(int outputMode, const char *sMode, XLaccess xlChanMaskTx) {
  
  XLstatus xlStatus;

  // to get an effect we deactivate the channel first.
  xlStatus = xlDeactivateChannel(g_xlPortHandle, g_xlChannelMask);

  xlStatus = xlCanSetChannelOutput(g_xlPortHandle, xlChanMaskTx, outputMode);
  printf("- SetChannelOutput: CM(0x%I64x), %s, %s, %d\n", xlChanMaskTx, sMode, xlGetErrorString(xlStatus), outputMode);
 
  // and activate the channel again.
  xlStatus = xlActivateChannel(g_xlPortHandle, g_xlChannelMask, XL_BUS_TYPE_CAN, XL_ACTIVATE_RESET_CLOCK);
  
  return xlStatus;

}

////////////////////////////////////////////////////////////////////////////

//! demoCreateRxThread

//! set the notification and creates the thread.
//!
////////////////////////////////////////////////////////////////////////////

XLstatus demoCreateRxThread(void) {
  XLstatus      xlStatus = XL_ERROR;
  DWORD         ThreadId=0;
 
  if (g_xlPortHandle!= XL_INVALID_PORTHANDLE) {

      // Send a event for each Msg!!!
      xlStatus = xlSetNotification (g_xlPortHandle, &g_hMsgEvent, 1);

      if (g_canFdSupport) {
        g_hRXThread = CreateThread(0, 0x1000, RxCanFdThread, (LPVOID) 0, 0, &ThreadId);
      }
      else { 
        g_hRXThread = CreateThread(0, 0x1000, RxThread, (LPVOID) 0, 0, &ThreadId);
      }

  }
  return xlStatus;
}

////////////////////////////////////////////////////////////////////////////

//! demoInitDriver

//! initializes the driver with one port and all founded channels which
//! have a connected CAN cab/piggy.
//!
////////////////////////////////////////////////////////////////////////////

XLstatus demoInitDriver(XLaccess *pxlChannelMaskTx, unsigned int *pxlChannelIndex) {

  XLstatus          xlStatus;
  unsigned int      i;
  XLaccess          xlChannelMaskFd = 0;
  XLaccess          xlChannelMaskFdNoIso = 0;
  
  // ------------------------------------
  // open the driver
  // ------------------------------------
  xlStatus = xlOpenDriver ();
  
  // ------------------------------------
  // get/print the hardware configuration
  // ------------------------------------
  if(XL_SUCCESS == xlStatus) {
    xlStatus = xlGetDriverConfig(&g_xlDrvConfig);
  }
  
  if(XL_SUCCESS == xlStatus) {
    demoPrintConfig();
    
    printf("Usage: xlCANdemo <BaudRate> <ApplicationName> <Identifier>\n\n");
    
    // ------------------------------------
    // select the wanted channels
    // ------------------------------------
    g_xlChannelMask = 0;
    for (i=0; i < g_xlDrvConfig.channelCount; i++) {
      
      // we take all hardware we found and supports CAN
      if (g_xlDrvConfig.channel[i].channelBusCapabilities & XL_BUS_ACTIVE_CAP_CAN) { 
        
        if (!*pxlChannelMaskTx) {
          *pxlChannelMaskTx = g_xlDrvConfig.channel[i].channelMask;
          *pxlChannelIndex  = g_xlDrvConfig.channel[i].channelIndex;
        }

        // check if we can use CAN FD - the virtual CAN driver supports CAN-FD, but we don't use it
        if ((g_xlDrvConfig.channel[i].channelCapabilities & XL_CHANNEL_FLAG_CANFD_ISO_SUPPORT)
           && (g_xlDrvConfig.channel[i].hwType != XL_HWTYPE_VIRTUAL)) {
          xlChannelMaskFd |= g_xlDrvConfig.channel[i].channelMask;
          
          // check CAN FD NO ISO support
          if (g_xlDrvConfig.channel[i].channelCapabilities & XL_CHANNEL_FLAG_CANFD_BOSCH_SUPPORT) {
            xlChannelMaskFdNoIso |= g_xlDrvConfig.channel[i].channelMask;
          }
        }
        else {
          g_xlChannelMask |= g_xlDrvConfig.channel[i].channelMask;
        }
        
      }
    }

    // if we found a CAN FD supported channel - we use it.
    if (xlChannelMaskFd && !g_canFdModeNoIso) {
      g_xlChannelMask = xlChannelMaskFd;
      printf("- Use CAN-FD for   : CM=0x%I64x\n", g_xlChannelMask);
      g_canFdSupport = 1;
    }

    if (xlChannelMaskFdNoIso && g_canFdModeNoIso) {
      g_xlChannelMask = xlChannelMaskFdNoIso;
      printf("- Use CAN-FD NO ISO for   : CM=0x%I64x\n", g_xlChannelMask);
      g_canFdSupport = 1;
    }
    
    if (!g_xlChannelMask) {
      printf("ERROR: no available channels found! (e.g. no CANcabs...)\n\n");
      xlStatus = XL_ERROR;
    }
  }

  g_xlPermissionMask = g_xlChannelMask;
  
  // ------------------------------------
  // open ONE port including all channels
  // ------------------------------------
  if(XL_SUCCESS == xlStatus) {
    
    // check if we can use CAN FD
    if (g_canFdSupport) {
      xlStatus = xlOpenPort(&g_xlPortHandle, g_AppName, g_xlChannelMask, &g_xlPermissionMask, RX_QUEUE_SIZE_FD, XL_INTERFACE_VERSION_V4, XL_BUS_TYPE_CAN);
    }
    // if not, we make 'normal' CAN
    else {
      xlStatus = xlOpenPort(&g_xlPortHandle, g_AppName, g_xlChannelMask, &g_xlPermissionMask, RX_QUEUE_SIZE, XL_INTERFACE_VERSION, XL_BUS_TYPE_CAN);
     
    }
     printf("- OpenPort         : CM=0x%I64x, PH=0x%02X, PM=0x%I64x, %s\n", 
              g_xlChannelMask, g_xlPortHandle, g_xlPermissionMask, xlGetErrorString(xlStatus));
       
  }

  if ( (XL_SUCCESS == xlStatus) && (XL_INVALID_PORTHANDLE != g_xlPortHandle) ) {
    
    // ------------------------------------
    // if we have permission we set the
    // bus parameters (baudrate)
    // ------------------------------------
    if (g_xlChannelMask == g_xlPermissionMask) {

      if(g_canFdSupport) {
        XLcanFdConf fdParams;
       
        memset(&fdParams, 0, sizeof(fdParams));
        
        // arbitration bitrate
        fdParams.arbitrationBitRate = 1000000;
        fdParams.tseg1Abr           = 6;
        fdParams.tseg2Abr           = 3;
        fdParams.sjwAbr             = 2;

        // data bitrate
        fdParams.dataBitRate = fdParams.arbitrationBitRate*2;
        fdParams.tseg1Dbr    = 6;
        fdParams.tseg2Dbr    = 3;
        fdParams.sjwDbr      = 2;

        if (g_canFdModeNoIso) {
          fdParams.options = CANFD_CONFOPT_NO_ISO;
        }

        xlStatus = xlCanFdSetConfiguration(g_xlPortHandle, g_xlChannelMask, &fdParams);
        printf("- SetFdConfig.     : ABaudr.=%u, DBaudr.=%u, %s\n", fdParams.arbitrationBitRate, fdParams.dataBitRate, xlGetErrorString(xlStatus));

      }
      else {
        xlStatus = xlCanSetChannelBitrate(g_xlPortHandle, g_xlChannelMask, g_BaudRate);
        printf("- SetChannelBitrate: baudr.=%u, %s\n",g_BaudRate, xlGetErrorString(xlStatus));
      }
    } 
    else {
      printf("-                  : we have NO init access!\n");
    }
  }
  else {
  
    xlClosePort(g_xlPortHandle);
    g_xlPortHandle = XL_INVALID_PORTHANDLE;
    xlStatus = XL_ERROR;
  }
  
  return xlStatus;
  
}                    

////////////////////////////////////////////////////////////////////////////

//! demoCleanUp()

//! close the port and the driver
//!
////////////////////////////////////////////////////////////////////////////

static XLstatus demoCleanUp(void)
{
  XLstatus xlStatus;
    
  if (g_xlPortHandle != XL_INVALID_PORTHANDLE) {
    xlStatus = xlClosePort(g_xlPortHandle);
    printf(": PH(0x%x), %s\n", g_xlPortHandle, xlGetErrorString(xlStatus));
  }

  g_xlPortHandle = XL_INVALID_PORTHANDLE;
  xlCloseDriver();

  return XL_SUCCESS;    // No error handling
}

////////////////////////////////////////////////////////////////////////////

//! main

//! 
//!
////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
  XLstatus      xlStatus;
  XLaccess      xlChanMaskTx = 0;

  int           stop = 0;
  int           activated = 0;
  int           c;
  unsigned int  xlChanIndex = 0;
  unsigned int  txID = 0x01;
  int           outputMode = XL_OUTPUT_MODE_NORMAL;


  printf("----------------------------------------------------------\n");
  printf("- xlCANdemo - Test Application for XL Family Driver API  -\n");
  printf("-             Vector Informatik GmbH,  " __DATE__"       -\n");
#ifdef WIN64
  printf("-             - 64bit Version -                          -\n");
#endif
  printf("----------------------------------------------------------\n");

  // ------------------------------------
  // commandline may specify application 
  // name and baudrate
  // ------------------------------------
  if (argc > 1) {
    g_BaudRate = atoi(argv[1]);
    if (g_BaudRate) {
      printf("Baudrate = %u\n", g_BaudRate);
      argc--;
      argv++;
    }
  }
  if (argc > 1) {
    strncpy_s(g_AppName, XL_MAX_APPNAME, argv[1], XL_MAX_APPNAME);
    g_AppName[XL_MAX_APPNAME] = 0;
    printf("AppName = %s\n", g_AppName);
    argc--;
    argv++;
  }
  if (argc > 1) {
    sscanf_s (argv[1], "%x", &txID ) ;
    if (txID) {
      printf("TX ID = %x\n", txID);
    }
  }

  // ------------------------------------
  // initialize the driver structures 
  // for the application
  // ------------------------------------
  xlStatus = demoInitDriver(&xlChanMaskTx, &xlChanIndex);
  printf("- Init             : %s\n",  xlGetErrorString(xlStatus));
  
  if(XL_SUCCESS == xlStatus) {
    // ------------------------------------
    // create the RX thread to read the
    // messages
    // ------------------------------------
    xlStatus = demoCreateRxThread();
    printf("- Create RX thread : %s\n",  xlGetErrorString(xlStatus));
  }

  if(XL_SUCCESS == xlStatus) {
    // ------------------------------------
    // go with all selected channels on bus
    // ------------------------------------
    xlStatus = xlActivateChannel(g_xlPortHandle, g_xlChannelMask, XL_BUS_TYPE_CAN, XL_ACTIVATE_RESET_CLOCK);
    printf("- ActivateChannel  : CM=0x%I64x, %s\n", g_xlChannelMask, xlGetErrorString(xlStatus));
    if(xlStatus == XL_SUCCESS) activated = 1;
  }

  printf("\n: Press <h> for help - actual channel Ch=%d, CM=0x%02I64x\n", xlChanIndex, xlChanMaskTx);

  // ------------------------------------
  // parse the key - commands
  // ------------------------------------
  while (stop == 0) {

    unsigned long n;
    INPUT_RECORD ir;

    ReadConsoleInput(GetStdHandle(STD_INPUT_HANDLE), &ir, 1, &n);

    if ((n == 1) && (ir.EventType == KEY_EVENT)) {

      if (ir.Event.KeyEvent.bKeyDown) {

        c = ir.Event.KeyEvent.uChar.AsciiChar;
        switch (c) {

          case 'v':
            if (g_silent) { g_silent = 0; printf("- screen on\n"); }
            else {g_silent = 1; printf("- screen off\n"); }
            break;

          case 't': // transmit a message
            
            printf("Start Transmitting a message!!!\n");
            demoTransmit(txID, xlChanMaskTx);
            printf("End Transmitting a message!!!\n");
            break;

          case 'b':  // transmit message burst 
            if (g_TXThreadRun) {
              demoStopTransmitBurst();
            }
            else {
              demoTransmitBurst(txID, xlChanMaskTx);
            }
            break; 

          case 'm': // transmit a remote message
            demoTransmitRemote(txID, xlChanMaskTx);
            break;

          case '-': // channel selection
            
            if (xlChanIndex == 0) xlChanIndex = g_xlDrvConfig.channelCount;
            xlChanIndex--;
            
            xlChanMaskTx = g_xlDrvConfig.channel[xlChanIndex].channelMask;
            printf("- TX Channel set to channel: %02d, %s CM(0x%I64x)\n", 
              g_xlDrvConfig.channel[xlChanIndex].channelIndex, g_xlDrvConfig.channel[xlChanIndex].name, xlChanMaskTx);
            break;
            
          case '+': // channel selection   
            xlChanIndex++;
            if (xlChanIndex >= g_xlDrvConfig.channelCount) xlChanIndex = 0;

            xlChanMaskTx = g_xlDrvConfig.channel[xlChanIndex].channelMask;
            printf("- TX Channel set to channel: %02d, %s CM(0x%I64x)\n", 
              g_xlDrvConfig.channel[xlChanIndex].channelIndex, g_xlDrvConfig.channel[xlChanIndex].name, xlChanMaskTx);
            break;

          case 'x':
            txID ^= XL_CAN_EXT_MSG_ID; // toggle ext/std
            printf("- Id set to 0x%08X\n", txID);
            break;

          case 'I': // id selection
            if (txID & XL_CAN_EXT_MSG_ID) txID = (txID-1) | XL_CAN_EXT_MSG_ID;
            else if (txID == 0) txID = 0x7FF;
            else txID--;
            printf("- Id set to 0x%08X\n", txID);
            break;

          case 'i':
            if (txID & XL_CAN_EXT_MSG_ID) txID = (txID+1) | XL_CAN_EXT_MSG_ID;
            else if (txID == 0x7FF) txID = 0;
            else txID++;
            printf("- Id set to 0x%08X\n", txID);
            break;

          case 'g':
            xlStatus = xlCanRequestChipState(g_xlPortHandle, g_xlChannelMask);
            printf("- RequestChipState : CM(0x%I64x), %s\n", g_xlChannelMask, xlGetErrorString(xlStatus));
            break;

          case 'a':
            if (g_TimerRate) g_TimerRate = 0; 
            else g_TimerRate = 20000;
 
            xlStatus = xlSetTimerRate(g_xlPortHandle, g_TimerRate);
            printf("- SetTimerRate     : %d, %s\n", g_TimerRate, xlGetErrorString(xlStatus));
            break;

          case 'o':
            if (outputMode == XL_OUTPUT_MODE_NORMAL) {
              outputMode = XL_OUTPUT_MODE_SILENT;
              demoSetOutput(outputMode, "SILENT", xlChanMaskTx);
            }
            else {
              outputMode = XL_OUTPUT_MODE_NORMAL;
              demoSetOutput(outputMode, "NORMAL", xlChanMaskTx);
            }
            break;

          case 'r':
            xlStatus = xlResetClock(g_xlPortHandle);
            printf("- ResetClock       : %s\n", xlGetErrorString(xlStatus));
            break;

          case 's':
            if (activated) activated = 0;
            else activated = 1;
            demoStartStop(activated);
            break;

          case 'p':
            demoPrintConfig();
            break;

          case 'y':
            xlStatus = xlGenerateSyncPulse(g_xlPortHandle, xlChanMaskTx);
            printf("- xlGenerateSyncPulse : CM(0x%I64x), %s\n", xlChanMaskTx, xlGetErrorString(xlStatus));
            break;

          case 27: // end application
            stop=1;
            break;

          case 'h':
            demoHelp();
            break;

          default:
            break;
                                                            // end switch
        }
      }
    }
  }                                                         // end while
  

  if((XL_SUCCESS != xlStatus) && activated) { 
    xlStatus = xlDeactivateChannel(g_xlPortHandle, g_xlChannelMask);
    printf("- DeactivateChannel: CM(0x%I64x), %s\n", g_xlChannelMask, xlGetErrorString(xlStatus));
  } 
  demoCleanUp();

  return(0);
}                                                  // end main()


////////////////////////////////////////////////////////////////////////////

//! TxThread

//! 
//!
////////////////////////////////////////////////////////////////////////////

DWORD WINAPI TxThread( LPVOID par ) 
{
  XLstatus      xlStatus = XL_SUCCESS;
  unsigned int  n = 1;
  XLcanTxEvent  canTxEvt;
  XLevent       xlEvent;
  unsigned int  cntSent;

  UNREFERENCED_PARAMETER(par);

  if(g_canFdSupport) {
  
    unsigned int i;

    memset(&canTxEvt, 0, sizeof(canTxEvt));
    canTxEvt.tag = XL_CAN_EV_TAG_TX_MSG;

    canTxEvt.tagData.canMsg.canId     = g_TXThreadCanId;
    canTxEvt.tagData.canMsg.msgFlags  = XL_CAN_TXMSG_FLAG_EDL | XL_CAN_TXMSG_FLAG_BRS;
    canTxEvt.tagData.canMsg.dlc       = 15;

    for(i=1; i<XL_CAN_MAX_DATA_LEN; ++i) {
      canTxEvt.tagData.canMsg.data[i] = (unsigned char)i-1;
    }
  }
  else {

    memset(&xlEvent, 0, sizeof(xlEvent));

    xlEvent.tag                 = XL_TRANSMIT_MSG;
    xlEvent.tagData.msg.id      = g_TXThreadCanId;
    xlEvent.tagData.msg.dlc     = 8;
    xlEvent.tagData.msg.flags   = 0;
    xlEvent.tagData.msg.data[0] = 1;
    xlEvent.tagData.msg.data[1] = 2;
    xlEvent.tagData.msg.data[2] = 3;
    xlEvent.tagData.msg.data[3] = 4;
    xlEvent.tagData.msg.data[4] = 5;
    xlEvent.tagData.msg.data[5] = 6;
    xlEvent.tagData.msg.data[6] = 7;
    xlEvent.tagData.msg.data[7] = 8;
  
  }

  while (g_TXThreadRun && XL_SUCCESS == xlStatus) {

    if(g_canFdSupport) {
      ++canTxEvt.tagData.canMsg.data[0];
      xlStatus = xlCanTransmitEx(g_xlPortHandle, g_TXThreadTxMask, n, &cntSent, &canTxEvt);
    }
    else {
      ++xlEvent.tagData.msg.data[0];
      xlStatus = xlCanTransmit(g_xlPortHandle, g_TXThreadTxMask, &n, &xlEvent);
    }
    
    Sleep(10);
 
  }

  if(XL_SUCCESS != xlStatus) {
    printf("Error xlCanTransmit:%s\n", xlGetErrorString(xlStatus));
  }

  g_TXThreadRun = 0;
  return NO_ERROR; 
}

///////////////////////////////////////////////////////////////////////////

//! RxThread

//! thread to readout the message queue and parse the incoming messages
//!
////////////////////////////////////////////////////////////////////////////

DWORD WINAPI RxThread(LPVOID par) 
{
  XLstatus        xlStatus;
  
  unsigned int    msgsrx = RECEIVE_EVENT_SIZE;
  XLevent         xlEvent; 
  
  UNUSED_PARAM(par); 
  
  g_RXThreadRun = 1;

  while (g_RXThreadRun) { 
   
    WaitForSingleObject(g_hMsgEvent,10);

    xlStatus = XL_SUCCESS;
    
    while (!xlStatus) {
      
      msgsrx = RECEIVE_EVENT_SIZE;

      xlStatus = xlReceive(g_xlPortHandle, &msgsrx, &xlEvent);      
      if ( xlStatus!=XL_ERR_QUEUE_IS_EMPTY ) {

        if (!g_silent) {
          printf("%s\n", xlGetEventString(&xlEvent));
        }

      }  
    }
          
  }
  return NO_ERROR; 
}



///////////////////////////////////////////////////////////////////////////

//! RxCANThread

//! thread to read the message queue and parse the incoming messages
//!
////////////////////////////////////////////////////////////////////////////
DWORD WINAPI RxCanFdThread(LPVOID par)
{
  XLstatus        xlStatus = XL_SUCCESS;
  DWORD           rc;
  XLcanRxEvent    xlCanRxEvt;
  
  UNUSED_PARAM(par); 

  g_RXCANThreadRun = 1;
  
  while (g_RXCANThreadRun) {
    rc = WaitForSingleObject(g_hMsgEvent, 10);
    if(rc != WAIT_OBJECT_0) continue;
    
    do {
      xlStatus = xlCanReceive(g_xlPortHandle, &xlCanRxEvt);
      if(xlStatus==XL_ERR_QUEUE_IS_EMPTY ) {
        break;
      }
     if (!g_silent) {
      printf("%s\n", xlCanGetEventString(&xlCanRxEvt));
     }

    } while(XL_SUCCESS == xlStatus);
  }
  
  return(NO_ERROR); 
} // RxCanFdThread

