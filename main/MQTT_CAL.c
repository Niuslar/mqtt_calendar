#include <stdio.h>
#include "../include/wifi.h"
#include "../include/mqtt.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "mqtt_client.h"
#include "ws2812b.h"

typedef struct{
    __uint8_t green;
    __uint8_t red;
    __uint8_t blue;
}RGB_t;

// Google calendars use colour ids. This array stores an APPROXIMATE RGB value for each colour ID
// Visit https://codepen.io/chiss22/pen/qBdEqdj to see the colours (NOT MY WEBSITE)

RGB_t rgb_google[] = {                     // Colour ID from google
    {.green = 55, .red = 221, .blue = 32}, //1
    {.green = 50, .red = 251, .blue = 50}, //2
    {.green = 0, .red = 250, .blue = 0},   //3
    {.green = 10, .red = 200, .blue = 0},  //4
    {.green = 20, .red = 200, .blue = 0},  //5
    {.green = 35, .red = 200, .blue = 0},  //6
    {.green = 200, .red = 0, .blue = 30},  //7
    {.green = 150, .red = 5, .blue = 20},  //8
    {.green = 150, .red = 60, .blue = 20}, //9
    {.green = 100, .red = 110, .blue = 10}, //10 (Avoid using this and the next two, they are very similar)
    {.green = 150, .red = 120, .blue = 10}, //11
    {.green = 70, .red = 250, .blue = 4},   //12
    {.green = 220, .red = 5, .blue = 40},   //13
    {.green = 60, .red = 1, .blue = 229},   //14
    {.green = 30, .red = 10, .blue = 229},  //15
    {.green = 10, .red = 30, .blue = 180},  //16
    {.green = 55, .red = 50, .blue = 200},  //17
    {.green = 70, .red = 70, .blue = 200},  //18
    {.green = 70, .red = 70, .blue = 70},  //19
    {.green = 85, .red = 75, .blue = 65},  //20
    {.green = 5, .red = 255, .blue = 85},  //21
    {.green = 15, .red = 250, .blue = 75},  //22
    {.green = 3, .red = 100, .blue = 150},  //23
    {.green = 100, .red = 140, .blue = 140},  //24
    {.green = 0, .red = 0, .blue = 0}  // 25 (Not a google colour ID, but used to turn the LEDs off)
};

RGB_t rgb_event;

//Function linked to the task
void send_rgb(){
    while(1){
        for(int i = 0; i < MAX_EVENTS; i++)
        {
            if(i+1 < current_date){
                // 100, 100 and 100 is to make a light colour to mark the days past
                send_to_pixel(100,100,100,i+1+first_of_month);
            }
            else{
                // NOTE: i represents the day of the month
                rgb_event.red = rgb_google[events[i].color_id-1].red;
                rgb_event.green = rgb_google[events[i].color_id-1].green;
                rgb_event.blue = rgb_google[events[i].color_id-1].blue;
                send_to_pixel(rgb_event.green, rgb_event.red, rgb_event.blue, i+1+first_of_month);
            }
        }
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }

}

void app_main(void)
{

    static const char *TAG = "MQTT_CAL";

    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
   
    //Start RGB 
    rgb_init();
    clear_strip();

    //Starting sequence to test green, red and blue on each LED
    for(int i = 1; i <= STRIP_LENGTH; i++){
        send_to_pixel(255,0,0,i);
        vTaskDelay(30/portTICK_PERIOD_MS);
    }

    for(int i = STRIP_LENGTH; i>0; i--){
        send_to_pixel(0,255,0,i);
        vTaskDelay(30/portTICK_PERIOD_MS);
    }

    for(int i = 1; i <= STRIP_LENGTH; i++){
        send_to_pixel(0,0,255,i);
        vTaskDelay(30/portTICK_PERIOD_MS);
    }

    for(int i = 1; i <= STRIP_LENGTH; i++){
        send_to_pixel(0,0,0,i);
        vTaskDelay(30/portTICK_PERIOD_MS);
    }   

    //Start WiFi
    ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
    wifi_init_sta();

   
    //MQTT
    esp_mqtt_client_config_t mqtt_cfg = {
        .uri = "mqtt://username:password@host:port",
    };

    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);

    mqtt_app_start(client);
    vTaskDelay(2000 / portTICK_PERIOD_MS);
    esp_mqtt_client_subscribe(client, "/events", 0);


    //Create task to read events and send data to LEDS on Core 0 
    //(on menuconfig change WiFi and MQTT to work on core 1)
    xTaskCreatePinnedToCore(
            send_rgb, // Function to be called
            "Send data to LED", //Name of task
            1024*32, // Stack size
            NULL,
            25, //Task priority (Higher number = higher prio) 1-25
            NULL,
            0); //Core to use

    

    vTaskStartScheduler();

    return;

}