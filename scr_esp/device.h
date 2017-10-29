#ifndef __DEVICE_H__
#define __DEVICE_H__

/***************************************************************************************
* INCLUDE
***************************************************************************************/
#include "wishock.h"
/***************************************************************************************
* PUBLIC DEFINE
***************************************************************************************/
#define DEVICE_ON     0
#define DEVICE_OFF    1
/***************************************************************************************
* PUBLIC FUNCTION PROTOTYPES
***************************************************************************************/
void deviceInit(void);
void deviceOn(void);
void deviceOff(void);
void deviceToggle(void);
void ledDeviceOn(void);
void ledDeviceOff(void);
void ledWifiOn(void);
void ledWifiOff(void);
void ledWifiToggle(void);
int deviceStatus(void);

#endif
