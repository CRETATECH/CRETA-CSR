
/*************************************************/
/*                  INCLUDE                      */
/*************************************************/
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "wishock.h"
#include "protocol.h"
#include "button.h"
#include "device.h"
/*************************************************/
/*                  EXTERN VARIABLE              */
/*************************************************/
char gMqttTopicIn[25];
char gMqttTopicOut[25];
WiFiClient gESPClient;
PubSubClient client(gESPClient);
const char* gMqttServer = "iot.eclipse.org";
/*************************************************/
/*                  LOCAL  VARIABLE              */
/*************************************************/
char gClientID[25];

/*************************************************/
/*                  FUCTION PROTOTYPE            */
/*************************************************/
String Get_macID (void);
void mqttCreateClientID (void);
/*************************************************/
/*                  MAIN FUNCTION                */
/*************************************************/
/**
 * @brief       process data received from server
 * @param       topic, payload (data) and length
 * @retval      None
 *              
 */
void callback(char* topic, byte* payload, unsigned int length) {
  String _mqttRecvData = "";
  #ifdef DEBUG
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
  #endif
  for (int i = 0; i < length; i++) {
    _mqttRecvData += String((char)payload[i]);
    Serial.write(payload[i]);
  }
  #ifdef DEBUG
    Serial.println(_mqttRecvData);
    Serial.println();
  #endif
  //protocolDataProcess(_mqttRecvData);
}
/**
 * @brief       create topic for mqtt to sub and pub, create client ID
 * @param       none
 * @retval      none
 *              
 */
void mqttCreateTopic(void)
{
  String _nameTopic = "ESP" + Get_macID();
  
  String _topic = _nameTopic + "/master";
  _topic.toCharArray(gMqttTopicIn, 24);
  
  _topic = _nameTopic + "/slave";
  _topic.toCharArray(gMqttTopicOut, 23);
  #ifdef DEBUG
    Serial.print("\r\ntopicIn: ");
    Serial.println(gMqttTopicIn);
    Serial.print("topicOut: ");
    Serial.println(gMqttTopicOut);
  #endif
  mqttCreateClientID ();
  client.setServer(gMqttServer, 1883);
  client.setCallback(callback);

}
/**
 * @brief       Subscribe a topic with qos 0
 * @param       None
 * @retval      None
 *              
 */
void mqttSubscribe(void)
{
   client.subscribe(gMqttTopicIn, 0); 
}

/**
 * @brief       connect to mqtt server with ID : gClientID
 * @param       None
 * @retval      0: connect failed
 *              1: connect success
 */
int mqttConnect (void)
{
  return client.connect(gClientID);
}
/**
 * @brief       check connect to mqtt server
 * @param       None
 * @retval      1: connected
 *              0: not connected
 */
int mqttConnected (void)
{
  return client.connected();
}

/**
 * @brief       Publish a topic with qos 0
 * @param       pJsonOut: String json you want to publish
 * @retval      None
 *              
 */
void mqttPublish (String pJsonOut)
{
  char _dataOut[100];
  pJsonOut.toCharArray(_dataOut, pJsonOut.length() + 1);
  client.publish(gMqttTopicOut, _dataOut);
  #ifdef DEBUG
    Serial.print("Publish json: ");
    Serial.print(_dataOut);
  #endif
}

void mqttPubTest (void)
{
  client.publish("topicTest", gClientID);
}

/**
 * @brief       maintain connect to server and process in coming message by call callback function
 * @param       None
 * @retval      None
 *              
 */
void mqttLoop (void)
{
  client.loop();
}
/*************************************************/
/*                  LOCAL FUCTION                */
/*************************************************/
/**
 * @brief       get macID of ESP8266
 * @param       None
 * @retval      macid by string
 *              
 */
String Get_macID (void)
{
  String _val;
  byte _mac[6];
  WiFi.macAddress(_mac);
  for (int i = 0; i < 6; i++)
  {
    if (_mac[i] < 0x10)
      _val += '0' + String(_mac[i], HEX);
    else _val += String(_mac[i], HEX);
  }
  return _val;
}

/**
 * @brief       create clientID to connect server
 * @param       None
 * @retval      None
 *              
 */
void mqttCreateClientID (void)
{
  String _temp = Get_macID();
  _temp = "ESP" + _temp;
  _temp.toCharArray(gClientID, 25);
}

