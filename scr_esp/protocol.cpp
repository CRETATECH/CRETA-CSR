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
String gUSER = "";
String gFunc = "";
String gAddr ="";
String gData = "";
/***************************************************************************************
* LOCAL FUNCTIONS PROTOTYPES
***************************************************************************************/

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
 * @brief       create Json to prepare for publishing
 * @param       pFunc, pAddr, pData
 * @retval      String json
 *              
 */
String protocolCreateJson (String pFunc, String pAddr, String pData)
{
  
   String _stringout = "{\"USER\" : \"CSR"  + Get_macID() + "\", \"FUNC\" : \"" + pFunc + "\", \"ADDR\" : \"" + pAddr + "\", \"DATA\" : \"" + pData + "\"}";
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
 * @brief       process data received from server
 * @param       pJsonRecv
 * @retval      None
 *              
 */
void protocolDataProcess (uint8_t* data, int len)
{
  String dataString = "";
  for (int i = 0; i < len; i++)
  {
    dataString += (String)((char)data[i]); //change data from char to string
  }
  /* parse json */
  if (jsonParse(dataString) == 0)
  {
    #ifdef DEBUG
      Serial.println("FAILED: cant parse json");
    #endif
    mqttPublish(protocolCreateJson("003", "", "001")); //parse json failed, addr empty  
  }
  else
  {
    #ifdef DEBUG
      Serial.println("SUCCESS: parse json ok");
    #endif    
    // send all data to mcu
    #ifdef DEBUG
      Serial.println("PROCESS: send data to mcu");
    #endif     
    protocolSendDatatoMCU(data, len);
  }
}


void protocolSendDatatoMCU (uint8_t* data, int len)
{
  for (int i = 0; i < len; i++)
    Serial.write(data[i]);
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
    return 0;
  }
  else
  { 
    String _a      = root["USER"];
    gUSER = _a;
    String _b      = root["FUNC"];
    gFunc = _b;
    String _c      = root["ADDR"];
    gAddr = _c;
    String _d      = root["DATA"];
    gData = _d;
    return 1; 
  }
}

