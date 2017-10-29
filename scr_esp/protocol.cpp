/***************************************************************************************
* INCLUDE
***************************************************************************************/
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include "wishock.h"
#include "button.h"
#include "device.h"
#include "protocol.h"
#include "mqtt.h"
/***************************************************************************************
* EXTERN VARIABLES
***************************************************************************************/
String gID = "";
String gFunc = "";
String gAddr ="";
String gData = "";
/***************************************************************************************
* LOCAL FUNCTIONS PROTOTYPES
***************************************************************************************/
int protocolCtrlFunc_Process (String pData);
int protocolDataFunc_Process (void);
/***************************************************************************************
* PUBLIC FUNCTION
***************************************************************************************/
/**
 * @brief       Initialize protocol
 * @param       none
 * @retval      None
 *              
 */
void protocolInit(void)
{


}

/**
 * @brief       parse Json
 * @param       pJson
 * @retval      0: parse failed
 *              1: parse success
 */
int jsonParse(String pJson)
{
  DynamicJsonBuffer _jsonBuffer;
  JsonObject& root = _jsonBuffer.parseObject(pJson);
  if (!root.success())
  {
    #ifdef DEBUG
      Serial.println("pars fail");
    #endif
    return 0;
  } 
  String _a      = root["ID"];
  gID = _a;
  String _b      = root["FUNC"];
  gFunc = _b;
  String _c      = root["ADDR"];
  gAddr = _c;
  String _d      = root["DATA"];
  gData = _d;
  #ifdef DEBUG
    Serial.println("pars ok");
  #endif
  return 1; 
}

/**
 * @brief       create Json to prepare for publishing
 * @param       pFunc, pAddr, pData
 * @retval      String json
 *              
 */
String protocolCreateJson (String pFunc, String pAddr, String pData)
{
  
   String _stringout = "{\"ID\" : \"ESP"  + Get_macID() + "\", \"FUNC\" : \"" + pFunc + "\", \"ADDR\" : \"" + pAddr + "\", \"DATA\" : \"" + pData + "\"}";
   return _stringout;
}

int protocolSerialRecv (String *s)
{

  char c;

  if(Serial.available())
  {
    c = Serial.read();
    if(c == '{') 
    {
      *s += c;
      uint32_t _start = millis();
      while((c != '}')&&(millis() < (_start + 100)))
      {
        if(Serial.available()) 
        {
          c = Serial.read();
          if (c != 0x5C) // khac '\'
            *s += c;
        }
      }
    }
    if(c == '}')
    {
      return 1;
    } 
    else
    {
      return 0;
    }
  }
  
}

/**
 * @brief       process data when button is pressed
 * @param       none
 * @retval      None
 * no toggle device here, toggle in interrupt, just check and publish             
 *              
 */
void protocolButtonProcess (void)
{
  uint32_t _time_out;
  _time_out = millis();
  if (deviceStatus() == DEVICE_ON)
  {
    while (!mqttConnected())
    {
      if ((millis() - _time_out) > 300)
        break;
    }
      mqttPublish(protocolCreateJson("Data", "1", "On")); 
  }
  else
  {
    while (!mqttConnected())
    {
      if ((millis() - _time_out) > 300)
        break;
    }
      mqttPublish(protocolCreateJson("Data", "1", "Off")); 
  }
}

/**
 * @brief       process data received from server
 * @param       pJsonRecv
 * @retval      None
 *              
 */
void protocolDataProcess (String pJsonRecv)
{
  int _temp;
  /* parse json */
  if (jsonParse(pJsonRecv) == 0)
  {
    mqttPublish(protocolCreateJson("Error", "1", "ErrFrame"));  
  }
  else
  {
    /* process */
    if (gFunc == "Ctrl")
    {
      #ifdef DEBUG
        Serial.println("func ctrl");
      #endif
      _temp = protocolCtrlFunc_Process(gData);
    }
    else if (gFunc == "Data")
    {
      #ifdef DEBUG
        Serial.println("func data");
      #endif
      _temp  = protocolDataFunc_Process();
    }
  }
}

/**
 * @brief       process Ctrl function
 * @param       none
 * @retval      PROCESS_NORMAL
 *              PROCESS_ERR
 *              FRAME_ERR
 */
int protocolCtrlFunc_Process (String pData)
{
  if (gAddr == "1")
  {
    if (pData == "On")
    {
      deviceOn();
      delay(50);
      if (deviceStatus() == DEVICE_ON)
      {
        mqttPublish(protocolCreateJson("Ctrl", "1", "On")); 
        return PROCESS_NORMAL;
      }
      else
      {
        mqttPublish(protocolCreateJson("Error", "1", "ErrProcess"));
        return PROCESS_ERR;
      }
    }
    else if (pData == "Off")
    {
      deviceOff();
      delay(50);
      if (deviceStatus() == DEVICE_OFF)
      {
        mqttPublish(protocolCreateJson("Ctrl", "1", "Off"));
        return PROCESS_NORMAL;
      }
      else
      {
        mqttPublish(protocolCreateJson("Error", "1", "ErrProcess"));
        return PROCESS_ERR;
      }
    }
    else
    {
      mqttPublish(protocolCreateJson("Error", "1", "ErrFrame"));  //wrong function
      return PROCESS_ERR;      
    }
  }
  else
  {
    mqttPublish(protocolCreateJson("Error", "1", "ErrFrame"));  //wrong addr
    return PROCESS_ERR;  
  }
}

/**
 * @brief       process Data function
 * @param       none
 * @retval      PROCESS_NORMAL
 *              FRAME_ERR
 */
int protocolDataFunc_Process (void)
{
  if (gAddr == "1")
  {
    if (deviceStatus() == DEVICE_ON)
    {
      mqttPublish(protocolCreateJson("Data", "1", "On"));
    }
    else
      mqttPublish(protocolCreateJson("Data", "1", "Off"));
    return PROCESS_NORMAL; 
  }
  else
    return FRAME_ERR;
}


