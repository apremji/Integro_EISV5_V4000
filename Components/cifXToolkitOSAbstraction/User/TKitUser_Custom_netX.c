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

***************************************************************************************

  $Id: TKitUser_Custom_netX.c 2303 2016-11-23 11:36:45Z Jin $:

  Description:
    USER implemented functions called by the cifX Toolkit.

  Changes:
    Date        Description
    -----------------------------------------------------------------------------------
    2006-08-07  initial version

**************************************************************************************/

/*****************************************************************************/
/*! \file TKitUser.c                                                         *
*    USER implemented functions called by the cifX Toolkit                   */
/*****************************************************************************/

#include "cifXErrors.h"
#include "cifXToolkit.h"
/* Standard includes */
#include <stdio.h>
#include <stdarg.h>

/*****************************************************************************/
/*!  \addtogroup CIFX_TK_USER User specific implementation
*    \{                                                                      */
/*****************************************************************************/

/*****************************************************************************/
/*! Returns the number of firmware files associated with the card/channel,
*   passed as argument.
*   \param ptDevInfo DeviceInfo including channel number, for which the
*                    firmware file count should be read
*   \return number for firmware files, to download. The returned value will
*           be used as maximum value for ulIdx in calls to
*           USER_GetFirmwareFile                                             */
/*****************************************************************************/
uint32_t USER_GetFirmwareFileCount(PCIFX_DEVICE_INFORMATION ptDevInfo)
{
  return 0;
}

/*****************************************************************************/
/*! Returns firmware file information for the given device/channel and Idx
*   passed as argument.
*   \param ptDevInfo  DeviceInfo including channel number, for which the
*                     firmware file should be delivered
*   \param ulIdx      Index of the returned file
*                     (0..USER_GetFirmwareFileCount() - 1)
*   \param ptFileInfo Short and full file name of the firmware. Used in
*                     calls to OS_OpenFile()
*   \return !=0 on success                                                   */
/*****************************************************************************/

int USER_GetFirmwareFile(PCIFX_DEVICE_INFORMATION ptDevInfo, uint32_t ulIdx, PCIFX_FILE_INFORMATION ptFileInfo)
{
  return 0;
}

/*****************************************************************************/
/*! Returns the number of configuration files associated with the card/
*   channel, passed as argument.
*   \param ptDevInfo DeviceInfo including channel number, for which the
*                    configuration file count should be read
*   \return number for confgiuration files, to download. The returned value
*           will be used as maximum value for ulIdx in calls to
*           USER_GetConfgirationFile                                         */
/*****************************************************************************/
uint32_t USER_GetConfigurationFileCount(PCIFX_DEVICE_INFORMATION ptDevInfo)
{
  return 0;
}

/*****************************************************************************/
/*! Returns configuration file information for the given device/channel and
*   Idx passed as argument.
*   \param ptDevInfo  DeviceInfo including channel number, for which the
*                     configuration file should be delivered
*   \param ulIdx      Index of the returned file
*                     (0..USER_GetConfigurationFileCount() - 1)
*   \param ptFileInfo Short and full file name of the configuration. Used in
*                     calls to OS_OpenFile()
*   \return !=0 on success                                                   */
/*****************************************************************************/
int USER_GetConfigurationFile(PCIFX_DEVICE_INFORMATION ptDevInfo, uint32_t ulIdx, PCIFX_FILE_INFORMATION ptFileInfo)
{
  return 0;
}

/*****************************************************************************/
/*! Returns OS file information for the given device/channel and Idx
*   passed as argument. This is needed for module downloading
*   \param ptDevInfo  DeviceInfo for which the
*                     firmware file should be delivered
*   \param ptFileInfo Short and full file name of the firmware. Used in
*                     calls to OS_OpenFile()
*   \return != 0 on success                                                   */
/*****************************************************************************/
int USER_GetOSFile(PCIFX_DEVICE_INFORMATION ptDevInfo, PCIFX_FILE_INFORMATION ptFileInfo)
{
  return 0;
}

/*****************************************************************************/
/*! Retrieve the full file name of the cifX Romloader binary image
*   \param ptDevInstance  Pointer to the device instance
*   \param ptFileInfo Short and full file name of the bootloader. Used in
*                     calls to OS_OpenFile()
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
void USER_GetBootloaderFile(PDEVICEINSTANCE ptDevInstance, PCIFX_FILE_INFORMATION ptFileInfo)
{
  return;
}

/*****************************************************************************/
/*! Retrieve the alias name of a cifX Board depending on the Device and
*   Serialnumber passed during this call
*   \param ptDevInfo Device- and Serial number of the card
*   \param ulMaxLen  Maximum length of alias
*   \param szAlias   Buffer to copy alias to. A string of length 0 means
*                    no alias                                                */
/*****************************************************************************/
void USER_GetAliasName(PCIFX_DEVICE_INFORMATION ptDevInfo, uint32_t ulMaxLen, char* szAlias)
{
  return;
}

/*****************************************************************************/
/*! Read the warmstart data from a given warmstart file
*   \param ptDevInfo Device- and Serial number of the card
*   \param ptPacket  Buffer for the warmstart packet                         */
/*****************************************************************************/
int USER_GetWarmstartParameters(PCIFX_DEVICE_INFORMATION ptDevInfo, CIFX_PACKET* ptPacket)
{
  return 0;
}

/*****************************************************************************/
/*! User trace function
*   right while cifXTKitAddDevice is being processed
*   \param ptDevInstance  Device instance
*   \param ulTraceLevel   Trace level
*   \param szFormat       Format string                                      */
/*****************************************************************************/
void USER_Trace(PDEVICEINSTANCE ptDevInstance, uint32_t ulTraceLevel, const char* szFormat, ...)
{
  va_list vaList;

  if(g_ulTraceLevel & ulTraceLevel)
  {
    va_start(vaList, szFormat);
    vprintf(szFormat, vaList);
    va_end(vaList);
    printf("\r\n");
  }

  UNREFERENCED_PARAMETER(ptDevInstance);
}


/*****************************************************************************/
/*! Check if interrupts should be enabled for this device
*   \param ptDevInfo  Device Infotmation
*   \return !=0 to enable interrupts                                         */
/*****************************************************************************/
int USER_GetInterruptEnable(PCIFX_DEVICE_INFORMATION ptDevInfo)
{
  return 0;
}

/*****************************************************************************/
/*! \}                                                                       */
/*****************************************************************************/
