#include <ArduinoJson.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include <TimerOne.h>
#define ONE_WIRE_BUS 2
#define WATER_SENSOR 3 // Pin 3
#define DEVICE_1 4
#define DEVICE_2 5
#define BUTTON_1 6
#define BUTTON_2 7

OneWire onewire(ONE_WIRE_BUS);
DallasTemperature sensors(&onewire);

int Tth = 27; //temperature threshold

String ss = "";
String id = "";
String func = "";
String addr = "";
String data = "";
void Serial_Proc (void);

void setup() {
  /* Set chan doc cam bien muc nuoc */
  /* Day thuc chat la mot cam bien thuong dong */
  pinMode(WATER_SENSOR, INPUT_PULLUP);
  pinMode(DEVICE_1, OUTPUT);
  pinMode(DEVICE_2, OUTPUT);
  pinMode(BUTTON_1, INPUT_PULLUP);
  pinMode(BUTTON_2, INPUT_PULLUP);
  deviceControl("0101", "0");
  deviceControl("0102", "0");
  delay(500); //delay to not recv sh*t data from esp reset
  /* Mo cong uart de giao tiep */
  Serial.begin(115200);
  while(!Serial);
  
  Timer1.initialize(1000000); // khởi tạo timer 1 đến 1 giây
  Timer1.attachInterrupt(Blink); 
  
  /* Khoi dong cam bien nhiet do */
  sensors.begin();
}

void loop() 
{
  Serial_Proc ();
  button1Pressed();
  button2Pressed();
  waterCheck();
}

void Serial_Proc (void)
{
  if(true == uartGetFrame(&ss))
  {
    /* Check JSON parse */
    if(true == parseJson(ss))
    {
      if(func == "002") 
      {
        if(addr == "0101") 
        {
          deviceStatusSend("002", addr, (digitalRead(DEVICE_1) == HIGH)? "0" : "1");
        }
        else if(addr == "0102") 
        {
          deviceStatusSend("002", addr, (digitalRead(DEVICE_2) == HIGH)? "0" : "1");
        }
        else if(addr == "0201") 
        {
          dataTempSend();
        }
        else if(addr == "0401") 
        {
          dataWaterSend();
        }
        else
          sendErrorFrame("004"); //sai addr
      } else if(func == "001")
        {
          if(true == deviceControl(addr, data))
          {
            /* Gui frame thong bao trang thai moi */
            if(addr == "0101") 
            {
              if(digitalRead(DEVICE_1) == HIGH) 
              {
                deviceStatusSend("001", addr, "0");
              } else 
                {
                deviceStatusSend("001", addr, "1");
                }
            } else if(addr == "0102") 
              {
              if(digitalRead(DEVICE_2) == HIGH) 
              {
                deviceStatusSend("001", addr, "0");
              } else {
                deviceStatusSend("001", addr, "1");
              }
            }
          }
        }
      else {
        sendErrorFrame("002");
      }
    }
    /* Clear */
    id = "";
    func = "";
    addr = "";
    data = "";
  }

}

void button1Pressed() {
  static uint8_t _lastStatus = HIGH;
  static uint32_t _lastPressed = 0;
  static bool _isPressed = false;
  uint8_t reading = digitalRead(BUTTON_1);
  if(reading == LOW) {
    if(reading != _lastStatus) {
      _lastPressed = millis();
    }
    else if((millis() - _lastPressed) > 250) {
      if(_isPressed == false) {
        digitalWrite(DEVICE_1, !digitalRead(DEVICE_1));
        deviceStatusSend("002", "0101", (digitalRead(DEVICE_1) == LOW)? "1" : "0");
        _isPressed = true;
      }
    }
  }
  else {
    _isPressed = false;
  }
  _lastStatus = reading;
}

void button2Pressed() {
  static uint8_t _lastStatus = HIGH;
  static uint32_t _lastPressed = 0;
  static bool _isPressed = false;
  uint8_t reading = digitalRead(BUTTON_2);
  if(reading == LOW) {
    if(reading != _lastStatus) {
      _lastPressed = millis();
    }
    else if((millis() - _lastPressed) > 250) {
      if(_isPressed == false) {
        digitalWrite(DEVICE_2, !digitalRead(DEVICE_2));
        deviceStatusSend("002", "0102", (digitalRead(DEVICE_2) == LOW)? "1" : "0");
        _isPressed = true;
      }
    }
  }
  else {
    _isPressed = false;
  }
  _lastStatus = reading;
}

void waterCheck() {
  static uint8_t _lastStatus = HIGH;
  static uint32_t _lastPressed = 0;
  static bool _isPressed = false;
  uint8_t reading = digitalRead(WATER_SENSOR);
  if(reading != _lastStatus) {
    _lastPressed = millis();
    _isPressed = false;
  }
  else {
    if((millis() - _lastPressed) > 500) {
      if(_isPressed == false) {
        deviceStatusSend("002", "0401", (reading == LOW)? "0" : "1");
        _isPressed = true;
      }
    }
  }
  _lastStatus = reading;
}

void sendErrorFrame(String code) {
  String frameTx = "";
  frameTx = "{\"USER\":\"CSR5ccf7fd16259\",\"FUNC\":\"";
  frameTx += "003";
  frameTx += "\",\"ADDR\":\"";
  frameTx += "";
  frameTx += "\",\"DATA\":\"";
  frameTx += code;
  frameTx += "\"}";
  Serial.println(frameTx);  
}

void deviceStatusSend(String func, String addr, String data){
  String frameTx = "";
  frameTx = "{\"USER\":\"CSR5ccf7fd16259\",\"FUNC\":\"";
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
  if(addr == "0101") {
    _pin = DEVICE_1;
  } else if (addr == "0102") {
    _pin = DEVICE_2;
  }
  if(_pin == "") {
    sendErrorFrame("004"); //sai addr
    return false;
  }
  if(data == "1") {
    digitalWrite(_pin, LOW);
  } else if (data == "0") {
    digitalWrite(_pin, HIGH);
  } else {
    sendErrorFrame("003");//sai data
    return false;
  }
  return true;
}

bool parseJson(String json) {
  DynamicJsonBuffer _jsonBuffer;
  JsonObject& root = _jsonBuffer.parseObject(json);
  if (!root.success())
  {
    sendErrorFrame("001");
    return false;
  }
  String _a = root["USER"];
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
  frameTx = "{\"USER\":\"CSR5ccf7fd16259\",\"FUNC\":\"002\",\"ADDR\":\"0401\",\"DATA\":\"";
  bool rWater = isWaterThreshold();
  String water = (rWater)? "1" : "0";
  frameTx += water;
  frameTx += "\"}";
  Serial.print(frameTx);
}

void dataTempSend() {
  String frameTx = "";
  /* Gui frame nhiet do */
  frameTx = "{\"USER\":\"CSR5ccf7fd16259\",\"FUNC\":\"002\",\"ADDR\":\"0201\",\"DATA\":\"";

  float rTemp = readTemperatureSensor();
  String temp(rTemp);
  frameTx += temp;
  frameTx += "\"}";
  Serial.print(frameTx);
}

bool isWaterThreshold() {
  return (digitalRead(WATER_SENSOR) == HIGH)? true : false;
}

float readTemperatureSensor() {
  sensors.requestTemperatures();
  delay(50);
  return sensors.getTempCByIndex(0);
}

void Blink (void)
{
  static int i = 0;
  i++;
  if (i > 10)
  {
    dataTempSend();
    //delay(50);
    //dataWaterSend();
    i = 0;
  }
//  int temp;
//  temp = (int)(readTemperatureSensor());
//  if (temp >= Tth)
//    deviceControl ("2", "On");
//  else if (temp < Tth)
//    deviceControl ("2", "Off"); 
}



