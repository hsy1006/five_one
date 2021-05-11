/*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
* �ļ���  �� Enddevice
* ����    �� �������������á�֣һ��
* �汾    �� V1.0.0
* ʱ��    �� 2021/5/10
* ��Ҫ    �� �ն˽ڵ������ļ�
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
#include <string.h>
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
    0,
    (cId_t *)NULL,
    GENERICAPP_MAX_CLUSTERS,
    (cId_t *)GenericApp_ClusterList
};
//�����ĸ�����
endPointDesc_t GenericApp_epDesc;//�ڵ�������
byte GenericApp_TaskID;//�������ȼ�
byte GenericApp_TransID;//���ݷ������к�
devStates_t GenericApp_NwkState;//����ڵ�״̬
/* �ڲ��������� ----------------------------------------------------------*/
void GenericApp_MessageMSGCB( afIncomingMSGPacket_t *pckt );//��Ϣ����������
void GenericApp_SendTheMessage( void );//���ݷ��ͺ�������
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
    GenericApp_TaskID             = task_id;//���ȼ���ʼ��
    GenericApp_NwkState           = DEV_INIT;//��ʼ���豸״̬
    GenericApp_TransID            = 0;//���ݷ������кų�ʼ��
    //�ڵ�������ʼ��
    GenericApp_epDesc.endPoint    = GENERICAPP_ENDPOINT;
    GenericApp_epDesc.task_id     = &GenericApp_TaskID;
    GenericApp_epDesc.simpleDesc  =
        (SimpleDescriptionFormat_t *)&GenericApp_SimpleDesc;
    GenericApp_epDesc.latencyReq  = noLatencyReqs;
    afRegister( &GenericApp_epDesc );//���ڵ�����������ע��
}
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* ������  �� GenericApp_ProcessEvent
* ����    �� byte task_id, Uint16 event
* ����    �� UINT16
* ����    �� �������������á�֣һ��
* ʱ��    �� 2021/5/10
* ����    �� ��Ϣ������
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
                case ZDO_STATE_CHANGE:
                    GenericApp_NwkState = (devStates_t)(MSGpkt->hdr.status);//��ȡ�ڵ��豸����
                    if (GenericApp_NwkState == DEV_END_DEVICE)//ʵ�����ݷ���
                    {
                        GenericApp_SendTheMessage() ;
                    }
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
* ������  �� GenericApp_SendTheMessage
* ����    �� void
* ����    �� void
* ����    �� �������������á�֣һ��
* ʱ��    �� 2021/5/10
* ����    �� ��Ϣ���ͺ���
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void GenericApp_SendTheMessage( void )
{
    unsigned char theMessageData[4] = "LED";//���Ҫ���͵�����
    afAddrType_t my_DstAddr;
    my_DstAddr.addrMode = (afAddrMode_t)Addr16Bit;
    my_DstAddr.endPoint = GENERICAPP_ENDPOINT;
    my_DstAddr.addr.shortAddr = 0x0000;
    AF_DataRequest( &my_DstAddr, &GenericApp_epDesc,GENERICAPP_CLUSTERID, 3,
                       theMessageData,
                       &GenericApp_TransID,
                       AF_DISCV_ROUTE,
                       AF_DEFAULT_RADIUS );//�������ݵķ���
    HalLedBlink(HAL_LED_2,0,50,500);
}
                                 
    
                    