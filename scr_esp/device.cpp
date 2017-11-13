/***************************************************************************************
* INCLUDE
***************************************************************************************/
#include "device.h"
#include "button.h"
#include <ESP8266WiFi.h>

/***************************************************************************************
* PRIVATE DEFINES
***************************************************************************************/

/***************************************************************************************
* PUBLIC VARIABLES
***************************************************************************************/

/***************************************************************************************
* PUBLIC FUNCTIONS
***************************************************************************************/
/**
 * Initialize gpio (led, device)
 */
void deviceInit(void){
    pinMode(PIN_LED_WIFI, OUTPUT);
}

/**
 * Turn led wifi ON
 */
void ledWifiOn(void){
    digitalWrite(PIN_LED_WIFI, LOW);
}
/**
 * Turn led wifi OFF
 */
void ledWifiOff(void){
    digitalWrite(PIN_LED_WIFI, HIGH);
}

/**
 * Toggle led
 */
void ledWifiToggle(void){
    if(digitalRead(PIN_LED_WIFI) == HIGH)
        ledWifiOn();
    else
        ledWifiOff();
}

