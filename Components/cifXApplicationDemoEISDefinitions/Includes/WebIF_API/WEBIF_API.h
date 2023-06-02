/************************************************************************************
copyright (c) hilscher gesellschaft fuer systemautomation mbh. all rights reserved.
***************************************************************************************
$id: $:

description:
 Web packet interface definition header file
**************************************************************************************/

#ifndef WEBIF_API_H_
#define WEBIF_API_H_

#include <stdint.h>
#include <Hil_CommandRange.h>
#include <Hil_Packet.h>
#include "SOCK_API.h"

/*
 * The following values are defined according the internal implementations:
 * - WEBIF_PCK_CONTENT_SIZE is maximally set to the sum of TX Buffer sizes (defined TcpBuf_MemPool.h).
 * - WEBIF_FIELD_MAX_SIZE is dependent of string storage implemented in Http.c and Utils_HashMap.c.
 * These values can be bigger, but there is no concret benefit.
 */

/*! Maximal response or request content size */
#define WEBIF_PCK_CONTENT_SIZE 1024

/*! Maximal field name and content size */
#define WEBIF_FIELD_MAX_SIZE 128

/*! \defgroup WEBIF_Cmds WebIf Commands
*  \{ */

/*! Indication which indicate the HTTP request reception */
#define WEBIF_HANDLE_HTTP_REQUEST_IND 0x0000AF00
/*! Response command of the WEBIF_HANDLE_HTTP_REQUEST_IND */
#define WEBIF_HANDLE_HTTP_REQUEST_RSP 0x0000AF01

/*! Indication which indicate the HTTP request body reception */
#define WEBIF_HANDLE_HTTP_REQUEST_CONTENT_IND 0x0000AF02
/*! Response command of WEBIF_HANDLE_HTTP_REQUEST_CONTENT_IND */
#define WEBIF_HANDLE_HTTP_REQUEST_CONTENT_RSP 0x0000AF03

/*! Indication which indicate the end of HTTP request reception */
#define WEBIF_FINISH_HANDLING_HTTP_REQUEST_IND 0x0000AF04
/*! Response command of WEBIF_FINISH_HANDLING_HTTP_REQUEST_IND */
#define WEBIF_FINISH_HANDLING_HTTP_REQUEST_RSP 0x0000AF05

/*! Request to get a field in the HTTP request */
#define WEBIF_GET_HTTP_REQUEST_FIELD_REQ 0x0000AF06
/*! Confirmation to WEBIF_GET_HTTP_REQUEST_FIELD_REQ */
#define WEBIF_GET_HTTP_REQUEST_FIELD_CNF 0x0000AF07

/*! Request to begin the HTTP response transmission */
#define WEBIF_GENERATE_HTTP_RESPONSE_REQ 0x0000AF08
/*! Confirmation to WEBIF_GENERATE_HTTP_RESPONSE_REQ */
#define WEBIF_GENERATE_HTTP_RESPONSE_CNF 0x0000AF09

/*! Request to set a field in HTTP response */
#define WEBIF_GENERATE_HTTP_RESPONSE_FIELD_REQ 0x0000AF0A
/*! Confirmation to WEBIF_GENERATE_HTTP_RESPONSE_FIELD_REQ */
#define WEBIF_GENERATE_HTTP_RESPONSE_FIELD_CNF 0x0000AF0B

/*! Request to transmit body in the HTTP response */
#define WEBIF_GENERATE_HTTP_RESPONSE_CONTENT_REQ 0x0000AF0C
/*! Confirmation to WEBIF_GENERATE_HTTP_RESPONSE_CONTENT_REQ */
#define WEBIF_GENERATE_HTTP_RESPONSE_CONTENT_CNF 0x0000AF0D

/*! Request to finalize the transmission of HTTP response body */
#define WEBIF_FINISH_GENERATION_HTTP_RESPONSE_REQ 0x0000AF0E
/*! Confirmation to WEBIF_FINISH_GENERATION_HTTP_RESPONSE_REQ */
#define WEBIF_FINISH_GENERATION_HTTP_RESPONSE_CNF 0x0000AF0F

/*! Request to be registered */
#define WEBIF_ENABLE_REQUEST_HANDLING_REQ 0x0000AF10
/*! Confirmation to WEBIF_ENABLE_REQ */
#define WEBIF_ENABLE_REQUEST_HANDLING_CNF 0x0000AF11
/*! Retrocompatibility definitions */
#define WEBIF_REGISTER_REQ WEBIF_ENABLE_REQUEST_HANDLING_REQ
#define WEBIF_REGISTER_CNF WEBIF_ENABLE_REQUEST_HANDLING_CNF

/*! Request to be registered */
#define WEBIF_DISABLE_REQUEST_HANDLING_REQ 0x0000AF20
/*! Confirmation to WEBIF_DISABLE_REQ */
#define WEBIF_DISABLE_REQUEST_HANDLING_CNF 0x0000AF21
/*! Retrocompatibility definitions */
#define WEBIF_UNREGISTER_REQ WEBIF_DISABLE_REQUEST_HANDLING_REQ
#define WEBIF_UNREGISTER_CNF WEBIF_DISABLE_REQUEST_HANDLING_CNF

/*! WebServer Start Request*/
#define WEBIF_START_REQ                             0x0000AF22
#define WEBIF_START_CNF                             0x0000AF23


/*!  \} */

/*! \defgroup WEBIF_Data WebIf Data Packets
*  \{ */

#define WEBIF_HTTP_METHOD_GET 0x00000001
#define WEBIF_HTTP_METHOD_HEAD 0x00000002
#define WEBIF_HTTP_METHOD_POST 0x00000003
#define WEBIF_HTTP_METHOD_PUT 0x00000004
#define WEBIF_HTTP_METHOD_DELETE 0x00000005
#define WEBIF_HTTP_METHOD_TRACE 0x00000006
#define WEBIF_HTTP_METHOD_CONNECT 0x00000007

/*! Special value to mark the chunked transfer */
#define WEBIF_CONTENT_LENGTH_CHUNK 0xFFFFFFFF
/*! Maximum value of the request body (content */
#define WEBIF_CONTENT_LENGTH_MAX (WEBIF_CONTENT_LENGTH_CHUNK - 1)

/*! Data about an eventual authenticated client */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST
{
#define WEBIF_AUTH_STATUS_AUTHENTICATED 0
#define WEBIF_AUTH_STATUS_NOT_AUTHENTICATED 2
  uint8_t uiStatus;
  #define WEBIF_AUTH_USERNAME_SIZE 32
  uint8_t szUserName[WEBIF_AUTH_USERNAME_SIZE];
} WEBIF_AUTH_T;

/*! Data of WEBIF_HANDLE_HTTP_REQUEST_IND packet */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST
{
  /*! HTTP method */
  uint32_t ulHttpMethod; /*! Request method */
  uint32_t ulContentLength; /*! Size of the request body */
  char aPathName[WEBIF_FIELD_MAX_SIZE]; /*! Request pathname */
  SOCK_ADDR_IP_T tClientAddr; /*! Client Address (V2 only), all fields are equal to zero if the remote address is unknown. */
  WEBIF_AUTH_T tAuth; /*! V2 Only: Information about the authentication */
} WEBIF_HANDLE_HTTP_REQUEST_IND_DATA_T;
#define WEBIF_GENERATE_HTTP_REQUEST_IND_DATA_SIZE_V1 ((sizeof(uint32_t) * 2) + (sizeof(char) * WEBIF_FIELD_MAX_SIZE))
#define WEBIF_GENERATE_HTTP_REQUEST_IND_DATA_SIZE_V2 (sizeof(WEBIF_HANDLE_HTTP_REQUEST_IND_DATA_T))

/*! Data of WEBIF_HANDLE_HTTP_REQUEST_CONTENT_IND packet */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST
{
  uint32_t ulOffset; /*! Offset of the content part */
  uint32_t ulDataSize;
  uint8_t aData[WEBIF_PCK_CONTENT_SIZE]; /*! data */
} WEBIF_HANDLE_HTTP_REQUEST_CONTENT_IND_DATA_T;

/*! Data of WEBIF_GET_HTTP_REQUEST_FIELD_REQ packet */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST
{
  uint32_t uiIsInUri; /*! The field is in the URI or in the HTTP header */
  char aName[WEBIF_FIELD_MAX_SIZE]; /*! Name of the field */
} WEBIF_GET_HTTP_REQUEST_FIELD_REQ_DATA_T;

/*! Data of WEBIF_GET_HTTP_REQUEST_FIELD_CNF packet */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST
{
  char aContent[WEBIF_FIELD_MAX_SIZE]; /*! Field content */
} WEBIF_GET_HTTP_REQUEST_FIELD_CNF_DATA_T;

/*! Data of WEBIF_GENERATE_HTTP_RESPONSE_REQ packet */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST
{
  uint32_t ulStatusCode; /*! HTTP status code */
  uint32_t ulContentLength; /*! Size of the response body */
  char aReasonPhrase[WEBIF_FIELD_MAX_SIZE]; /*! OPTIONAL (V2 only) reason phrase */
} WEBIF_GENERATE_HTTP_RESPONSE_REQ_DATA_T;
#define WEBIF_GENERATE_HTTP_RESPONSE_REQ_DATA_SIZE_V1 (sizeof(uint32_t) * 2)
#define WEBIF_GENERATE_HTTP_RESPONSE_REQ_DATA_SIZE_V2 (sizeof(WEBIF_GENERATE_HTTP_RESPONSE_REQ_DATA_T))

/*! Data of WEBIF_GENERATE_HTTP_RESPONSE_FIELD_RE packet */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST
{
  char aName[WEBIF_FIELD_MAX_SIZE]; /*! Field name */
  char aContent[WEBIF_FIELD_MAX_SIZE]; /*! Field content */
} WEBIF_GENERATE_HTTP_RESPONSE_FIELD_REQ_DATA_T;

/*! Data of WEBIF_GENERATE_HTTP_RESPONSE_CONTENT_REQ packet */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST
{
  uint32_t ulDataSize;
  uint8_t aData[WEBIF_PCK_CONTENT_SIZE]; /*! Data */
} WEBIF_GENERATE_HTTP_RESPONSE_CONTENT_REQ_DATA_T;


/*! WebServer Start request packet. */
typedef HIL_EMPTY_PACKET_T          WEBIF_START_REQ_T;

/*! WebServer Start confirmation packet. */
typedef HIL_EMPTY_PACKET_T          WEBIF_START_CNF_T;


/*!  \} */

/*! \defgroup WEBIF_Packets WebIf Packets
 *  \{ */

/*! Union representing all packet data */
typedef union
{
  WEBIF_HANDLE_HTTP_REQUEST_IND_DATA_T tRequestIndData;
  WEBIF_HANDLE_HTTP_REQUEST_CONTENT_IND_DATA_T tRequestContentIndData;

  WEBIF_GET_HTTP_REQUEST_FIELD_REQ_DATA_T tGetRequestFieldReqData;
  WEBIF_GET_HTTP_REQUEST_FIELD_CNF_DATA_T tGetRequestFieldCnfData;

  WEBIF_GENERATE_HTTP_RESPONSE_REQ_DATA_T tResponseReqData;
  WEBIF_GENERATE_HTTP_RESPONSE_FIELD_REQ_DATA_T tResponseFieldReqData;
  WEBIF_GENERATE_HTTP_RESPONSE_CONTENT_REQ_DATA_T tResponseContentReqData;
} WEBIF_PCK_DATA_T;

/*! WebIf packet */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST
{
  HIL_PACKET_HEADER_T tHead; /*! Packet header */
  WEBIF_PCK_DATA_T tData; /*! Packet data */
} WEBIF_PCK_T;

/*!  \} */

#endif /* WEBIF_API_H_ */
