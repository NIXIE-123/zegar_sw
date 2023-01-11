/*!
*	ZEGAR NA ESP32 Z WYSWIETLACZEM LED SPI
*	Autorzy:
*	Beniamin Parczewski
*	Aleksander Ostrowski
*/
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include <string.h>
#include "time.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "freertos/event_groups.h"
#include "nvs_flash.h"
#include "esp_sntp.h"
#include "spi.h"

#define WIFI_SSID      "Redmi Note 9 Pro"
#define WIFI_PASS      "olek1234"

EventGroupHandle_t s_wifi_event_group;
esp_event_handler_instance_t instance_got_ip, instance_any_id;
xQueueHandle queue1;

static int s_retry_num = 0;

void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data){
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < 2) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI("wifi station", "retry to connect to the AP");
        } else {
            xEventGroupSetBits(s_wifi_event_group, BIT1);
        }
        ESP_LOGI("wifi station", "connect to the AP fail");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI("wifi station", "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, BIT0);
    }
}
static void task_spi(){
	while(1){
		time_t now;
		xQueueReceive(queue1, &now, portMAX_DELAY);
		struct tm timeinfo;
		localtime_r(&now, &timeinfo);
		uint8_t led_data[8]={timeinfo.tm_hour/10,timeinfo.tm_hour%10,timeinfo.tm_min/10,timeinfo.tm_min%10,timeinfo.tm_sec/10,timeinfo.tm_sec%10,0,0};//6 i 7 parametr tablicy ma wynosić 0, resztę można zapisywać daymi od 0 do 9
		write_data_LED_disp(DOT_PARAM_CONFIG,led_data);
	}
}
static void task_sntp(){
	while(1){
		s_wifi_event_group = xEventGroupCreate();
	   	ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL, &instance_any_id));
	    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL, &instance_got_ip));
	    ESP_ERROR_CHECK(esp_wifi_start() );
	    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group, BIT0 | BIT1, pdFALSE, pdFALSE, portMAX_DELAY);
	    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, instance_got_ip));
	    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, instance_any_id));
	    vEventGroupDelete(s_wifi_event_group);

	    if(bits & BIT0){
	    	sntp_init();
	        int retry = 0;
	        const int retry_count = 11;
	        while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < retry_count) {
	        	ESP_LOGI("sntp_sync", "Waiting for system time to be set... (%d/%d)", retry, retry_count);
	           	vTaskDelay(2000 / portTICK_PERIOD_MS);
	        }
	        sntp_stop();
	    }
	    ESP_ERROR_CHECK(esp_wifi_stop() );

		vTaskDelay(300000/portTICK_PERIOD_MS);
	}
}
void inicjalizacja_wifi_sntp(){
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    wifi_config_t wifi_config = {
        	           	 .sta = {
        	           	 .ssid = WIFI_SSID,
        	           	 .password = WIFI_PASS,
        	           	 .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        	           	    .pmf_cfg = {
        	           	       .capable = true,
        	                   .required = false
        	               	},
        	             },
    			  };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org");
    sntp_set_sync_mode(SNTP_SYNC_MODE_IMMED);
    setenv("TZ", "CET-1CEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00", 1);
    tzset();
}
void app_main(void)
{
	esp_err_t err = nvs_flash_init();
	if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
		ESP_ERROR_CHECK(nvs_flash_erase());
		err = nvs_flash_init();
	}
	ESP_ERROR_CHECK(err);
    spi_init();
    write_config_LED_disp(LED_MSB_PARAM_CONFIG,LED_LSB_PARAM_CONFIG);

    queue1 = xQueueCreate(1,sizeof(time_t));
    xTaskCreate(task_spi, "task_spi", 1024, NULL, 3, NULL);
    inicjalizacja_wifi_sntp();
    xTaskCreate(task_sntp, "task_sntp", 2048, NULL, 8, NULL);


	while(1){
		time_t now;
		time(&now);
	    xQueueSend(queue1,&now,( TickType_t ) 0 );
		vTaskDelay(100/portTICK_PERIOD_MS);
    }
}
