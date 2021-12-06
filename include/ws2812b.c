#include <stdio.h>
#include "ws2812b.h"
#include "driver/rmt.h"

//Create a struct to store each LED status 
typedef struct{
    __uint8_t green;
    __uint8_t red;
    __uint8_t blue;
}LED_status_t;

//Create array to store the state of each LED
LED_status_t LEDS[STRIP_LENGTH];

// Create array to store the "pulses" for the whole strip (each LED needs 24 bits)
rmt_item32_t buffer[STRIP_LENGTH][24];

//Note: with a freq. of 80Mhz, each tick is 0.0125us. 
//The pattern we need to send a 1 is 0.4us High followed by 0.85 Low
//To send a 0 is 0.85us High ans 0.45 Low

//Convertion: Ticks to us
// 32T = 0.4us
// 36T = 0.45us
// 64T = 0.8us
// 68T = 0.85us

rmt_item32_t commands[] = {
    //LOW (0) = 32 ticks high, 68 low
    {{{32,1,68,0}}},
    //HIGH (1) = 64T High and 36 Low
    {{{64,1,36,0}}},
    //Reset = 50 us low (4000 ticks low)
    {{{2000,0,2000,0}}}
};

rmt_config_t rmt_config_param = {
    .channel = RMT_TX_CHANNEL, 
    .rmt_mode = RMT_MODE_TX,
    .gpio_num = RMT_GPIO_NUM,
    .clk_div = 1,
    .mem_block_num = 2
};

void rgb_init(){
    rmt_config_param.tx_config.carrier_en = false;
    rmt_config_param.tx_config.loop_en = 0;
    rmt_config_param.tx_config.idle_output_en = true;
    rmt_config_param.tx_config.idle_level = RMT_IDLE_LEVEL_LOW; 

    rmt_config(&rmt_config_param);
    rmt_driver_install(RMT_TX_CHANNEL,0,0);

}

//Take the LEDS array and fill the buffer 
static void rgb_to_pulse()
{
    // Loop through the LEDs array 
    for(int i = 0; i < STRIP_LENGTH; i++){
        //Store the colours in one variable
        __uint32_t rgb = ((LEDS[i].green << 16) | (LEDS[i].red << 8) | LEDS[i].blue);
        //Loop through the bits and store in buffer 
        for(int j = 0 ; j < 24; j++)
        {
            buffer[i][j] = commands[(rgb>>(23-j))&1];
        }
    }    
}

//Send color to single LED (led_number starts from 1)
void send_to_pixel(__uint8_t green, __uint8_t red, __uint8_t blue, __uint8_t led_number){
    
    //Update the LEDS array with the new values
    LEDS[led_number-1].blue = blue;
    LEDS[led_number-1].green = green;
    LEDS[led_number-1].red = red;

    //Update buffer
    rgb_to_pulse();

    //Send buffer to strip
    for(int i = 0; i < STRIP_LENGTH; i++){
        rmt_write_items(RMT_TX_CHANNEL, buffer[i], 24, 0);
    }

    //send reset
    rmt_write_items(RMT_TX_CHANNEL, &commands[2], 1, 0);
}

void clear_strip(){
    for(int i = 0; i < STRIP_LENGTH*24; i++){
        rmt_write_items(RMT_TX_CHANNEL, &commands[0],1,0);
    }
    rmt_write_items(RMT_TX_CHANNEL, &commands[2], 1, 0);
}
