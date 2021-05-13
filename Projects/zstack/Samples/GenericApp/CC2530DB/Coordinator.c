/*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
* �ļ���  �� Coordinator
* ����    �� �������������á�֣һ��
* �汾    �� V1.0.0
* ʱ��    �� 2021/5/10
* ��Ҫ    �� Э���������ļ�
********************************************************************
* ����
*
*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>*/
/* ͷ�ļ� ----------------------------------------------------------------*/
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
/* �궨�� ----------------------------------------------------------------*/
/* �ṹ���ö�� ----------------------------------------------------------*/
//��ʾ���ʵ�ֵ�Ե�ͨ��
const cId_t GenericApp_ClusterList[GENERICAPP_MAX_CLUSTERS] = 
{
    GENERICAPP_CLUSTERID
};
//���豸����
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
//�������
endPointDesc_t GenericApp_epDesc;//�ڵ�������
byte GenericApp_TaskID;//�������ȼ�
byte GenericApp_TransID;//���ݷ������к�
unsigned char uartbuf[128];
/* �ڲ��������� ----------------------------------------------------------*/
void GenericApp_MessageMSGCB( afIncomingMSGPacket_t *pckt );//��Ϣ����������
void GenericApp_SendTheMessage( void );//���ݷ��ͺ�������
static void rxCB( uint8 port,uint8 event );//�ص���������
/* ���� ------------------------------------------------------------------*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* ������  �� GenericApp_Init
* ����    �� byte task_id
* ����    �� void
* ����    �� �������������á�֣һ��
* ʱ��    �� 2021/5/10
* ����    �� �����ʼ������
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void GenericApp_Init( byte task_id )
{
    halUARTCfg_t uartConfig;
    GenericApp_TaskID             = task_id;//�������ȼ���ʼ��
    GenericApp_TransID            = 0;//���ݷ������кų�ʼ��
    //�ڵ�������ʼ��
    GenericApp_epDesc.endPoint    = GENERICAPP_ENDPOINT;
    GenericApp_epDesc.task_id     = &GenericApp_TaskID;
    GenericApp_epDesc.simpleDesc  =
        (SimpleDescriptionFormat_t *)&GenericApp_SimpleDesc;
    GenericApp_epDesc.latencyReq  = noLatencyReqs;
    afRegister( &GenericApp_epDesc );//���ڵ�����������ע��
    uartConfig.configured         = TRUE;//����uart
    uartConfig.baudRate           = HAL_UART_BR_115200;//������
    uartConfig.flowControl        = FALSE;//������
    uartConfig.callBackFunc       = rxCB;//�ص�����
    HalUARTOpen(0, &uartConfig);//���ڳ�ʼ��
}
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* ������  �� GenericApp_ProcessEvent
* ����    �� byte task_id, Uint16 event
* ����    �� UINT16
* ����    �� �������������á�֣һ��
* ʱ��    �� 2021/5/10
* ����    �� ��Ϣ���մ�����
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
UINT16 GenericApp_ProcessEvent( byte task_id, UINT16 events )
{
    afIncomingMSGPacket_t *MSGpkt;//����ָ�������Ϣ�ṹ���ָ��
    if ( events & SYS_EVENT_MSG )
    {
        MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive(GenericApp_TaskID );//����Ϣ�����Ͻ�����Ϣ
        while ( MSGpkt )
        {
            switch ( MSGpkt->hdr.event )
            {
                case AF_INCOMING_MSG_CMD://���Ƿ���յ���Ϣ�����ж�
                    GenericApp_MessageMSGCB( MSGpkt );
                    break;
                default:
                    break;
            }
            osal_msg_deallocate( (uint8 *)MSGpkt );//������յ�����Ϣ���ͷŴ洢�ռ�
            MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive
                ( GenericApp_TaskID );//�ٴ���Ϣ�����������Ϣ
        }
        return (events ^ SYS_EVENT_MSG);
    }
    return 0;
}
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* ������  �� rxCB
* ����    �� uint8 port, uint8 event
* ����    �� void
* ����    �� �������������á�֣һ��
* ʱ��    �� 2021/5/11
* ����    �� �ص�����
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
static void rxCB(uint8 port, uint8 event)
{
    HalUARTRead(0, uartbuf,16);//�Ӵ����ж�ȡ���ݲ������uartbuf��
    if(osal_memcmp(uartbuf,"www.wlwmaker.com",16))//�жϽ��յ����ַ�
    {
        HalUARTWrite(0, uartbuf,16);//�����յ����ַ����������
    }
}
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* ������  �� GenericApp_MessageMSGCB
* ����    �� afIncomingMSGPacket_t *pkt 
* ����    �� void
* ����    �� �������������á�֣һ��
* ʱ��    �� 2021/5/10
* ����    �� ��Ϣ��ȡ������
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void GenericApp_MessageMSGCB( afIncomingMSGPacket_t *pkt )
{
    unsigned char buffer[4] = "    ";
    switch ( pkt->clusterId )
    {
        case GENERICAPP_CLUSTERID:
            osal_memcpy(buffer,pkt->cmd.Data,3);//�����յ������ݿ�����������buffer
            if((buffer[0] == 'L') || (buffer[1] == 'E') || (buffer[2] == 'D'))
            {
                HalLedBlink(HAL_LED_2,0,50,500);//led2��˸
            }
            else
            {
                HalLedSet(HAL_LED_2,HAL_LED_MODE_ON);//led2��
            }
        break;
    }
}


