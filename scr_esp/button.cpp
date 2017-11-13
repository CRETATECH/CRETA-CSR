/***************************************************************************************
* INCLUDE
***************************************************************************************/
#include <ESP8266WiFi.h>
#include "button.h"
#include "device.h"
#include "mqtt.h"
#include "protocol.h"

/***************************************************************************************
* LOCAL VARIABLES
***************************************************************************************/

/***************************************************************************************
* EXTERN VARIABLES
***************************************************************************************/
bool isButtonPressed = false;
/***************************************************************************************
* PUBLIC FUNCTION
***************************************************************************************/
/**
 * @brief       Init all button
 * @param       None
 */
void buttonInit(void){
    pinMode(PIN_BUTTON_CONFIG, INPUT_PULLUP);
}

/**
 * @brief       Button config check
 * @param       None
 * @retval      true
 *              false
 */
bool buttonConfigCheck(void){
    static uint8_t buttonLastStatus = HIGH;
    static uint32_t buttonLastPressed = 0;
    uint8_t buttonStatus = digitalRead(PIN_BUTTON_CONFIG);
    if(buttonStatus != buttonLastStatus){
        if(buttonStatus == LOW){
            buttonLastPressed = millis();
        }
    }
    else{
        if(buttonStatus == LOW){
            if((millis() - buttonLastPressed) > 3000){
                return true;
            }
        }
    }
    buttonLastStatus = buttonStatus;
    return false;  
}


