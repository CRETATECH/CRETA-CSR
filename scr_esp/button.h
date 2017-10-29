#ifndef __BUTTON_H__
#define __BUTTON_H__

/***************************************************************************************
* INCLUDE
***************************************************************************************/
#include "wishock.h"

/***************************************************************************************
* PUBLIC FUNCTION PROTOTYPES
***************************************************************************************/
void buttonInit(void);
void buttonConfigISRHandler(void);
bool buttonConfigCheck(void);
bool buttonControlCheck(void);

#endif
