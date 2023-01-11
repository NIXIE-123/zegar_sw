/*!
*	ZEGAR NA ESP32 Z WYSWIETLACZEM LED SPI
*	Autorzy:
*	Beniamin Parczewski
*	Aleksander Ostrowski
*/
#ifndef COMPONENTS_SPI_SPI_H_
#define COMPONENTS_SPI_SPI_H_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"

#define PIN_NUM_MOSI 12
#define PIN_NUM_CLK 14
#define PIN_NUM_CS 15
#define SPI_TRANSFER_SIZE 48
/*!
*	USTAWIENIA KONTROLERA LED MC14489
*	2 Kontrolery zostały połączone szeregowo
*/
#define LED_LSB_PARAM_CONFIG 49 /*!<	Konfiguracja 2-giego układu	*/
#define LED_MSB_PARAM_CONFIG 1	/*!<	Konfiguracja 1-szego układu	*/
#define DOT_PARAM_CONFIG 0x41	/*!<	Ustawienia wyświetlania kropek młodsze 4 bity - 2-gi układ, starsze 4 bity - 1-szy układ	*/

spi_device_handle_t spi2;


void spi_init();
void write_config_LED_disp(uint8_t config_reg_1,uint8_t config_reg_0);
void write_data_LED_disp(uint8_t config_dots, uint8_t *data);

#endif /* COMPONENTS_SPI_SPI_H_ */
