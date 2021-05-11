/*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
* �ļ���  �� Coordinator
* ����    �� �������������á�֣һ��
* �汾    �� V1.0.0
* ʱ��    �� 2021/5/10
* ��Ҫ    �� Coordinatorͷ�ļ�  
********************************************************************
* ����
*
*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>*/
#ifndef COORDINATOR_H
#define COORDINATOR_H
/* ͷ�ļ� ----------------------------------------------------------------*/
#include "ZComDef.h"
/* �궨�� ----------------------------------------------------------------*/
#define GENERICAPP_ENDPOINT             10
#define GENERICAPP_PROFID               0x0F04
#define GENERICAPP_DEVICEID             0x0001
#define GENERICAPP_DEVICE_VERSION       0
#define GENERICAPP_FLAGS                0
#define GENERICAPP_MAX_CLUSTERS         1
#define GENERICAPP_CLUSTERID            1
/* �ṹ���ö�� ----------------------------------------------------------*/
/* ��������---------------------------------------------------------------*/
extern void GenericApp_Init( byte task_id );
extern UINT16 GenericApp_ProcessEvent( byte task_id, UINT16 events );
/* �ⲿ�������� ----------------------------------------------------------*/


#endif // _COORDINATOR_H   

