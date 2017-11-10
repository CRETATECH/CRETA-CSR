
#ifndef __SENSOR_READER__
#define __SENSOR_READER__

#define ONE_WIRE_BUS 2
#define WATER_SENSOR 3 // Pin 3
#define DEVICE_1 4
#define DEVICE_2 5
#define BUTTON_1 6
#define BUTTON_2 7

typedef struct frame {
  uint8_t framelength;
  uint8_t frametype;
  uint8_t framefunc;
  uint16_t frameaddr;
  uint8_t* framedata;
  uint16_t framechecksum;
} frame_t;

#endif
