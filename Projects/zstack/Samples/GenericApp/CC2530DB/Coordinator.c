/*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
* 文件名  ： Coordinator
* 作者    ： 黄舒怡、李梦婷、郑一凡
* 版本    ： V1.0.0
* 时间    ： 2021/5/10
* 简要    ： 协调器驱动文件
********************************************************************
* 副本
*
*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>*/
/* 头文件 ----------------------------------------------------------------*/
#include "OSAL.h"
#include "AF.h"
#include "ZDApp.h"
#include "ZDObject.h"
#include "ZDProfile.h"
#include <String.h>
#include "Coordinator.h"
#include "DebugTrace.h"
#if !defined( WIN32 )
#include "OnBoard.h"
#endif
#include "hal_lcd.h"
#include "hal_led.h"
#include "hal_key.h"
#include "hal_uart.h"
/* 宏定义 ----------------------------------------------------------------*/
/* 结构体或枚举 ----------------------------------------------------------*/
//演示如何实现点对点通信
const cId_t GenericApp_ClusterList[GENERICAPP_MAX_CLUSTERS] = 
{
    GENERICAPP_CLUSTERID
};
//简单设备描述
const SimpleDescriptionFormat_t GenericApp_SimpleDesc = 
{
    GENERICAPP_ENDPOINT,
    GENERICAPP_PROFID,
    GENERICAPP_DEVICEID,
    GENERICAPP_DEVICE_VERSION,
    GENERICAPP_FLAGS,
    GENERICAPP_MAX_CLUSTERS,
    (cId_t *)GenericApp_ClusterList,
    0,
    (cId_t *)NULL
};
//定义变量
endPointDesc_t GenericApp_epDesc;//节点描述符
byte GenericApp_TaskID;//任务优先级
byte GenericApp_TransID;//数据发送序列号
unsigned char uartbuf[128];
/* 内部函数声明 ----------------------------------------------------------*/
void GenericApp_MessageMSGCB( afIncomingMSGPacket_t *pckt );//消息处理函数声明
void GenericApp_SendTheMessage( void );//数据发送函数声明
static void rxCB( uint8 port,uint8 event );//回调函数声明
/* 函数 ------------------------------------------------------------------*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* 函数名  ： GenericApp_Init
* 参数    ： byte task_id
* 返回    ： void
* 作者    ： 黄舒怡、李梦婷、郑一凡
* 时间    ： 2021/5/10
* 描述    ： 任务初始化函数
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void GenericApp_Init( byte task_id )
{
    halUARTCfg_t uartConfig;
    GenericApp_TaskID             = task_id;//任务优先级初始化
    GenericApp_TransID            = 0;//数据发送序列号初始化
    //节点描述初始化
    GenericApp_epDesc.endPoint    = GENERICAPP_ENDPOINT;
    GenericApp_epDesc.task_id     = &GenericApp_TaskID;
    GenericApp_epDesc.simpleDesc  =
        (SimpleDescriptionFormat_t *)&GenericApp_SimpleDesc;
    GenericApp_epDesc.latencyReq  = noLatencyReqs;
    afRegister( &GenericApp_epDesc );//将节点描述符进行注册
    uartConfig.configured         = TRUE;//配置uart
    uartConfig.baudRate           = HAL_UART_BR_115200;//波特率
    uartConfig.flowControl        = FALSE;//流控制
    uartConfig.callBackFunc       = rxCB;//回调函数
    HalUARTOpen(0, &uartConfig);//串口初始化
}
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* 函数名  ： GenericApp_ProcessEvent
* 参数    ： byte task_id, Uint16 event
* 返回    ： UINT16
* 作者    ： 黄舒怡、李梦婷、郑一凡
* 时间    ： 2021/5/10
* 描述    ： 消息接收处理函数
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
UINT16 GenericApp_ProcessEvent( byte task_id, UINT16 events )
{
    afIncomingMSGPacket_t *MSGpkt;//定义指向接收消息结构体的指针
    if ( events & SYS_EVENT_MSG )
    {
        MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive(GenericApp_TaskID );//从消息队列上接收消息
        while ( MSGpkt )
        {
            switch ( MSGpkt->hdr.event )
            {
                case AF_INCOMING_MSG_CMD://对是否接收到消息进行判断
                    GenericApp_MessageMSGCB( MSGpkt );
                    break;
                default:
                    break;
            }
            osal_msg_deallocate( (uint8 *)MSGpkt );//处理接收到的消息后，释放存储空间
            MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive
                ( GenericApp_TaskID );//再从消息队列里接收消息
        }
        return (events ^ SYS_EVENT_MSG);
    }
    return 0;
}
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* 函数名  ： rxCB
* 参数    ： uint8 port, uint8 event
* 返回    ： void
* 作者    ： 黄舒怡、李梦婷、郑一凡
* 时间    ： 2021/5/11
* 描述    ： 回调函数
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
static void rxCB(uint8 port, uint8 event)
{
    HalUARTRead(0, uartbuf,16);//从串口中读取数据并存放在uartbuf中
    if(osal_memcmp(uartbuf,"www.wlwmaker.com",16))//判断接收到的字符
    {
        HalUARTWrite(0, uartbuf,16);//讲接收到的字符输出到串口
    }
}
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* 函数名  ： GenericApp_MessageMSGCB
* 参数    ： afIncomingMSGPacket_t *pkt 
* 返回    ： void
* 作者    ： 黄舒怡、李梦婷、郑一凡
* 时间    ： 2021/5/10
* 描述    ： 消息读取处理函数
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void GenericApp_MessageMSGCB( afIncomingMSGPacket_t *pkt )
{
    unsigned char buffer[4] = "    ";
    switch ( pkt->clusterId )
    {
        case GENERICAPP_CLUSTERID:
            osal_memcpy(buffer,pkt->cmd.Data,3);//将接收到的数据拷贝到缓冲区buffer
            if((buffer[0] == 'L') || (buffer[1] == 'E') || (buffer[2] == 'D'))
            {
                HalLedBlink(HAL_LED_2,0,50,500);//led2闪烁
            }
            else
            {
                HalLedSet(HAL_LED_2,HAL_LED_MODE_ON);//led2亮
            }
        break;
    }
}


