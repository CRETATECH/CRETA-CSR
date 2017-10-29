/***************************************************************************************
* INCLUDE
***************************************************************************************/
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <EEPROM.h>
#include "user_interface.h"
#include "state.h"
#include "wishock.h"
#include "button.h"
#include "device.h"
#include "protocol.h"
#include "mqtt.h"
/***************************************************************************************
* LOCAL TYPEDEFS
***************************************************************************************/


/***************************************************************************************
* LOCAL DEFINES
***************************************************************************************/


/***************************************************************************************
* LOCAL FUNCTIONS PROTOTYPES
***************************************************************************************/
void EEPROM_Write_ConfigFlag(uint8_t pData);
void Wifi_Connect(void);
uint8_t EEPROM_Read_ConfigFlag(void);
void TimerISRHandler (void);
void stateConfig(void);
void stateControl(void);
/***************************************************************************************
* LOCAL VARIABLES
***************************************************************************************/
fsm_t gState = STATE_CONTROL;
os_timer_t gTimer;
led_status_t gLedFlag;
uint32_t _time = 0;
/***************************************************************************************
* EXTERN VARIABLES
***************************************************************************************/
extern char gClientID[25];

/***************************************************************************************
* PUBLIC FUNCTIONS
***************************************************************************************/
/**
 * @brief       main finite state machine
 * @param       none
 * @retval      none
 */
void stateMachine(void){
    switch(gState){
        case STATE_CONFIG:
            stateConfig();
        break;
        case STATE_CONTROL:
            stateControl();
        break;
        default:
        break;
    }
}

/**
 * @brief       update button control status
 * @param       none
 * @retval      none
 */
void stateUpdate(void){
    /* gState update (if, elseif,...) */
    /* check config button is pressed or not in control state */
    if (true == buttonConfigCheck())
      gState = STATE_CONFIG;
}

/**
 * @brief       Init all gpio, timer, mqtt, protocol before go to state machine
 * @param       none
 * @retval      none
 */
void stateSetup (void)
{
    #ifdef DEBUG
      Serial.println("");
    #endif
    int vFlagConfig = 0;
    /* Setup serial */
    Serial.begin(115200);
    /* Setup button */
    buttonInit();
    /* Setup device */
    deviceInit();
    /* Setup timer */
    os_timer_disarm(&gTimer);
    os_timer_setfn(&gTimer, (os_timer_func_t *)TimerISRHandler, NULL);
    os_timer_arm(&gTimer, 400, 1);
    /* Anything else here */


    protocolInit();
    mqttCreateTopic();
    if (EEPROM_Read_ConfigFlag() == 0x05)
      vFlagConfig = 1; // configed
    else vFlagConfig = 0; // not configed yet

    if (vFlagConfig == 0)
      gState = STATE_CONFIG;
    else gState = STATE_CONTROL;
}
/***************************************************************************************
* LOCAL FUNCTIONS
***************************************************************************************/
/**
 * @brief       use smart config to config wifi
 * @param       none
 * @retval      none
 */
void stateConfig(void)
{  
  /* stop to sure can start again */
  /* in some situation, config with wrong pass can make smartConfig work wrong */
  WiFi.stopSmartConfig();
  gLedFlag = LED_STATUS_OFF;
  _time = millis();
  WiFi.mode(WIFI_STA);
  if(digitalRead(PIN_BUTTON_CONFIG) != LOW)
  {
  #ifdef DEBUG
      Serial.println("\r\nStart config...");
  #endif
    WiFi.beginSmartConfig();
    while(1)
    {
      delay(1000);
      if (true == buttonConfigCheck())
      {
          gLedFlag = LED_STATUS_ON;
          while(digitalRead(PIN_BUTTON_CONFIG) == LOW){
              delay(1);
          }
          gState = STATE_CONTROL;
          WiFi.stopSmartConfig();
          return;
      }
      if (WiFi.smartConfigDone())
      {
      #ifdef DEBUG
        Serial.println("Config done!!!\r\n");
      #endif
        break;
      }
    }
    /* write 0x05 to address 0x10 to inform that configed */
    EEPROM_Write_ConfigFlag (0x05);
    gState = STATE_CONTROL;
  }
}




/**
 * @brief       check connect to router, server
 * @param       none
 * @retval      none
 */
void stateControl(void)
{
    gLedFlag = LED_STATUS_ON;
    /* Check router connect */
    if (WiFi.status() == WL_CONNECTED)
    {
      /* Check connect server */
      if (!mqttConnected())
      {
        #ifdef DEBUG
          Serial.println("MQTT Server not connect...");
        #endif
        if (mqttConnect())
        {
          mqttSubscribe();
          #ifdef DEBUG
            Serial.println("MQTT Server connected, subscribe to topic");
          #endif
        }
      }
      else
      {
//        if (buttonControlCheck() == true)
//        {
//          protocolButtonProcess();
//        }
        String a;
        
        if (protocolSerialRecv(&a) == 1)
          mqttPublish(a);
        mqttLoop();
      }
    }
    else Wifi_Connect();
}

/**
 * @brief       write data to EEPROM (address is 0x10)
 * @param       pData: data want to write
 * @retval      none
 */
void EEPROM_Write_ConfigFlag(uint8_t pData){
    EEPROM.begin(50);
    EEPROM.write(EEPROM_ADDRESS, pData);
    EEPROM.commit();
}

/**
 * @brief       Read data from EEPROM
 * @param       none
 * @retval      data read
 */
uint8_t EEPROM_Read_ConfigFlag(void){
    EEPROM.begin(50);
    return EEPROM.read(EEPROM_ADDRESS);
}

/**
 * @brief       connect to the last wifi connected
 * @param       none
 * @retval      none
 */
void Wifi_Connect (void)
{
  gLedFlag = LED_STATUS_BLINK;
  WiFi.begin();
  #ifdef DEBUG
    WiFi.printDiag(Serial);
    Serial.print("\r\nWiFi connecting");
  #endif
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(100);
    #ifdef DEBUG
      Serial.print('.');
    #endif
    if (true == buttonConfigCheck())
    {
      gState = STATE_CONFIG;
      return;
    }
  }
  mqttConnect();
  mqttSubscribe();
  #ifdef DEBUG
    Serial.println("\r\nWiFi connected!!!");
  #endif
  mqttPubTest();
  #ifdef DEBUG
    Serial.println("publish test");
  #endif
}

/**
 * @brief       timer interrupt service routine
 * @param       none
 * @retval      none
 */
void TimerISRHandler (void)
{
  if (gLedFlag == LED_STATUS_BLINK)
  {
    ledWifiToggle();
  }
  else if (gLedFlag == LED_STATUS_ON) {
        ledWifiOn();
        if (gState == STATE_CONFIG)
        {
          if ((millis() - _time) > 1000)
          {
            gLedFlag = LED_STATUS_OFF;
            _time = millis();
          }
        }
  }

  else {
    ledWifiOff();
    if ((millis() - _time) > 10000)
    {
     gLedFlag = LED_STATUS_ON;
     _time = millis();
    }
  }


}
