/**************************************************************************************
Exclusion of Liability for this demo software:
  The following software is intended for and must only be used for reference and in an
  evaluation laboratory environment. It is provided without charge and is subject to
  alterations. There is no warranty for the software, to the extent permitted by
  applicable law. Except when otherwise stated in writing the copyright holders and/or
  other parties provide the software "as is" without warranty of any kind, either
  expressed or implied.
  Please refer to the Agreement in README_DISCLAIMER.txt, provided together with this file!
  By installing or otherwise using the software, you accept the terms of this Agreement.
  If you do not agree to the terms of this Agreement, then do not install or use the
  Software!
**************************************************************************************/

/**************************************************************************************

Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.

***************************************************************************************/
/* Includes */

#include <stdio.h>

#include "AppEIS_DemoApplication.h"
#include "App_DemoApplication.h"
#include "App_PacketCommunication.h"
#include "App_SystemPackets.h"
#include "hostAbstractionLayer.h"
#include "Hil_ApplicationCmd.h"
#include "Hil_Results.h"
#include "Hil_Packet.h"
#include "Hil_DualPortMemory.h"
#include "Hil_SystemCmd.h"
#include "App_EventHandler.h"

#include "cip_id.h"
#include "eip_tcp.h"
#include "eip_en.h"
#include "eip_qos.h"
#include "cip_as.h"
#include "cip_pdc.h"
#include "cip_common.h"
#include "EipAps_Public.h"

/*****************************************************************************************************************************************/
/*****************************************************************************************************************************************/
/* Configuration related definitions */

#define IP_ADDRESS(a,b,c,d) (uint32_t)(((a) << 24) | ((b) << 16) | ((c) << 8) | (d))

/*****************************************************************************************************************************************/
/* CIP Identity Object - Class 0x01 - Configuration                                                                                      */
/*****************************************************************************************************************************************/

#define VENDOR_ID                            CIP_VENDORID_HILSCHER
#define DEVICE_TYPE_COMMUNICATION_ADAPTER    0x0C   /* CIP Profile - "Communication Adapter" */
#define PRODUCT_CODE                         PRODUCT_CODE_EIS_EXTENDED_DEMO_2
#define MAJOR_REVISION                       1
#define MINOR_REVISION                       1
char    abProductName[] = "\x1B" "EIS_V5_EXTENDED_CONFIG_DEMO";

static uint16_t usVendor       = VENDOR_ID;                               /* Attribute  1: Hilscher Vendor ID                   */
static uint16_t usDeviceType   = DEVICE_TYPE_COMMUNICATION_ADAPTER;       /* Attribute  2: Communications Device                */
static uint16_t usProductCode  = PRODUCT_CODE;                            /* Attribute  3: Vendor specific Product Code         */
static uint16_t usRevision     = (MAJOR_REVISION << 8) | MINOR_REVISION;  /* Attribute  4: Major: 1, Minor : 1                  */

/*************************************************************************************/
/* Assembly instances that shall be supported                                        */
/*************************************************************************************/

#define EIP_DEMO_CONSUMING_ASSEMBLY_INSTANCE            0x64
#define EIP_DEMO_CONSUMING_ASSEMBLY_INSTANCE_SIZE       sizeof(APP_PROCESS_DATA_INPUT_T)

#define EIP_DEMO_PRODUCING_ASSEMBLY_INSTANCE            0x65
#define EIP_DEMO_PRODUCING_ASSEMBLY_INSTANCE_SIZE       sizeof(APP_PROCESS_DATA_OUTPUT_T)

#define EIP_DEMO_CONFIGURATION_ASSEMBLY_INSTANCE        0x66
#define EIP_DEMO_CONFIGURATION_ASSEMBLY_INSTANCE_SIZE   4

#define EIP_DEMO_LISTENONLY_ASSEMBLY_INSTANCE           0xC0
#define EIP_DEMO_INPUTONLY_ASSEMBLY_INSTANCE            0xC1

/* Default data of configuration assembly --> Adapt as needed! */
static uint8_t s_abConfigAssemblyData[EIP_DEMO_CONFIGURATION_ASSEMBLY_INSTANCE_SIZE] = {0x11, 0x22, 0x33, 0x44};

/*****************************************************************************************************************************************/
/* TCP/IP Interface Object - Class 0xF5 - Configuration                                                                                  */
/*****************************************************************************************************************************************/

uint32_t                    ulConfigCapability = EIP_TCP_CAP_BOOTP                           /* Attribute  2 */
                                                | EIP_TCP_CAP_DHCP
                                                | EIP_TCP_CAP_CNFSET
                                                | EIP_TCP_CAP_ACD_CAPABLE;

uint32_t                    ulConfigControl = EIP_TCP_CNTRL_DHCP;                            /* Attribute  3 */

EIP_TCP_INTERFACE_CONFIG_T  tInterfaceConfig         = { IP_ADDRESS(192,168,0,10),      /* Attribute  5                  */
                                                                                        /* IP :           192.168.0.10   */
                                                         IP_ADDRESS(255,255,255,0),     /* Subnet :       255.255.255.0  */
                                                         IP_ADDRESS(0,0,0,0),           /* Gateway:       0.0.0.0.       */
                                                         IP_ADDRESS(0,0,0,0),           /* Name Server1 : 0.0.0.0.       */
                                                         IP_ADDRESS(0,0,0,0),           /* Name Server2 : 0.0.0.0.       */
                                                         {0x0, 0x0} };                  /* Domain Name  : ""             */

uint8_t                     abHostname[EIP_TCP_MAX_HOSTNAME_LENGTH + 2];                /* Host name */
EIP_TCP_MCAST_CONFIG_T      tMcastConfig;                                               /* Multicast configuration */
EIP_TCP_ACD_LAST_CONFLICT_T tLastConflictDetected;                                      /* Last detected IP conflict  */




uint8_t                     bTTLValue = 1;                                              /* Attribute  8 */

uint8_t                     bSelectAcd = 1;                                             /* Attribute 10 */

uint16_t                    usEncapInactivityTimeout = EIP_TCP_ENCAP_TIMEOUT_DEFAULT;                 /* Attribute 13 */


/*****************************************************************************************************************************************/
/* Ethernet Link Object - Class 0xF6 - Configuration                                                                                      */
/*****************************************************************************************************************************************/

uint32_t ulInterfaceControl0 = 0x1;                         /* Interface control for port 0: Autoneg */
uint32_t ulInterfaceControl1 = 0x1;                         /* Interface control for port 1: Autoneg */

uint8_t  bInterfaceType0 = EIP_EN_INTF_TYPE_TWISTEDPAIR;   /* Interface type for port 0 */
uint8_t  bInterfaceType1 = EIP_EN_INTF_TYPE_TWISTEDPAIR;   /* Interface type for port 1 */

uint8_t bAdminState0 = 1;         /* Admin state for port 0 : port enabled */
uint8_t bAdminState1 = 1;         /* Admin state for port 1 : port enabled */

uint8_t  abInterfaceLabel1[] = "\x05" "Port0";                 /* Attribute 10 - Port 0                                                                 */
uint8_t  abInterfaceLabel2[] = "\x05" "Port1";                 /* Attribute 10 - Port 1                                                                 */
uint8_t  bMDIXCfg = EIP_EN_INTF_MDIX_AUTO;        /* Attribute 300 - vendor specific attribute to set MDI settings (used for port 0 and 1) */


/*****************************************************************************************************************************************/
/* QoS Object - Configuration                                                                                                            */
/*****************************************************************************************************************************************/

uint8_t bDSCP_PTP_Event     = EIP_QOS_DSCP_PTP_EVENT_DEFAULT;
uint8_t bDSCP_PTP_General   = EIP_QOS_DSCP_PTP_GENERAL_DEFAULT;
uint8_t bDSCP_Urgent        = EIP_QOS_DSCP_URGENT_DEFAULT;
uint8_t bDSCP_Scheduled     = EIP_QOS_DSCP_SCHEDULED_DEFAULT;
uint8_t bDSCP_High          = EIP_QOS_DSCP_HIGH_DEFAULT;
uint8_t bDSCP_Low           = EIP_QOS_DSCP_LOW_DEFAULT;
uint8_t bDSCP_Explicit      = EIP_QOS_DSCP_EXPLICIT_DEFAULT;

/*****************************************************************************************************************************************/
/* List of object attributes that will be written during the configuration process.                                                      */
/* These configuration parameters represent the factory default settings.                                                                */
/*****************************************************************************************************************************************/

CIP_OBJ_DIR_T s_atCipObjDirConfigValues[] =
{
    /* Class,              Instance, Attribute,                                           DataSize,                                                      pointer to data */
    /* Identity Object */
    { CIP_ID_CLASS_NUMBER,  1,        CIP_ID_ATTR_1_VENDOR_ID,                             sizeof(usVendor),                                              (uint8_t*)&usVendor },
    { CIP_ID_CLASS_NUMBER,  1,        CIP_ID_ATTR_2_DEVICE_TYPE,                           sizeof(usDeviceType),                                          (uint8_t*)&usDeviceType },
    { CIP_ID_CLASS_NUMBER,  1,        CIP_ID_ATTR_3_PRODUCT_CODE,                          sizeof(usProductCode),                                         (uint8_t*)&usProductCode },
    { CIP_ID_CLASS_NUMBER,  1,        CIP_ID_ATTR_4_REVISION,                              sizeof(usRevision),                                            (uint8_t*)&usRevision },

    /* The serial number attribute is not settable anymore via CIP means.
       A user serial number may be set into the DDP (Device Data Provider) in a taglist-modified firmware (DDP_PASSIVE).
       Per default, the EtherNet/IP firmware will use the serial number located in the FDL (Flash Device Label)
    */

    { CIP_ID_CLASS_NUMBER,  1,        CIP_ID_ATTR_7_PRODUCT_NAME,                          sizeof(abProductName)-1,                                       (uint8_t*)&abProductName[0] },

    /* TCP/IP Interface Object */
    { EIP_TCP_CLASS_NUMBER, 1,        EIP_TCP_ATTR_2_CFG_CAPABILITY,                       sizeof(ulConfigCapability),                                    (uint8_t*)&ulConfigCapability },
    { EIP_TCP_CLASS_NUMBER, 1,        EIP_TCP_ATTR_3_CFG_CONTROL,                          sizeof(ulConfigControl),                                       (uint8_t*)&ulConfigControl },
    { EIP_TCP_CLASS_NUMBER, 1,        EIP_TCP_ATTR_5_INTERFACE_CFG,                        sizeof(tInterfaceConfig) - EIP_TCP_MAX_DOMAINNAME_LENGTH,      (uint8_t*)&tInterfaceConfig },
    { EIP_TCP_CLASS_NUMBER, 1,        EIP_TCP_ATTR_6_HOST_NAME,                            2,                                                             (uint8_t*)&abHostname[0] },
    { EIP_TCP_CLASS_NUMBER, 1,        EIP_TCP_ATTR_8_TTL_VALUE,                            sizeof(bTTLValue),                                             (uint8_t*)&bTTLValue },
    { EIP_TCP_CLASS_NUMBER, 1,        EIP_TCP_ATTR_9_MCAST_CFG,                            sizeof(tMcastConfig),                                          (uint8_t*)&tMcastConfig },
    { EIP_TCP_CLASS_NUMBER, 1,        EIP_TCP_ATTR_10_SELECT_ACD,                          sizeof(bSelectAcd),                                            (uint8_t*)&bSelectAcd },
    { EIP_TCP_CLASS_NUMBER, 1,        EIP_TCP_ATTR_11_LAST_CONFLICT_DETECTED,              sizeof(tLastConflictDetected),                                 (uint8_t*)&tLastConflictDetected },
    { EIP_TCP_CLASS_NUMBER, 1,        EIP_TCP_ATTR_13_ENCAP_TIMEOUT,                       sizeof(usEncapInactivityTimeout),                              (uint8_t*)&usEncapInactivityTimeout },

    /* Ethernet Link Object */
    { EIP_EN_CLASS_NUMBER,  1,        EIP_EN_ATTR_6_INTERFACE_CONTROL,                     sizeof(ulInterfaceControl0),                                   (uint8_t*)&ulInterfaceControl0 },
    { EIP_EN_CLASS_NUMBER,  2,        EIP_EN_ATTR_6_INTERFACE_CONTROL,                     sizeof(ulInterfaceControl1),                                   (uint8_t*)&ulInterfaceControl1 },
    { EIP_EN_CLASS_NUMBER,  1,        EIP_EN_ATTR_7_INTERFACE_TYPE,                        sizeof(bInterfaceType0),                                       (uint8_t*)&bInterfaceType0 },
    { EIP_EN_CLASS_NUMBER,  2,        EIP_EN_ATTR_7_INTERFACE_TYPE,                        sizeof(bInterfaceType1),                                       (uint8_t*)&bInterfaceType1 },
    { EIP_EN_CLASS_NUMBER,  1,        EIP_EN_ATTR_9_ADMIN_STATE,                           sizeof(bAdminState0),                                          (uint8_t*)&bAdminState0 },
    { EIP_EN_CLASS_NUMBER,  2,        EIP_EN_ATTR_9_ADMIN_STATE,                           sizeof(bAdminState1),                                          (uint8_t*)&bAdminState1 },
    { EIP_EN_CLASS_NUMBER,  1,        EIP_EN_ATTR_10_INTERFACE_LABEL,                      sizeof(abInterfaceLabel1)-1,                                   (uint8_t*)&abInterfaceLabel1[0] },
    { EIP_EN_CLASS_NUMBER,  2,        EIP_EN_ATTR_10_INTERFACE_LABEL,                      sizeof(abInterfaceLabel2)-1,                                   (uint8_t*)&abInterfaceLabel2[0] },
    { EIP_EN_CLASS_NUMBER,  1,        EIP_EN_ATTR_768_MDIX_CONFIG,                         sizeof(bMDIXCfg),                                              (uint8_t*)&bMDIXCfg },
    { EIP_EN_CLASS_NUMBER,  2,        EIP_EN_ATTR_768_MDIX_CONFIG,                         sizeof(bMDIXCfg),                                              (uint8_t*)&bMDIXCfg },

    /* Quality of Service Object */
    { EIP_QOS_CLASS_NUMBER, 1,        EIP_QOS_ATTR_2_DSCP_PTP_EVENT,                       sizeof(bDSCP_PTP_Event),                                       (uint8_t*)&bDSCP_PTP_Event },
    { EIP_QOS_CLASS_NUMBER, 1,        EIP_QOS_ATTR_3_DSCP_PTP_GENERAL,                     sizeof(bDSCP_PTP_General),                                     (uint8_t*)&bDSCP_PTP_General },
    { EIP_QOS_CLASS_NUMBER, 1,        EIP_QOS_ATTR_4_DSCP_URGENT,                          sizeof(bDSCP_Urgent),                                          (uint8_t*)&bDSCP_Urgent },
    { EIP_QOS_CLASS_NUMBER, 1,        EIP_QOS_ATTR_5_DSCP_SCHEDULED,                       sizeof(bDSCP_Scheduled),                                       (uint8_t*)&bDSCP_Scheduled },
    { EIP_QOS_CLASS_NUMBER, 1,        EIP_QOS_ATTR_6_DSCP_HIGH,                            sizeof(bDSCP_High),                                            (uint8_t*)&bDSCP_High },
    { EIP_QOS_CLASS_NUMBER, 1,        EIP_QOS_ATTR_7_DSCP_LOW,                             sizeof(bDSCP_Low),                                             (uint8_t*)&bDSCP_Low },
    { EIP_QOS_CLASS_NUMBER, 1,        EIP_QOS_ATTR_8_DSCP_EXPLICIT,                        sizeof(bDSCP_Explicit),                                        (uint8_t*)&bDSCP_Explicit },

    /* Default data of configuration assembly instance */
    { CIP_AS_CLASS_NUMBER,  EIP_DEMO_CONFIGURATION_ASSEMBLY_INSTANCE, CIP_AS_ATTR_3_DATA,  sizeof(s_abConfigAssemblyData),                                (uint8_t*)s_abConfigAssemblyData },
};

/* Number of entries in s_atCipObjDirConfigValues */
uint32_t s_ulCipObjDirEntries = sizeof(s_atCipObjDirConfigValues)/sizeof(s_atCipObjDirConfigValues[0]);

/* Pointer to s_atCipObjDirConfigValues */
CIP_OBJ_DIR_T* s_ptCipObjDirConfigValues = &s_atCipObjDirConfigValues[0];


/*****************************************************************************************************************************************/
/* List of assembly instances that shall be created/registered                                                                           */
/*****************************************************************************************************************************************/

CIP_ASSEMBLY_DIR_T s_atCipAssemblyDir[] =
{
    /* Instance,                                 Size,                                            Offset,    Flags */
    { EIP_DEMO_CONSUMING_ASSEMBLY_INSTANCE,     EIP_DEMO_CONSUMING_ASSEMBLY_INSTANCE_SIZE,        0,         EIP_AS_TYPE_INPUT | EIP_AS_OPTION_FIXEDSIZE },
    { EIP_DEMO_PRODUCING_ASSEMBLY_INSTANCE,     EIP_DEMO_PRODUCING_ASSEMBLY_INSTANCE_SIZE,        0,         EIP_AS_TYPE_OUTPUT | EIP_AS_OPTION_FIXEDSIZE | EIP_AS_OPTION_NO_RUNIDLE },
    { EIP_DEMO_CONFIGURATION_ASSEMBLY_INSTANCE, EIP_DEMO_CONFIGURATION_ASSEMBLY_INSTANCE_SIZE,    0,         EIP_AS_TYPE_CONFIG | EIP_AS_OPTION_FIXEDSIZE },
    { EIP_DEMO_LISTENONLY_ASSEMBLY_INSTANCE,    0,                                                0,         EIP_AS_TYPE_LISTENONLY },
    { EIP_DEMO_INPUTONLY_ASSEMBLY_INSTANCE,     0,                                                0,         EIP_AS_TYPE_INPUTONLY },
};

/* Number of assembly instances */
uint32_t s_ulCipAssemblyDirEntries = sizeof(s_atCipAssemblyDir)/sizeof(s_atCipAssemblyDir[0]);

/* Pointer to assembly list */
CIP_ASSEMBLY_DIR_T* s_ptCipAssemblyDir = &s_atCipAssemblyDir[0];

/*****************************************************************************************************************************************/
/* List of connections that shall be supported                                                                                           */
/*****************************************************************************************************************************************/
CIP_SUPP_CONNECTION_DIR_T s_atCipSuppConnectionDir[] =
{
    /* Consume,                               Produce,                                 Config,                                     Connectiontype */
    { EIP_DEMO_CONSUMING_ASSEMBLY_INSTANCE,    EIP_DEMO_PRODUCING_ASSEMBLY_INSTANCE,    EIP_DEMO_CONFIGURATION_ASSEMBLY_INSTANCE,   CIP_CTYPE_EXCLUSIVE_OWNER },
    { EIP_DEMO_LISTENONLY_ASSEMBLY_INSTANCE,   EIP_DEMO_PRODUCING_ASSEMBLY_INSTANCE,    EIP_DEMO_CONFIGURATION_ASSEMBLY_INSTANCE,   CIP_CTYPE_LISTEN_ONLY },
    { EIP_DEMO_INPUTONLY_ASSEMBLY_INSTANCE,    EIP_DEMO_PRODUCING_ASSEMBLY_INSTANCE,    EIP_DEMO_CONFIGURATION_ASSEMBLY_INSTANCE,   CIP_CTYPE_INPUT_ONLY },
};

/* Number of connections */
uint32_t s_ulCipSupConnectionDirEntries = sizeof(s_atCipSuppConnectionDir) / sizeof(s_atCipSuppConnectionDir[0]);

/* Pointer to connection list */
CIP_SUPP_CONNECTION_DIR_T* s_ptCipSuppConnectionDir = &s_atCipSuppConnectionDir[0];


/*****************************************************************************************************************************************/
/* List of additional CIP objects that shall be supported                                                                                */
/*****************************************************************************************************************************************/

/*****************************************************************************************************************************************/
/* List of additional CIP objects that shall be supported                                                                                */
/*****************************************************************************************************************************************/
//#define SUPPORT_CIP_SYNC
#ifdef SUPPORT_CIP_SYNC
static CIP_REGISTER_OBJ_DIR_T s_atCipRegisterObjDir[] =
{
  /* Class,                      OptionFlags,*/
  { CIP_TIMESYNC_CLASS_NUMBER,    EIP_OBJECT_MR_REGISTER_OPTION_FLAGS_USE_OBJECT_PROVIDED_BY_STACK },
};

/* Number of additional object classes to register */
uint32_t s_ulCipRegisterObjDirEntries = sizeof(s_atCipRegisterObjDir) / sizeof(s_atCipRegisterObjDir[0]);

/* Pointer to the list of additional object classes to register */
CIP_REGISTER_OBJ_DIR_T* s_ptCipRegisterObjDir = &s_atCipRegisterObjDir[0];
#else
/* Number of additional object classes to register */
uint32_t s_ulCipRegisterObjDirEntries = 0;

/* Pointer to the list of object classes to register */
CIP_REGISTER_OBJ_DIR_T* s_ptCipRegisterObjDir = NULL;
#endif

/**************************************************************************************/
/**************************************************************************************/

/* register an assembly instance (connection point) */
static uint32_t AppEIS_RegisterAssembly( APP_DATA_T* ptAppData,
                                         uint32_t    ulInstance,
                                         uint32_t    ulSize,
                                         uint32_t    ulOffset,
                                         uint32_t    ulFlags )
{
  uint32_t                             ulRet = CIFX_NO_ERROR;
  EIP_OBJECT_AS_PACKET_REGISTER_REQ_T* ptReq = (EIP_OBJECT_AS_PACKET_REGISTER_REQ_T*)(&ptAppData->aptChannels[EIP_DEMO_CHANNEL_INDEX]->tPacket);

  ptReq->tData.ulInstance  = ulInstance;
  ptReq->tData.ulSize      = ulSize;
  ptReq->tData.ulDPMOffset = ulOffset;
  ptReq->tData.ulFlags     = ulFlags;

  ptReq->tHead.ulDest = HIL_PACKET_DEST_DEFAULT_CHANNEL;
  ptReq->tHead.ulCmd  = EIP_OBJECT_AS_REGISTER_REQ;
  ptReq->tHead.ulLen  = EIP_OBJECT_AS_REGISTER_REQ_SIZE;

  ulRet = Pkt_SendReceivePacket( ptAppData, EIP_DEMO_CHANNEL_INDEX, &ptAppData->aptChannels[EIP_DEMO_CHANNEL_INDEX]->tPacket, TXRX_TIMEOUT );

  return (ulRet != CIFX_NO_ERROR) ? ulRet : ptAppData->aptChannels[EIP_DEMO_CHANNEL_INDEX]->tPacket.tHeader.ulState;
}

/**************************************************************************************/
/**************************************************************************************/

/* Create a connection for a set of connection points */
static uint32_t AppEIS_CreateConnection( APP_DATA_T* ptAppData,
                                         uint32_t    ulConsumerCP,
                                         uint32_t    ulProducerCP,
                                         uint32_t    ulConfigCP,
                                         uint8_t     bConnType )
{
  uint32_t                             ulRet            = CIFX_NO_ERROR;
  EIP_OBJECT_PACKET_CIP_SERVICE_REQ_T* ptReq            = (EIP_OBJECT_PACKET_CIP_SERVICE_REQ_T*)(&ptAppData->aptChannels[EIP_DEMO_CHANNEL_INDEX]->tPacket);
  CIP_PDC_CONNECTION_CFG_T*            ptConnectionData = (CIP_PDC_CONNECTION_CFG_T*)&ptReq->tData.abData[0];

  ptReq->tData.bService    = CIP_CMD_CREATE;
  ptReq->tData.ulClass     = CIP_PDC_CLASS_NUMBER;
  ptReq->tData.ulInstance  = 0;
  ptReq->tData.ulAttribute = 0;
  ptReq->tData.ulMember    = 0;

  ptConnectionData->ulConsumerPoint = ulConsumerCP;
  ptConnectionData->ulProducerPoint = ulProducerCP;
  ptConnectionData->ulConfigPoint   = ulConfigCP;
  ptConnectionData->bConnType       = bConnType;

  ptConnectionData->bSuppTrigTypes = CIP_PDC_TTYPE_CYCLIC | CIP_PDC_TTYPE_COS | CIP_PDC_TTYPE_APPLICATION;
  ptConnectionData->ulMaxOTRpi     = 1000000; /* 1000ms */
  ptConnectionData->ulMaxTORpi     = 1000000; /* 1000ms */
  ptConnectionData->ulMinOTRpi     = 1000;    /* 1ms */
  ptConnectionData->ulMinTORpi     = 1000;    /* 1ms */

  /* issue CIP Service Request */
  ptReq->tHead.ulDest = HIL_PACKET_DEST_DEFAULT_CHANNEL;
  ptReq->tHead.ulCmd  = EIP_OBJECT_CIP_SERVICE_REQ;
  ptReq->tHead.ulLen  = EIP_OBJECT_CIP_SERVICE_REQ_SIZE + sizeof(CIP_PDC_CONNECTION_CFG_T);

  ulRet = Pkt_SendReceivePacket( ptAppData, EIP_DEMO_CHANNEL_INDEX, &ptAppData->aptChannels[EIP_DEMO_CHANNEL_INDEX]->tPacket, TXRX_TIMEOUT );

  return (ulRet != CIFX_NO_ERROR) ? ulRet : ptAppData->aptChannels[EIP_DEMO_CHANNEL_INDEX]->tPacket.tHeader.ulState;
}

/**************************************************************************************/
/**************************************************************************************/

/* Register an additional CIP object class */
static uint32_t AppEIS_RegisterObject( APP_DATA_T* ptAppData,
                                       uint32_t    ulClass,
                                       uint32_t    ulOptionFlags )
{
  uint32_t                             ulRet = CIFX_NO_ERROR;
  EIP_OBJECT_MR_PACKET_REGISTER_REQ_T* ptReq = (EIP_OBJECT_MR_PACKET_REGISTER_REQ_T*)(&ptAppData->aptChannels[EIP_DEMO_CHANNEL_INDEX]->tPacket);

  /* prepare packet to set attribute value */
  ptReq->tData.ulReserved1   = 0;
  ptReq->tData.ulClass       = ulClass;
  ptReq->tData.ulOptionFlags = ulOptionFlags;

  ptReq->tHead.ulDest = HIL_PACKET_DEST_DEFAULT_CHANNEL;
  ptReq->tHead.ulCmd  = EIP_OBJECT_MR_REGISTER_REQ;
  ptReq->tHead.ulLen  = EIP_OBJECT_MR_REGISTER_REQ_SIZE;

  ulRet = Pkt_SendReceivePacket( ptAppData, EIP_DEMO_CHANNEL_INDEX, &ptAppData->aptChannels[EIP_DEMO_CHANNEL_INDEX]->tPacket, TXRX_TIMEOUT );

  return (ulRet != CIFX_NO_ERROR) ? ulRet : ptAppData->aptChannels[EIP_DEMO_CHANNEL_INDEX]->tPacket.tHeader.ulState;
}

/**************************************************************************************/
/**************************************************************************************/

/* Set an attribute value in the object directory */
static uint32_t AppEIS_SetAttributeSingle( APP_DATA_T* ptAppData,
                                           uint32_t    ulClass,
                                           uint32_t    ulInstance,
                                           uint32_t    ulAttribute,
                                           uint8_t*    pbData,
                                           uint32_t    ulSize )
{
  uint32_t                             ulRet = CIFX_NO_ERROR;
  EIP_OBJECT_PACKET_CIP_SERVICE_REQ_T* ptReq = (EIP_OBJECT_PACKET_CIP_SERVICE_REQ_T*)(&ptAppData->aptChannels[EIP_DEMO_CHANNEL_INDEX]->tPacket);

  /* prepare packet to set attribute value */
  ptReq->tData.bService    = CIP_CMD_SET_ATTR_SINGLE;
  ptReq->tData.ulClass     = ulClass;
  ptReq->tData.ulInstance  = ulInstance;
  ptReq->tData.ulAttribute = ulAttribute;
  ptReq->tData.ulMember    = 0;

  memcpy( &ptReq->tData.abData[0], pbData, ulSize );

  /* Issue CIP Service Request */
  ptReq->tHead.ulDest = HIL_PACKET_DEST_DEFAULT_CHANNEL;
  ptReq->tHead.ulCmd  = EIP_OBJECT_CIP_SERVICE_REQ;
  ptReq->tHead.ulLen  = EIP_OBJECT_CIP_SERVICE_REQ_SIZE + ulSize;

  ulRet = Pkt_SendReceivePacket( ptAppData, EIP_DEMO_CHANNEL_INDEX, &ptAppData->aptChannels[EIP_DEMO_CHANNEL_INDEX]->tPacket, TXRX_TIMEOUT );

  return (ulRet != CIFX_NO_ERROR) ? ulRet : ptAppData->aptChannels[EIP_DEMO_CHANNEL_INDEX]->tPacket.tHeader.ulState;
}

/**************************************************************************************/
/**************************************************************************************/

#ifdef HOST_APPLICATION_STORES_REMANENT_DATA

#include "Hil_ComponentID.h"
static uint32_t s_ulRemanentDataSize = 0;

/* Pass BLOB of NV-data down to the stack (as stored in an earlier STORE_REMANENT_DATA_IND) */
uint32_t AppEIS_SetRemanentData( APP_DATA_T* ptAppData )
{
  /* Set remanent data for the stack when host stores (according to taglist) */
  uint32_t                     ulRet = CIFX_NO_ERROR;
  HIL_SET_REMANENT_DATA_REQ_T* ptReq = (HIL_SET_REMANENT_DATA_REQ_T*)(&ptAppData->aptChannels[EIP_DEMO_CHANNEL_INDEX]->tPacket);

  HOSTAL_RemanentData_Load( "EisV5Remanent", sizeof(ptAppData->aptChannels[EIP_DEMO_CHANNEL_INDEX]->tPacket.abData), ptReq->tData.abData, &s_ulRemanentDataSize );

  ptReq->tHead.ulDest        = HIL_PACKET_DEST_DEFAULT_CHANNEL;
  ptReq->tHead.ulCmd         = HIL_SET_REMANENT_DATA_REQ;
  ptReq->tHead.ulLen         = sizeof(ptReq->tData.ulComponentId) + s_ulRemanentDataSize;
  ptReq->tData.ulComponentId = HIL_TASK_ID_EIP_APS;

  /* Set remanent data to the stack.
  Under the following conditions, the stack will reset the remanent data according to
  the current stack's state on the next CONFIG_DONE packet:
  1) If a packet of size HIL_SET_REMANENT_DATA_REQ_SIZE is sent (i.e. not containing any
  actual remanent data).
  2) If inconsistent remanent data is set, i.e. data of correct length but with, e.g., a
  checksum failure.
  3) If remanent data has previously been invalidated via HIL_DELETE_CONFIG_REQ.
  */
  ulRet = Pkt_SendReceivePacket( ptAppData, EIP_DEMO_CHANNEL_INDEX, &ptAppData->aptChannels[EIP_DEMO_CHANNEL_INDEX]->tPacket, TXRX_TIMEOUT );

  return (ulRet != CIFX_NO_ERROR) ? ulRet : ptAppData->aptChannels[EIP_DEMO_CHANNEL_INDEX]->tPacket.tHeader.ulState;
}

/**************************************************************************************/
/**************************************************************************************/

/* This indication will only be sent by the stack, if it is configured
 * accordingly in the taglist of the firmware.
 * Per default, the stack stores the remanent data.
 * */
void AppEIS_HandlStoreRemanentInd( APP_DATA_T* ptAppData )
{
  HIL_STORE_REMANENT_DATA_IND_T* ptInd = (HIL_STORE_REMANENT_DATA_IND_T*)(&ptAppData->aptChannels[EIP_DEMO_CHANNEL_INDEX]->tPacket);

  s_ulRemanentDataSize = ptInd->tHead.ulLen - sizeof(ptInd->tData.ulComponentId);

  HOSTAL_RemanentData_Store( "EisV5Remanent", ptInd->tData.abData, s_ulRemanentDataSize );

  ptInd->tHead.ulLen  = HIL_STORE_REMANENT_DATA_RES_SIZE;
  ptInd->tHead.ulCmd |= 0x01;

  (void)Pkt_SendPacket(ptAppData, EIP_DEMO_CHANNEL_INDEX, &ptAppData->aptChannels[EIP_DEMO_CHANNEL_INDEX]->tPacket, TX_TIMEOUT);
}
#endif /* HOST_APPLICATION_STORES_REMANENT_DATA */

/**************************************************************************************/
/**************************************************************************************/

/* Complete the configuration */
static uint32_t AppEIS_ConfigDone( APP_DATA_T* ptAppData )
{
  uint32_t ulRet = CIFX_NO_ERROR;

  ptAppData->aptChannels[EIP_DEMO_CHANNEL_INDEX]->tPacket.tHeader.ulDest = HIL_PACKET_DEST_DEFAULT_CHANNEL;
  ptAppData->aptChannels[EIP_DEMO_CHANNEL_INDEX]->tPacket.tHeader.ulCmd  = EIP_APS_CONFIG_DONE_REQ;
  ptAppData->aptChannels[EIP_DEMO_CHANNEL_INDEX]->tPacket.tHeader.ulLen  = 0;

  ulRet = Pkt_SendReceivePacket( ptAppData, EIP_DEMO_CHANNEL_INDEX, &ptAppData->aptChannels[EIP_DEMO_CHANNEL_INDEX]->tPacket, TXRX_TIMEOUT );

  return (ulRet != CIFX_NO_ERROR) ? ulRet : ptAppData->aptChannels[EIP_DEMO_CHANNEL_INDEX]->tPacket.tHeader.ulState;
}

/**************************************************************************************/
/**************************************************************************************/

/* A channel init, in the Extended Configuration Case, has the meaning of shutdown/restart stack
   and allow for subsequent configuration from a defined state.
*/
uint32_t AppEIS_ChannelInit(APP_DATA_T* ptAppData)
{
  App_SysPkt_AssembleChannelInitReq( &ptAppData->aptChannels[EIP_DEMO_CHANNEL_INDEX]->tPacket );
  return Pkt_SendReceivePacket( ptAppData, EIP_DEMO_CHANNEL_INDEX, &ptAppData->aptChannels[EIP_DEMO_CHANNEL_INDEX]->tPacket, TXRX_TIMEOUT);
}

/**************************************************************************************/
/**************************************************************************************/

/* A delete config will delete the stored configuration, if applicable (only for simpel config),
   and will also delete the remanent data.
*/
uint32_t AppEIS_DeleteConfig(APP_DATA_T* ptAppData)
{
  ptAppData->aptChannels[EIP_DEMO_CHANNEL_INDEX]->tPacket.tHeader.ulDest = HIL_PACKET_DEST_DEFAULT_CHANNEL;
  ptAppData->aptChannels[EIP_DEMO_CHANNEL_INDEX]->tPacket.tHeader.ulCmd  = HIL_DELETE_CONFIG_REQ;
  ptAppData->aptChannels[EIP_DEMO_CHANNEL_INDEX]->tPacket.tHeader.ulLen  = 0;
  return Pkt_SendReceivePacket( ptAppData, EIP_DEMO_CHANNEL_INDEX, &ptAppData->aptChannels[EIP_DEMO_CHANNEL_INDEX]->tPacket, TXRX_TIMEOUT);
}

/**************************************************************************************/
/**************************************************************************************/

/* Download the configuration */
uint32_t AppEIS_ConfigureStack( APP_DATA_T* ptAppData )
{
  uint32_t ulRet = CIFX_NO_ERROR;
  uint32_t ulIdx;

  /* bring everything to a defined (rebooted) state */
  ulRet = AppEIS_ChannelInit(ptAppData);
  if (ulRet != 0)
    return ulRet;

  /* register all assemblies */
  for (ulIdx = 0; ulIdx < s_ulCipAssemblyDirEntries; ulIdx++)
  {
    ulRet = AppEIS_RegisterAssembly( ptAppData,
                                  s_ptCipAssemblyDir[ulIdx].ulInstance,
                                  s_ptCipAssemblyDir[ulIdx].ulSize,
                                  s_ptCipAssemblyDir[ulIdx].ulOffset,
                                  s_ptCipAssemblyDir[ulIdx].ulFlags);
    if( ulRet != 0 )
      return ulRet;
  }

  /* create all connections */
  for (ulIdx = 0; ulIdx < s_ulCipSupConnectionDirEntries; ulIdx++)
  {
    ulRet = AppEIS_CreateConnection( ptAppData,
                                  s_ptCipSuppConnectionDir[ulIdx].ulConsumerCP,
                                  s_ptCipSuppConnectionDir[ulIdx].ulProducerCP,
                                  s_ptCipSuppConnectionDir[ulIdx].ulConfigCP,
                                  s_ptCipSuppConnectionDir[ulIdx].bConnType);
    if( ulRet != 0 )
      return ulRet;
  }

  /* register all objects */
  for (ulIdx = 0; ulIdx < s_ulCipRegisterObjDirEntries; ulIdx++)
  {
    ulRet = AppEIS_RegisterObject( ptAppData,
                                s_ptCipRegisterObjDir[ulIdx].ulClass,
                                s_ptCipRegisterObjDir[ulIdx].ulOptionFlags);
    if( ulRet != 0 )
      return ulRet;
  }

  /* download the object dictionary defaults */
  for (ulIdx = 0; ulIdx < s_ulCipObjDirEntries; ulIdx++)
  {
    ulRet = AppEIS_SetAttributeSingle( ptAppData,
                              s_ptCipObjDirConfigValues[ulIdx].ulClass,
                              s_ptCipObjDirConfigValues[ulIdx].ulInstance,
                              s_ptCipObjDirConfigValues[ulIdx].ulAttribute,
                              s_ptCipObjDirConfigValues[ulIdx].pbData,
                              s_ptCipObjDirConfigValues[ulIdx].ulSize);
    if( ulRet != 0 )
      return ulRet;
  }

  ulRet = AppEIS_ConfigDone( ptAppData );  /* complete the configuration */
  if( ulRet != 0 )
    return ulRet;

  App_SysPkt_AssembleStartStopCommReq( &ptAppData->aptChannels[EIP_DEMO_CHANNEL_INDEX]->tPacket, true );
  ulRet = Pkt_SendReceivePacket( ptAppData, EIP_DEMO_CHANNEL_INDEX, &ptAppData->aptChannels[EIP_DEMO_CHANNEL_INDEX]->tPacket, TXRX_TIMEOUT );
  if( ulRet != CIFX_NO_ERROR )
    return ulRet;

  return ulRet;
}

/**************************************************************************************/
/**************************************************************************************/

static void AppEIS_HandleResetInd( APP_DATA_T* ptAppData )
{
  uint32_t                        ulRet = CIFX_NO_ERROR;
  EIP_OBJECT_PACKET_RESET_IND_T*  ptInd = (EIP_OBJECT_PACKET_RESET_IND_T*) (&ptAppData->aptChannels[EIP_DEMO_CHANNEL_INDEX]->tPacket);

  /* keep a copy of the indicationon on the stack,
  so we can reuse the packet in the meantime */
  EIP_OBJECT_PACKET_RESET_IND_T tResetInd = *ptInd;

  PRINTF("Reset Type %i Indication" NEWLINE, (int)ptInd->tData.ulResetTyp);

  if( ptInd->tData.ulResetTyp == 1 )
  {
    /* Type 1 reset -> We are supposed to restore the factory defaults in NV storage, so
       we send a DELETE_CONFIG_REQ to delete the remanent data (i.e. render it invalid)
       The stack will then reset the remanent data from the configuration we download
       subsequently as soon as we send the CONFIG_DONE packet.
    */
    ulRet = AppEIS_DeleteConfig(ptAppData);
    if (ulRet != CIFX_NO_ERROR)
    {
      PRINTF("HIL_CONFIG_DELETE_REQ failed with error code 0x%08x" NEWLINE, (unsigned int)ulRet);
    }
  }

  /* Respond to the reset indication (this will trigger the reply on the network) */
  ptInd = (EIP_OBJECT_PACKET_RESET_IND_T*)&ptAppData->aptChannels[EIP_DEMO_CHANNEL_INDEX]->tPacket;
  *ptInd = tResetInd;
  ptInd->tHead.ulLen = EIP_OBJECT_RESET_RES_SIZE;
  ptInd->tHead.ulSta = SUCCESS_HIL_OK;
  ptInd->tHead.ulCmd |= 0x01; /* Make it a response */

  /* Send reset response */
  ulRet = Pkt_SendPacket( ptAppData, EIP_DEMO_CHANNEL_INDEX, (CIFX_PACKET *) ptInd, TX_TIMEOUT);

  if( ulRet != CIFX_NO_ERROR )
  {
    PRINTF("Sending EIP_OBJECT_RESET_RES failed with error code 0x%08x" NEWLINE, (unsigned int)ulRet);
  }

  /* Finally, send a channel init request to perform the actual reset. */
  AppEIS_ChannelInit(ptAppData);

  /* Reconfigure the stack */
  AppEIS_ConfigureStack( ptAppData );
}

/**************************************************************************************/
/**************************************************************************************/

static void AppEIS_HandleConnectionInd(APP_DATA_T* ptAppData)
{
  EIP_OBJECT_PACKET_CONNECTION_IND_T* ptInd = (EIP_OBJECT_PACKET_CONNECTION_IND_T*) (&ptAppData->aptChannels[EIP_DEMO_CHANNEL_INDEX]->tPacket);

  PRINTF("Connection changed: Type %d, State %d" NEWLINE, (int)ptInd->tData.bConnType, (int)ptInd->tData.ulConnectionState );

  PRINTF("Config path: class %d, instance %d, connpoint %d, attrib %d, member %d" NEWLINE,
    (int)ptInd->tData.tConfigPath.ulClass,
    (int)ptInd->tData.tConfigPath.ulInstance,
    (int)ptInd->tData.tConfigPath.ulConnPoint,
    (int)ptInd->tData.tConfigPath.ulAttribute,
    (int)ptInd->tData.tConfigPath.ulMember);

  PRINTF("Consumption path: class %d, instance %d, connpoint %d, attrib %d, member %d" NEWLINE,
    (int)ptInd->tData.tConsumptionPath.ulClass,
    (int)ptInd->tData.tConsumptionPath.ulInstance,
    (int)ptInd->tData.tConsumptionPath.ulConnPoint,
    (int)ptInd->tData.tConsumptionPath.ulAttribute,
    (int)ptInd->tData.tConsumptionPath.ulMember);

  PRINTF("Production path: class %d, instance %d, connpoint %d, attrib %d, member %d" NEWLINE,
    (int)ptInd->tData.tProductionPath.ulClass,
    (int)ptInd->tData.tProductionPath.ulInstance,
    (int)ptInd->tData.tProductionPath.ulConnPoint,
    (int)ptInd->tData.tProductionPath.ulAttribute,
    (int)ptInd->tData.tProductionPath.ulMember);

  ptInd->tHead.ulLen  = 0;
  ptInd->tHead.ulSta  = SUCCESS_HIL_OK;
  ptInd->tHead.ulCmd |= 0x01; /* Make it a response */

  (void)Pkt_SendPacket( ptAppData, EIP_DEMO_CHANNEL_INDEX, (CIFX_PACKET *) ptInd, TX_TIMEOUT );
}

/**************************************************************************************/
/**************************************************************************************/

static void AppEIS_HandleObjChangeInd( APP_DATA_T* ptAppData )
{
  EIP_OBJECT_PACKET_CIP_OBJECT_CHANGE_IND_T* ptInd = (EIP_OBJECT_PACKET_CIP_OBJECT_CHANGE_IND_T*) (&ptAppData->aptChannels[EIP_DEMO_CHANNEL_INDEX]->tPacket);

  PRINTF( "Object Changed: Class %x, Instance %d, Attribute %d, Member %d  " NEWLINE,
          (int) ptInd->tData.ulClass,
          (int) ptInd->tData.ulInstance,
          (int) ptInd->tData.ulAttribute,
          (int) ptInd->tData.ulMember);

  /* Send object change response packet. */
  ptInd->tHead.ulSta  = SUCCESS_HIL_OK;
  ptInd->tHead.ulCmd |= 0x01; /* Make it a response */

  (void)Pkt_SendPacket( ptAppData, EIP_DEMO_CHANNEL_INDEX, (CIFX_PACKET *) ptInd, TX_TIMEOUT );
}

/**************************************************************************************/
/**************************************************************************************/

static void AppEIS_HandleCl3Ind( APP_DATA_T* ptAppData )
{
  EIP_OBJECT_PACKET_CL3_SERVICE_IND_T *ptInd = (EIP_OBJECT_PACKET_CL3_SERVICE_IND_T*) (&ptAppData->aptChannels[EIP_DEMO_CHANNEL_INDEX]->tPacket);
  EIP_OBJECT_PACKET_CL3_SERVICE_RES_T *ptRes = (EIP_OBJECT_PACKET_CL3_SERVICE_RES_T*) (&ptAppData->aptChannels[EIP_DEMO_CHANNEL_INDEX]->tPacket);

  PRINTF("Explicit Message Received" NEWLINE);
  PRINTF("Service %x, Class %x, Instance %d, Attribute %d, Member %d" NEWLINE, (unsigned int)ptInd->tData.bService,
                                                                               (unsigned int)ptInd->tData.ulObject,
                                                                               (unsigned int)ptInd->tData.ulInstance,
                                                                               (unsigned int)ptInd->tData.ulAttribute,
                                                                               (unsigned int)ptInd->tData.ulMember );

  uint32_t ulGRC = CIP_GSR_BAD_CLASS_INSTANCE;
  uint32_t ulERC = 0;

  switch( ptInd->tData.ulObject )
  {
    case CIP_AS_CLASS_NUMBER:
    {
      switch( ptInd->tData.bService )
      {
        case CIP_CMD_SET_ATTR_SINGLE:
        {
          if( ptInd->tData.ulInstance == EIP_DEMO_CONFIGURATION_ASSEMBLY_INSTANCE )
          {
            if( ptInd->tData.ulAttribute == CIP_AS_ATTR_3_DATA )
            {
              uint32_t ulDataLength = ptInd->tHead.ulLen - EIP_OBJECT_CL3_SERVICE_IND_SIZE;

              if( 0 == ulDataLength )
              {
                /* Config size is 0. Accept it and use default configuration or the configuration that has been set before. */
                ulGRC = CIP_GSR_SUCCESS;
                ulERC = 0;
              }
              else if( ulDataLength > EIP_DEMO_CONFIGURATION_ASSEMBLY_INSTANCE_SIZE )
              {
                ulGRC = CIP_GSR_CONFIG_TOO_BIG;
              }
              else if( ulDataLength < EIP_DEMO_CONFIGURATION_ASSEMBLY_INSTANCE_SIZE )
              {
                ulGRC = CIP_GSR_CONFIG_TOO_SMALL;
              }
              else
              {
                bool fConfigOk = true;

                /* Check content of configuration data here:
                 *
                 * ... */

                if( true == fConfigOk )
                {
                  /* Store new configuration data */
                  memcpy( &s_abConfigAssemblyData, &ptInd->tData.abData[0], sizeof(s_abConfigAssemblyData) );

                  ulGRC = CIP_GSR_SUCCESS;
                  ulERC = 0;
                }
                else
                {
                  ulGRC = CIP_GSR_BAD_ATTR_DATA;
                  ulERC = 2; /* Byte index of invalid data within configuration. Value 2 is just an example, put in the correct value. */
                }
              }
            }
          }
          break;
        }

        default:
        {

          ulGRC = CIP_GSR_BAD_SERVICE;
          ulERC = 0;
          break;
        }
      }
    }
    break;
    default:
    {
      break;
    }
  }

  ptRes->tData.ulGRC  = ulGRC;
  ptRes->tData.ulERC  = ulERC;
  ptRes->tHead.ulLen  = EIP_OBJECT_CL3_SERVICE_RES_SIZE;
  ptRes->tHead.ulCmd |= 0x01; /* Make it a response */

  if( ptRes->tData.ulGRC != CIP_GSR_SUCCESS )
    ptRes->tHead.ulSta = ERR_HIL_FAIL;
  else
    ptRes->tHead.ulSta = SUCCESS_HIL_OK;

  (void)Pkt_SendPacket(ptAppData, EIP_DEMO_CHANNEL_INDEX, (CIFX_PACKET *) ptInd, TX_TIMEOUT);
}

/**************************************************************************************/
/**************************************************************************************/

uint32_t AppEIS_SetMacAddress( APP_DATA_T* ptAppData )
{
  uint32_t                   ulRet = CIFX_NO_ERROR;
  HIL_DDP_SERVICE_SET_REQ_T* ptReq = (HIL_DDP_SERVICE_SET_REQ_T*)&ptAppData->aptChannels[EIP_DEMO_CHANNEL_INDEX]->tPacket;

  uint8_t abMyComMacAddresses[8][6] =
  {
    { 0xa, 0xb, 0xc, 0xd, 0xe, 0x0 },   /* This is the first chassis MAC address which is used by EtherNet/IP */
    { 0xa, 0xb, 0xc, 0xd, 0xe, 0x1 },   /* Port 0 MAC Address used for LLDP                                   */
    { 0xa, 0xb, 0xc, 0xd, 0xe, 0x2 },   /* Port 1 MAC Address used for LLDP                                   */
    { 0xa, 0xb, 0xc, 0xd, 0xe, 0x3 },   /* This is the second chassis MAC (RawEthernet/NDIS)                  */
    { 0xa, 0xb, 0xc, 0xd, 0xe, 0x4 },
    { 0xa, 0xb, 0xc, 0xd, 0xe, 0x5 },
    { 0xa, 0xb, 0xc, 0xd, 0xe, 0x6 },
    { 0xa, 0xb, 0xc, 0xd, 0xe, 0x7 },
  };

  ptReq->tHead.ulDest = HIL_PACKET_DEST_DEFAULT_CHANNEL;
  ptReq->tHead.ulCmd  = HIL_DDP_SERVICE_SET_REQ;
  ptReq->tHead.ulLen  = sizeof(ptReq->tData.ulDataType) + sizeof(abMyComMacAddresses);

  /* Set MAC adrress for the protocol stack (override pre-defined MAC address from FDL) */
  ptReq->tData.ulDataType = HIL_DDP_SERVICE_DATATYPE_MAC_ADDRESSES_COM;

  memcpy(ptReq->tData.uDataType.atMacAddress, abMyComMacAddresses, sizeof(abMyComMacAddresses));

  ulRet = Pkt_SendReceivePacket( ptAppData, EIP_DEMO_CHANNEL_INDEX, &ptAppData->aptChannels[EIP_DEMO_CHANNEL_INDEX]->tPacket, TXRX_TIMEOUT );
  return (ulRet != CIFX_NO_ERROR) ? ulRet : ptAppData->aptChannels[EIP_DEMO_CHANNEL_INDEX]->tPacket.tHeader.ulState;
}

/**************************************************************************************/
/**************************************************************************************/

uint32_t AppEIS_SetSerialNumber( APP_DATA_T* ptAppData )
{
  uint32_t                   ulRet          = CIFX_NO_ERROR;
  HIL_DDP_SERVICE_SET_REQ_T* ptReq          = (HIL_DDP_SERVICE_SET_REQ_T*)&ptAppData->aptChannels[EIP_DEMO_CHANNEL_INDEX]->tPacket;
  char*                      szSerialNumber = "76543";

  ptReq->tHead.ulDest     = HIL_PACKET_DEST_DEFAULT_CHANNEL;
  ptReq->tHead.ulCmd      = HIL_DDP_SERVICE_SET_REQ;
  ptReq->tHead.ulLen      = sizeof(ptReq->tData.ulDataType) + strlen(szSerialNumber) + 1;
  ptReq->tData.ulDataType = HIL_DDP_SERVICE_DATATYPE_OEM_SERIALNUMBER;

  memcpy(ptReq->tData.uDataType.szString, szSerialNumber, strlen(szSerialNumber) + 1);

  ulRet = Pkt_SendReceivePacket( ptAppData, EIP_DEMO_CHANNEL_INDEX, &ptAppData->aptChannels[EIP_DEMO_CHANNEL_INDEX]->tPacket, TXRX_TIMEOUT );

  /* Check status of cifX function and confirmation packet */
  ulRet = (ulRet != CIFX_NO_ERROR) ? ulRet : ptAppData->aptChannels[EIP_DEMO_CHANNEL_INDEX]->tPacket.tHeader.ulState;
  if( CIFX_NO_ERROR != ulRet )
  {
    return ulRet;
  }

  /* also render the OEM serial number "valid" in the corresponding bitfield */
  ptReq->tHead.ulDest            = HIL_PACKET_DEST_DEFAULT_CHANNEL;
  ptReq->tHead.ulCmd             = HIL_DDP_SERVICE_SET_REQ;
  ptReq->tHead.ulLen             = sizeof(ptReq->tData.ulDataType) + sizeof(ptReq->tData.uDataType.ulValue);
  ptReq->tData.ulDataType        = HIL_DDP_SERVICE_DATATYPE_OEM_OPTIONS;
  ptReq->tData.uDataType.ulValue = 0x1;    /* set bit zero to set OEM serial number valid */

  ulRet = Pkt_SendReceivePacket( ptAppData, EIP_DEMO_CHANNEL_INDEX, &ptAppData->aptChannels[EIP_DEMO_CHANNEL_INDEX]->tPacket, TXRX_TIMEOUT );

  /* Check status of cifX function and confirmation packet */
  return (ulRet != CIFX_NO_ERROR) ? ulRet : ptAppData->aptChannels[EIP_DEMO_CHANNEL_INDEX]->tPacket.tHeader.ulState;
}

/**************************************************************************************/
/**************************************************************************************/

uint32_t AppEIS_ActivateDdp( APP_DATA_T* ptAppData )
{
  uint32_t                   ulRet = CIFX_NO_ERROR;
  HIL_DDP_SERVICE_SET_REQ_T* ptReq = (HIL_DDP_SERVICE_SET_REQ_T*)&ptAppData->aptChannels[EIP_DEMO_CHANNEL_INDEX]->tPacket;

  /* required when inital DPP state is passive: Set DDP active now */
  ptReq->tHead.ulDest            = HIL_PACKET_DEST_DEFAULT_CHANNEL;
  ptReq->tHead.ulCmd             = HIL_DDP_SERVICE_SET_REQ;
  ptReq->tHead.ulLen             = sizeof(ptReq->tData.ulDataType) + sizeof(ptReq->tData.uDataType.ulValue);
  ptReq->tData.ulDataType        = HIL_DDP_SERVICE_DATATYPE_STATE;
  ptReq->tData.uDataType.ulValue = HIL_DDP_SERVICE_STATE_ACTIVE;

  ulRet = Pkt_SendReceivePacket( ptAppData, EIP_DEMO_CHANNEL_INDEX, &ptAppData->aptChannels[EIP_DEMO_CHANNEL_INDEX]->tPacket, TXRX_TIMEOUT );

  /* Check status of cifX function and confirmation packet */
  return (ulRet != CIFX_NO_ERROR) ? ulRet : ptAppData->aptChannels[EIP_DEMO_CHANNEL_INDEX]->tPacket.tHeader.ulState;
}

/**************************************************************************************/
/**************************************************************************************/

bool AppEIS_PacketHandler( CIFX_PACKET* ptPacket,
                           void*        pvUserData )
{
  bool        fPacketCouldBeHandled = false;
  APP_DATA_T* ptAppData             = (APP_DATA_T*) pvUserData;

  if( ptPacket != &ptAppData->aptChannels[EIP_DEMO_CHANNEL_INDEX]->tPacket )
  {
    PRINTF("Unexpected packet resource received!!!" NEWLINE);
    return false;
  }

  switch( ptPacket->tHeader.ulCmd )
  {
    case EIP_OBJECT_CONNECTION_IND:
      AppEIS_HandleConnectionInd( ptAppData );
      fPacketCouldBeHandled = true;
      break;

    case EIP_OBJECT_RESET_IND:
      AppEIS_HandleResetInd(ptAppData);
      fPacketCouldBeHandled = true;
      break;

    case EIP_OBJECT_CL3_SERVICE_IND:
      AppEIS_HandleCl3Ind( ptAppData );
      fPacketCouldBeHandled = true;
      break;

    case EIP_OBJECT_CIP_OBJECT_CHANGE_IND:
      AppEIS_HandleObjChangeInd( ptAppData );
      fPacketCouldBeHandled = true;
      break;

    case HIL_LINK_STATUS_CHANGE_IND:
      (void)App_SysPkt_HandleLinkStatusChangeInd( ptAppData, EIP_DEMO_CHANNEL_INDEX, &ptAppData->aptChannels[EIP_DEMO_CHANNEL_INDEX]->tPacket );
      fPacketCouldBeHandled = true;
      break;

#ifdef HOST_APPLICATION_STORES_REMANENT_DATA
    case HIL_STORE_REMANENT_DATA_IND:
      AppEIS_HandlStoreRemanentInd( ptAppData );
      fPacketCouldBeHandled = true;
      break;
#endif

    default:
      if( (ptAppData->aptChannels[EIP_DEMO_CHANNEL_INDEX]->tPacket.tHeader.ulCmd & 0x1) == 0 ) /* received an indication*/
      {
        PRINTF("Warning: Disregarded indication packet received!" NEWLINE);
        ptAppData->aptChannels[EIP_DEMO_CHANNEL_INDEX]->tPacket.tHeader.ulLen    = 0;
        ptAppData->aptChannels[EIP_DEMO_CHANNEL_INDEX]->tPacket.tHeader.ulState  = ERR_HIL_UNKNOWN_COMMAND;
        ptAppData->aptChannels[EIP_DEMO_CHANNEL_INDEX]->tPacket.tHeader.ulCmd   |= 0x01; /* Make it a response */

        (void)Pkt_SendPacket( ptAppData, EIP_DEMO_CHANNEL_INDEX, &ptAppData->aptChannels[EIP_DEMO_CHANNEL_INDEX]->tPacket, TX_TIMEOUT );
        fPacketCouldBeHandled  = true;
      }
      else
      { /* received a confirmation */
        PRINTF("Warning: Disregarded confirmation packet received!" NEWLINE);
      }
      break;
  }

  return fPacketCouldBeHandled;
}

/**************************************************************************************/
/**************************************************************************************/
