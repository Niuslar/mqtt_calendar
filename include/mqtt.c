#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_system.h"
#include "esp_event.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "esp_log.h"
#include "mqtt.h"



static const char *TAG = "MQTT_MODULE";

//Each position in the array represents the day of the month 
event_t events[MAX_EVENTS];

static esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event)
{

    switch (event->event_id) {
        case MQTT_EVENT_BEFORE_CONNECT:
            ESP_LOGI(TAG, "The client is initialized and about to start connecting to the broker.");
            break;
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "The client has successfully established a connection to the broker.");
            break;
        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, "The broker has acknowledged the client’s subscribe request.");
            break;     
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "The broker has acknowledged the client’s publish message.");
            break;               
        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "MQTT_EVENT_DATA");
            //Send the event data to process day and color id
            char message_buf[10];
            sprintf(message_buf, "%.*s", event->data_len, event->data);
            update_events(message_buf);
            break;
        default:
            ESP_LOGI(TAG, "Other event id:%d", event->event_id);
            break;
    }
    return ESP_OK;
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    mqtt_event_handler_cb(event_data);
}

void mqtt_app_start(esp_mqtt_client_handle_t client)
{
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client);
    esp_mqtt_client_start(client);
}

void mqtt_subscribe(esp_mqtt_client_handle_t client, const char *topic, int qos)
{
    esp_mqtt_client_subscribe(client, topic, qos);
}

void update_events(char *message)
{
    //printf("%s\n",message);
    //Use tokens to split the values
    char *token;
    token = strtok(message, ",");
    //First get the date and store it as an int 
    int date = atoi(token);
    //Then get the colour and do the same
    token = strtok(NULL,",");
    int colour = atoi(token);
    //Now store the colour on the date on an array 
    if(colour != 0)
    events[date-1].color_id = colour;
}   

event_t *read_events()
{
    return events;
}

