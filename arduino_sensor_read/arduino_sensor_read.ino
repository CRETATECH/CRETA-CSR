#include "defineHardWare.h"
#include <DallasTemperature.h>
#include <OneWire.h>
#include <TimerOne.h>
#define ONE_WIRE_BUS 2
#define WATER_SENSOR 3 // Pin 3
#define DEVICE_1 4
#define DEVICE_2 5

OneWire onewire(ONE_WIRE_BUS);
DallasTemperature sensors(&onewire);

int Tth = 27; //temperature threshold

frame_t frameRx;

void setup() {
  /* Set chan doc cam bien muc nuoc */
  /* Day thuc chat la mot cam bien thuong dong */
  pinMode(WATER_SENSOR, INPUT_PULLUP);
  pinMode(DEVICE_1, OUTPUT);
  pinMode(DEVICE_2, OUTPUT);
  pinMode(BUTTON_1, INPUT_PULLUP);
  pinMode(BUTTON_2, INPUT_PULLUP);
  deviceControl("1", "Off");
  deviceControl("2", "Off");
  delay(500); //delay to not recv sh*t data from esp reset
  /* Mo cong uart de giao tiep */
  Serial.begin(115200);
  while(!Serial);
  
  /* Khoi dong cam bien nhiet do */
  sensors.begin();
}

void loop() 
{
  Serial_Proc();
  button1Pressed();
  button2Pressed();
}

void Serial_Proc (void) {
  if(uartGetFrame(&frameRx) == true) {
    /* Function control */
    if(frameRx.framefunc == 0x01) {
      if((frameRx.frameaddr&0xFF00) == 0x0100) {
        deviceControl((uint8_t)frameRx.frameaddr, frameRx.framedata[0]);
      }
    }
    /* Function data */
    else if(frameRx.framefunc == 0x02) {
      if((frameRx.frameaddr&0xFF00) == 0x0100) {
        uint8_t _pin = 0xFF;
        uint8_t _status = 0xFF;
        if((uint8_t)(frameRx.frameaddr&0x00FF) == 0x01) {
          _pin = DEVICE_1;
        } else if ((uint8_t)(frameRx.frameaddr&0x00FF) == 0x02) {
          _pin = DEVICE_2;
        }
        _status = ((digitalRead(_pin) == LOW)? 0x01 : 0x00);
        deviceStatusSend(0x02, (uint8_t)(frameRx.frameaddr&0x00FF), _status);
      }
      else if(frameRx.frameaddr == 0x0201) {
        dataTempSend();
      }
      else if(frameRx.frameaddr == 0x0401) {
        dataWaterSend();
      }
    }
  }
  /* Reset */
  frameRx.framelength = 0;
  frameRx.frametype = 0;
  frameRx.framefunc = 0;
  frameRx.frameaddr = 0;
  free(frameRx.framedata);
  frameRx.framechecksum = 0;
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
    else if((millis() - _lastPressed) > 150) {
      if(_isPressed == false) {
        digitalWrite(DEVICE_1, !digitalRead(DEVICE_1));
        deviceStatusSend(0x02, 0x01, (digitalRead(DEVICE_1) == LOW)? 0x01 : 0x00);
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
  uint8_t reading = digitalRead(BUTTON_1);
  if(reading == LOW) {
    if(reading != _lastStatus) {
      _lastPressed = millis();
    }
    else if((millis() - _lastPressed) > 150) {
      if(_isPressed == false) {
        digitalWrite(DEVICE_2, !digitalRead(DEVICE_2));
        deviceStatusSend(0x02, 0x02, (digitalRead(DEVICE_2) == LOW)? 0x01 : 0x00);
        _isPressed = true;
      }
    }
  }
  else {
    _isPressed = false;
  }
  _lastStatus = reading;
}

void deviceStatusSend(uint8_t func, uint8_t addr, uint8_t data){
  Serial.write(0x27); // Start
  Serial.write(0x07); // Start
  Serial.write(0x01); // Lenght
  Serial.write(0x02); // Type
  Serial.write(func); // Function
  Serial.write(0x01); // Addr
  Serial.write(addr); // Addr
  Serial.write(data);
  uint16_t _sum = 4 + func + addr + data;
  Serial.write((uint8_t)(_sum >> 8));
  Serial.write((uint8_t)_sum);
}

bool deviceControl(uint8_t addr, uint8_t data){
  uint8_t _pin = 0xFF;
  uint8_t _status = 0xFF;
  if(addr == 0x01) {
    _pin = DEVICE_1;
  } else if (addr == 0x02) {
    _pin = DEVICE_2;
  }
  if(_pin == 0xFF) {
    return false;
  }
  if(data == 0x01) {
    digitalWrite(_pin, LOW);
    _status = (digitalRead(_pin) == LOW)? 0x01 : 0x00;
    deviceStatusSend(0x01, addr, _status);
  } else if (data == 0x00) {
    digitalWrite(_pin, HIGH);
    _status = (digitalRead(_pin) == LOW)? 0x01 : 0x00;
    deviceStatusSend(0x01, addr, _status);
  } else {
    return false;
  }
  return true;
}

bool uartGetFrame(frame_t* frame) {
  char c = 0;
  uint32_t _timeout;
  if(Serial.available()) {
    /* Get START element */
    c = Serial.read();
    if(c != 0x27) {
      return false;
    }
    _timeout = millis();
    while(!Serial.available()) {
      if((millis() - _timeout) > 200) {
        return false;
      }
    }
    c = Serial.read();
    if(c != 0x07) {
      return false;
    }
    /* Get length */
    _timeout = millis();
    while(!Serial.available()) {
      if((millis() - _timeout) > 200) {
        return false;
      }
    }
    c = Serial.read();
    frame->framelength = c;
    /* Get type */
    _timeout = millis();
    while(!Serial.available()) {
      if((millis() - _timeout) > 200) {
        return false;
      }
    }
    c = Serial.read();
    frame->frametype = c;
    /* Get function */
    _timeout = millis();
    while(!Serial.available()) {
      if((millis() - _timeout) > 200) {
        return false;
      }
    }
    c = Serial.read();
    frame->framefunc = c;
    /* Get Addr */
    _timeout = millis();
    while(!Serial.available()) {
      if((millis() - _timeout) > 200) {
        return false;
      }
    }
    c = Serial.read();
    frame->frameaddr = (uint16_t)c << 8;
    _timeout = millis();
    while(!Serial.available()) {
      if((millis() - _timeout) > 200) {
        return false;
      }
    }
    c = Serial.read();
    frame->frameaddr |= (uint16_t)c;
    /* Get data */
    free(frame->framedata);
    frame->framedata = (uint8_t*)malloc(frame->framelength);
    int datanum = 0;
    _timeout = millis();
    while(datanum < frame->framelength) {
      if((millis() - _timeout) > 200) {
        return false;
      }
      if(Serial.available()) {
        c = Serial.read();
        frame->framedata[datanum] = c;
        datanum++;
      }
    }
    /* Get Checksum */
    _timeout = millis();
    while(!Serial.available()) {
      if((millis() - _timeout) > 200) {
        return false;
      }
    }
    c = Serial.read();
    frame->framechecksum = (uint16_t)c << 8;
    _timeout = millis();
    while(!Serial.available()) {
      if((millis() - _timeout) > 200) {
        return false;
      }
    }
    c = Serial.read();
    frame->framechecksum |= (uint16_t)c;
    uint16_t _sum = 0;
    _sum += (uint16_t)frame->framelength;
    _sum += (uint16_t)frame->frametype;
    _sum += (uint16_t)frame->framefunc;
    _sum += (uint16_t)(frame->frameaddr >> 8);
    _sum += (uint16_t)(frame->frameaddr & 0x00FF);
    datanum = 0;
    while(datanum < frame->framelength) {
      _sum += frame->framedata[datanum];
      datanum++;
    }
    if(_sum != frame->framechecksum) {
      return false;
    }
    else {
      return true;
    }
  }
  return false;
}

void dataWaterSend() {
  Serial.write(0x27); // Start
  Serial.write(0x07); // Start
  Serial.write(0x01); // Lenght
  Serial.write(0x02); // Type
  Serial.write(0x02); // Function
  Serial.write(0x04); // Addr
  Serial.write(0x01); // Addr
  Serial.write((isWaterThreshold())? 0x01 : 0x00);
  uint16_t _sum = 10 + ((isWaterThreshold())? 0x01 : 0x00);
  Serial.write((uint8_t)(_sum >> 8));
  Serial.write((uint8_t)_sum);
}

void dataTempSend() {
  Serial.write(0x27); // Start
  Serial.write(0x07); // Start
  Serial.write(0x02); // Lenght
  Serial.write(0x02); // Type
  Serial.write(0x02); // Function
  Serial.write(0x02); // Addr
  Serial.write(0x01); // Addr
  uint16_t _sum = 9;
  uint16_t _temp = (uint16_t)(readTemperatureSensor()*100);
  _sum += _temp>>8;
  _sum += _temp & 0x00FF;
  Serial.write((uint8_t)(_temp >> 8));
  Serial.write((uint8_t)(_temp & 0x00FF));
  Serial.write((uint8_t)(_sum >> 8));
  Serial.write((uint8_t)(_sum & 0x00FF));
}

bool isWaterThreshold() {
  return (digitalRead(WATER_SENSOR) == HIGH)? true : false;
}

float readTemperatureSensor() {
  sensors.requestTemperatures();
  delay(50);
  return sensors.getTempCByIndex(0);
}

