#ifndef MQTT_H
#define MQTT_H

#include "mqtt_client.h"

#define MAX_EVENTS 31

typedef struct{
    int color_id;
}event_t;

void mqtt_app_start(esp_mqtt_client_handle_t client);
void update_events(char *message);
event_t *read_events();
#endif /*MQTT_H*/