#ifndef MQTT_H
#define MQTT_H

#include "mqtt_client.h"

#define MAX_EVENTS 31

typedef struct{
    int color_id;
}event_t;

//Each position in the array represents the day of the month 
event_t events[MAX_EVENTS];

__uint8_t first_of_month;
__uint8_t current_date;

void mqtt_app_start(esp_mqtt_client_handle_t client);
void update_events(char *message);
event_t *read_events();
#endif /*MQTT_H*/