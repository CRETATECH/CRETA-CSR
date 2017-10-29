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

/***************************************************************************************
* PUBLIC FUNCTION PROTOTYPES
***************************************************************************************/
void protocolInit(void);
//int pars_json(String json);
void protocolDataProcess (String recv_json);
void protocolButtonProcess (void);
int protocolSerialRecv (String *s);
#endif
