#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

/***************************************************************************************
* INCLUDE
***************************************************************************************/
#include "wishock.h"

/***************************************************************************************
* PRIVATE DEFINES
***************************************************************************************/
#define  PROCESS_NORMAL  0
#define  FRAME_ERR       1
#define  PROCESS_ERR     2

enum {
  ERROR_PARSE_JSON = 1,
}error_t;

/***************************************************************************************
* PUBLIC FUNCTION PROTOTYPES
***************************************************************************************/
void protocolInit(void);
int jsonParse(String pJson);
int protocolSerialRecv (String *s);
void protocolSendDatatoMCU (uint8_t* data, int len);
void protocolDataProcess (uint8_t* data, int len);
#endif
