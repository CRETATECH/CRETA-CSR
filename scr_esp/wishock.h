#ifndef WISHOCK_H
#define WISHOCK_H

/*************************************************/
/*                  DEFINE SYMBOL                */
/*************************************************/
#define PIN_BUTTON_CONFIG             0
#define PIN_LED_WIFI                  4
#define EEPROM_ADDRESS                0x10
#define DEBUG            

typedef enum {
  STATE_CONFIG  = 0,
  STATE_CONTROL = ~STATE_CONFIG,
}fsm_t;

typedef enum {
  LED_STATUS_BLINK = 0,
  LED_STATUS_ON ,
  LED_STATUS_OFF,
}led_status_t;



#endif
