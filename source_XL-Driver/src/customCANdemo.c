/**********************************************************************************************************************
 *  START OF FILE: vMCAL.c
 *********************************************************************************************************************/
//Son was here
/////////////////////////////////////////////////////////////////////////////
// include

#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <stdbool.h>
#include "vxlapi.h"
#include <stdint.h>
/////////////////////////////////////////////////////////////////////////////
// definition 
#define UNUSED_PARAM(a) { a=a; }

#define RECEIVE_EVENT_SIZE         1        // DO NOT EDIT! Currently 1 is supported only
#define RX_QUEUE_SIZE              4096     // internal driver queue size in CAN events
#define RX_QUEUE_SIZE_FD           16384    // driver queue size for CAN-FD Rx events
#define ENABLE_CAN_FD_MODE_NO_ISO  0        // switch to activate no iso mode on a CAN FD channel
#define CAN                        0        //Flag for normal CAN
#define CAN_FD                     1        //Flag for CANFD

/////////////////////////////////////////////////////////////////////////////
// globals

char            g_AppName[XL_MAX_APPNAME+1]  = "autosar_executable";               //!< Application name which is displayed in VHWconf
XLportHandle    g_xlPortHandle              = XL_INVALID_PORTHANDLE;      //!< Global porthandle (we use only one!)
XLdriverConfig  g_xlDrvConfig;                                            //!< Contains the actual hardware configuration
XLaccess        g_xlChannelMask             = 0;                          //!< Global channelmask (includes all founded channels)
XLaccess        g_xlPermissionMask          = 0;                          //!< Global permissionmask (includes all founded channels)
unsigned int    g_BaudRate                  = 500000;                     //!< Default baudrate
int             g_silent                    = 0;                          //!< flag to visualize the message events (on/off)
unsigned int    g_TimerRate                 = 0;                          //!< Global timerrate (to toggel)

unsigned int    CANType                     = CAN;                          //!< Global CAN type flag. 
//It starts with CAN but later will change to CAN FD if CAN FD is detected! 

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
int count = 0;

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
XLstatus CAN_DriverInit(XLaccess* pxlChannelMaskTx, unsigned int* pxlChannelIndex);
XLstatus CAN_CreateRxThread(void);
void vMCAL_CAN_Init();
XLstatus CAN_Transmit(uint32_t txID, XLaccess xlChanMaskTx, uint8_t *payload, uint8_t dataLength);

extern XLstatus CAN_DriverInit(XLaccess* pxlChannelMaskTx, unsigned int* pxlChannelIndex);
extern XLstatus CAN_CreateRxThread(void);
extern void vMCAL_CAN_Init();
extern XLstatus CAN_Transmit(uint32_t txID, XLaccess xlChanMaskTx, uint8_t *payload, uint8_t dataLength);
////////////////////////////////////////////////////////////////////////////
// Custom API 

// void CAN_Msg_mapper(const XLevent* xlEvent)
// {
//     if (xlEvent->tag == XL_RECEIVE_MSG)
//     {

//         // hrh could be set to 3 or 1 
//         CanIf_HwHandleType hrh = 3;

//         Can_IdType canId = xlEvent->tagData.msg.id;

//         uint8 canDlc = xlEvent->tagData.msg.dlc;

//         const uint8* canSduPtr = xlEvent->tagData.msg.data;


//         // xlFlushReceiveQueue(g_xlPortHandle);

//     }
// }

// void CanFD_Msg_mapper(const XLcanRxEvent* xlCanRxEvt)
// {
//   if(xlCanRxEvt->tag == XL_RECEIVE_MSG)
//     {
//       // hrh could be set to 3 or 1 
//       CanIf_HwHandleType hrh = 3;//mapChannelToHrh((CanIf_HwHandleType)xlEvent->chanIndex);

//       Can_IdType canId = xlCanRxEvt->tagData.canRxOkMsg.canId;

//       uint8 canDlc = xlCanRxEvt->tagData.canRxOkMsg.dlc;

//       // const uint8* canSduPtr = xlCanRxEvt->tagData.canRxOkMsg.data;

//       uint8 Service31_write[8]= {0x31, 0x01, 0x08, 0xFF, 0x02, 0x0A, 0x02, 0x02};
//       const uint8 canSduPtr[64];
//       memset(&canSduPtr, 0, sizeof(canSduPtr));
//       memcpy(canSduPtr, Service31_write, 8*sizeof(Service31_write));

//       CanIf_RxIndicationAsr403(hrh, canId, canDlc, canSduPtr);

//       // xlFlushReceiveQueue(g_xlPortHandle);

//     }
// }

////////////////////////////////////////////////////////////////////////////
// CAN XLDriver API

XLstatus CAN_DriverInit(XLaccess* pxlChannelMaskTx, unsigned int* pxlChannelIndex)
{

    XLstatus          xlStatus;
    unsigned int      i;
    XLaccess          xlChannelMaskFd = 0;
    XLaccess          xlChannelMaskFdNoIso = 0;
    
    // ------------------------------------
    // open the driver
    // ------------------------------------
    xlStatus = xlOpenDriver();
    
    // ------------------------------------
    // get/print the hardware configuration
    // ------------------------------------
    if(XL_SUCCESS == xlStatus) {
        xlStatus = xlGetDriverConfig(&g_xlDrvConfig);
    }
    
    if(XL_SUCCESS == xlStatus) {
        // demoPrintConfig();
        
        // printf("Usage: xlCANdemo <BaudRate> <ApplicationName> <Identifier>\n\n");
        
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

        // if we found a CAN FD supported channel - we change to CAN FD 
        if (xlChannelMaskFd && !g_canFdModeNoIso) {
        g_xlChannelMask = xlChannelMaskFd;
        printf("- Use CAN-FD for   : CM=0x%I64x\n", g_xlChannelMask);
        CANType = CAN_FD;
        }

        if (xlChannelMaskFdNoIso && g_canFdModeNoIso) {
        g_xlChannelMask = xlChannelMaskFdNoIso;
        printf("- Use CAN-FD NO ISO for   : CM=0x%I64x\n", g_xlChannelMask);
        CANType = CAN_FD;
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
        if (CANType) {
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

        if(CANType) {
        //Channel bitrate for CAN FD
            XLcanFdConf fdParams;
        
            memset(&fdParams, 0, sizeof(fdParams));
            
            // arbitration bitrate
            fdParams.arbitrationBitRate = g_BaudRate;
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
        //Channel bitrate for CAN 
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

XLstatus CAN_CreateRxThread(void) 
{
    XLstatus      xlStatus = XL_ERROR;
    DWORD         ThreadId=0;
 
    if (g_xlPortHandle!= XL_INVALID_PORTHANDLE) {

        // Send a event for each Msg!!!
        xlStatus = xlSetNotification (g_xlPortHandle, &g_hMsgEvent, 1);

        if (CANType) {
            g_hRXThread = CreateThread(0, 0x1000, RxCanFdThread, (LPVOID) 0, 0, &ThreadId);
        }
        else { 
            g_hRXThread = CreateThread(0, 0x1000, RxThread, (LPVOID) 0, 0, &ThreadId);
        }

    }
    return xlStatus;
}

// This one is for noramal CAN 
DWORD WINAPI RxThread(LPVOID par)
{
    XLstatus xlStatus;
    unsigned int msgsrx = RECEIVE_EVENT_SIZE;
    XLevent xlEvent;
    struct s_xl_can_msg* canMsg;
    unsigned int guard = 0;

    UNUSED_PARAM(par);

    g_RXThreadRun = 1;

    while (g_RXThreadRun)
    {
        WaitForSingleObject(g_hMsgEvent, 10);

        xlStatus = XL_SUCCESS;

        while (xlStatus == XL_SUCCESS)
        {
            msgsrx = RECEIVE_EVENT_SIZE;

            xlStatus = xlReceive(g_xlPortHandle, &msgsrx, &xlEvent);

            if (xlStatus != XL_ERR_QUEUE_IS_EMPTY && guard == 0)
            {
                // if (!g_silent)
                // {
                //     CAN_Msg_mapper(&xlEvent);


                //     triggerRestart = 1;
                //     xlFlushReceiveQueue(g_xlPortHandle);
                // }

                canMsg = (struct s_xl_can_msg*)&xlEvent.tagData.msg;
                if (canMsg->id != 848 && canMsg->dlc > 0 && canMsg->dlc <= 8) 
                
                {
                    if (!g_silent)
                    {
                    // CAN_Msg_mapper(&xlEvent);
                                   printf("%s\n", xlCanGetEventString(&xlEvent));

                    // triggerRestart = 1;
                    guard++;


                    }
                xlCanFlushTransmitQueue(g_xlPortHandle, g_xlChannelMask);
                }
            }
            else guard = 0;
        }
        // xlFlusheceiveQueue(g_xlPortHandle);
    }

    return NO_ERROR;
}//RxThread 

//This one is for CAN FD
DWORD WINAPI RxCanFdThread(LPVOID par) // for CANFD
{
   XLstatus        xlStatus = XL_SUCCESS;
   DWORD           rc;

   XLcanRxEvent    xlCanRxEvt; // received message here

   UNUSED_PARAM(par);

   g_RXCANThreadRun = 1;

   while (g_RXCANThreadRun) {
       rc = WaitForSingleObject(g_hMsgEvent, 10);
       if (rc != WAIT_OBJECT_0) continue;

       do {
           xlStatus = xlCanReceive(g_xlPortHandle, &xlCanRxEvt);
            
           if (xlStatus == XL_ERR_QUEUE_IS_EMPTY) 
           {
               break;
           }
           if (!g_silent) 
           {
            //   CanFD_Msg_mapper(&xlCanRxEvt);
               printf("%s\n", xlCanGetEventString(&xlCanRxEvt));
           }

       } while (XL_SUCCESS == xlStatus);
   }

   return(NO_ERROR);
} // RxCanFdThread

void vMCAL_CAN_Init(void)
{
   static bool isInitialized = false;
   static XLaccess xlChanMaskTx = 0;
   XLstatus xlStatus;
   unsigned int xlChanIndex = 0;
   unsigned int txID = 0x01;

   if (!isInitialized) 
   {
    xlStatus = CAN_DriverInit(&xlChanMaskTx, &xlChanIndex);
    printf("- Init             : %s\n", xlGetErrorString(xlStatus));
    if (XL_SUCCESS == xlStatus) 
    {
        xlStatus = CAN_CreateRxThread();
        printf("- Create RX thread : %s\n", xlGetErrorString(xlStatus));
    }
    if (XL_SUCCESS == xlStatus) 
    {
        xlStatus = xlActivateChannel(g_xlPortHandle, g_xlChannelMask, XL_BUS_TYPE_CAN, XL_ACTIVATE_RESET_CLOCK);
        printf("- ActivateChannel  : CM=0x%I64x, %s\n", g_xlChannelMask, xlGetErrorString(xlStatus));
    }
    isInitialized = true;
   }
}

XLstatus CAN_Transmit(uint32_t txID, XLaccess xlChanMaskTx, uint8_t *payload, uint8_t dataLength)
{
    XLstatus             xlStatus;
    unsigned int         messageCount = 1;
    static int           cnt = 0;

    if (CANType) {
        unsigned int  fl[3] = {

          0 , // CAN (no FD)
          XL_CAN_TXMSG_FLAG_EDL,
          XL_CAN_TXMSG_FLAG_EDL | XL_CAN_TXMSG_FLAG_BRS,
        };

        XLcanTxEvent canTxEvt;
        unsigned int cntSent;
        unsigned int i;
        unsigned char buffer[64];

        memset(&canTxEvt, 0, sizeof(canTxEvt));

        canTxEvt.tag = XL_CAN_EV_TAG_TX_MSG;
        canTxEvt.tagData.canMsg.canId = txID;
        canTxEvt.tagData.canMsg.msgFlags = fl[cnt % (sizeof(fl) / sizeof(fl[0]))];
        canTxEvt.tagData.canMsg.dlc = 8;

    //   if EDL is set, demonstrate transmit with DLC=15 (64 bytes) (16*4 bytes)
        if (canTxEvt.tagData.canMsg.msgFlags & XL_CAN_TXMSG_FLAG_EDL) {
            canTxEvt.tagData.canMsg.dlc = 15;
        }
        ++cnt;
        memcpy(canTxEvt.tagData.canMsg.data, payload, dataLength * sizeof(unsigned char));
        xlStatus = xlCanTransmitEx(g_xlPortHandle, xlChanMaskTx, messageCount, &cntSent, &canTxEvt);

    }
    else {
        static XLevent       xlEvent;

        memset(&xlEvent, 0, sizeof(xlEvent));

        xlEvent.tag = XL_TRANSMIT_MSG;
        xlEvent.tagData.msg.id = txID;
        xlEvent.tagData.msg.dlc = 8;
        xlEvent.tagData.msg.flags = 0;

        memcpy(xlEvent.tagData.msg.data, payload, dataLength * sizeof(unsigned char));

        xlStatus = xlCanTransmit(g_xlPortHandle, xlChanMaskTx, &messageCount, &xlEvent);
        //printf(": PH(0x%x), %s\n", g_xlPortHandle);

    }

    printf("\n- Transmit         : CM(0x%I64x), %s\n", xlChanMaskTx, xlGetErrorString(xlStatus));

    return xlStatus;
}

XLstatus vMCAL_Stop(void)
{
    XLstatus           xlStatus; 

    xlStatus = xlDeactivateChannel(g_xlPortHandle, g_xlChannelMask);
    
    if (xlStatus == XL_SUCCESS && g_xlPortHandle != XL_INVALID_PORTHANDLE)
    {
        xlStatus = xlClosePort(g_xlPortHandle);
    }   
    
    g_xlPortHandle = XL_INVALID_PORTHANDLE;
    xlCloseDriver();
    printf("vMCAL (XLDriver) stopped!");
    return XL_SUCCESS;
}


int main() {
    // Khởi tạo CAN
    vMCAL_CAN_Init();

    // Chờ một chút để đảm bảo hệ thống khởi tạo xong
    Sleep(1000);

    // Thiết lập ID và dữ liệu truyền
    uint32_t txID = 0x123;  // ID của CAN message
    XLaccess xlChanMaskTx = g_xlChannelMask; // Kênh truyền đã được lấy từ quá trình khởi tạo
    uint8_t payload[12] = {0x0C, 0x10, 0x34, 0x01, 0x12, 0x00, 0xBC, 0x61, 0x4E, 0x11, 0x22, 0x33}; // Dữ liệu cần truyền
    uint8_t dataLength = sizeof(payload);

    while (1){
 // Gửi dữ liệu lên CAN Bus
    printf("🚀 Gửi CAN Message...\n");
    XLstatus status = CAN_Transmit(txID, xlChanMaskTx, payload, dataLength);

    if (status == XL_SUCCESS) {
        printf("✅ Gửi CAN Message thành công!\n");
    } else {
        printf("❌ Gửi CAN Message thất bại! Lỗi: %s\n", xlGetErrorString(status));
    }

    // Chờ một chút để đảm bảo hệ thống khởi tạo xong
    // Sleep(2000);
    }
   
    // Dừng CAN trước khi thoát
    vMCAL_Stop();
    
    return 0;
}
