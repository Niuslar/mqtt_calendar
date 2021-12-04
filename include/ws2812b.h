/* ws2812b.h 
   Author: Niuslar
   Date: 02/12/2021 */

#ifndef WS2812B_H
#define WS2812B_H

#include "driver/rmt.h"

#define RMT_TX_CHANNEL RMT_CHANNEL_0

#define RMT_GPIO_NUM 12

#define STRIP_LENGTH 35 

/* APIs */
void rgb_init();
void clear_strip();
void send_to_pixel(__uint8_t green, __uint8_t red, __uint8_t blue, __uint8_t led_number); //Sends RGB to individual LED

#endif /*WS2812B_H*/
