#pragma once
#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "mqtt_client.h"
void mqtt_app_start();
void mqtt_app_destroy();

void mqtt_app_publish(char *topic, char *msg);