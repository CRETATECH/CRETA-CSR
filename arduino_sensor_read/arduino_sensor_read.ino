#include <ArduinoJson.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include "defineHardware.h"

OneWire onewire(ONE_WIRE_BUS);
DallasTemperature ds18b20(&onewire);

String ss = "";
String id = "";
String func = "";
String addr = "";
String data = "";

void setup() {
  /* Set chan doc cam bien muc nuoc */
  /* Day thuc chat la mot cam bien thuong dong */
  pinMode(WATER_SENSOR, INPUT_PULLUP);
  pinMode(DEVICE_1, OUTPUT);
  pinMode(DEVICE_2, OUTPUT);
  /* Mo cong uart de giao tiep */
  Serial.begin(115200);
  while(!Serial);
  /* Khoi dong cam bien nhiet do */
  ds18b20.begin();
}

void loop() {
  if(true == uartGetFrame(&ss)){
    /* Check JSON parse */
    if(true == parseJson(ss)){
      if(func == "Data") {
        if(addr == "1") {
          deviceStatusSend("Ctrl", addr, (digitalRead(DEVICE_1) == HIGH)? "Off" : "On");
        }
        else if(addr == "2") {
          deviceStatusSend("Ctrl", addr, (digitalRead(DEVICE_2) == HIGH)? "Off" : "On");
        }
        else if(addr == "3") {
          dataTempSend();
        }
        else if(addr == "4") {
          dataWaterSend();
        }
      } else if(func == "Ctrl") {
        if(true == deviceControl(addr, data)){
          /* Gui frame thong bao trang thai moi */
          if(addr == "1") {
            if(digitalRead(DEVICE_1) == HIGH) {
              deviceStatusSend("Ctrl", addr, "Off");
            } else {
              deviceStatusSend("Ctrl", addr, "On");
            }
          } else if(addr == "2") {
            if(digitalRead(DEVICE_2) == HIGH) {
              deviceStatusSend("Ctrl", addr, "Off");
            } else {
              deviceStatusSend("Ctrl", addr, "On");
            }
          }
        }
      }
    }
    /* Clear */
    id = "";
    func = "";
    addr = "";
    data = "";
  }
  //dataTempSend();
  //dataWaterSend();
  //delay(5000);
}

void deviceStatusSend(String func, String addr, String data){
  String frameTx = "";
  frameTx = "{\"ID\":\"CSR\",\"FUNC\":\"";
  frameTx += func;
  frameTx += "\",\"ADDR\":\"";
  frameTx += addr;
  frameTx += "\",\"DATA\":\"";
  frameTx += data;
  frameTx += "\"}";
  Serial.println(frameTx);
}

bool deviceControl(String addr, String data){
  uint8_t _pin = "";
  if(addr == "1") {
    _pin = DEVICE_1;
  } else if (addr == "2") {
    _pin = DEVICE_2;
  }
  if(_pin == "") {
    return false;
  }
  if(data == "On") {
    digitalWrite(_pin, LOW);
  } else if (data == "Off") {
    digitalWrite(_pin, HIGH);
  } else {
    return false;
  }
  return true;
}

bool parseJson(String json) {
  DynamicJsonBuffer _jsonBuffer;
  JsonObject& root = _jsonBuffer.parseObject(json);
  if (!root.success())
  {
    return false;
  }
  String _a = root["ID"];
  id = _a;
  String _b = root["FUNC"];
  func = _b;
  String _c = root["ADDR"];
  addr = _c;
  String _d = root["DATA"];
  data = _d;
  return true;
}

bool uartGetFrame(String* s) {
  char c = "";
  *s = "";
  if(Serial.available()){
    c = Serial.read();
    if(c == '{') {
      *s += c;
      uint32_t _start = millis();
      while((c != '}')&&(millis() < (_start + 100))){
        if(Serial.available()) {
          c = Serial.read();
          *s += c;
        }
      }
    }
  }
  if(c == '}') {
    return true;
  } else {
    return false;
  }
}

void dataWaterSend() {
  String frameTx = "";
  /* Gui frame muc nuoc */
  frameTx = "{\"ID\":\"CSR\",\"FUNC\":\"Data\",\"ADDR\":\"4\",\"DATA\":\"";
  bool rWater = isWaterThreshold();
  String water = (rWater)? "Upper" : "Lower";
  frameTx += water;
  frameTx += "\"}";
  Serial.println(frameTx);
}

void dataTempSend() {
  String frameTx = "";
  /* Gui frame nhiet do */
  frameTx = "{\"ID\":\"CSR\",\"FUNC\":\"Data\",\"ADDR\":\"3\",\"DATA\":\"";
  //float ftemp = readTemperatureSensor();
  float rTemp = 30.03;
  String temp(rTemp);
  frameTx += temp;
  frameTx += "\"}";
  Serial.println(frameTx);
}

bool isWaterThreshold() {
  return (digitalRead(WATER_SENSOR) == HIGH)? true : false;
}

float readTemperatureSensor() {
  ds18b20.requestTemperatures();
  return ds18b20.getTempCByIndex(0);
}

