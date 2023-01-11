/*!
*	ZEGAR NA ESP32 Z WYSWIETLACZEM LED SPI
*	Autorzy:
*	Beniamin Parczewski
*	Aleksander Ostrowski
*/
#include "spi.h"

/*!
*	KONFIGURACJA SPI
*/
void spi_init() {
    esp_err_t ret;

    spi_bus_config_t buscfg={
        .miso_io_num = -1,
        .mosi_io_num = PIN_NUM_MOSI,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = SPI_TRANSFER_SIZE,
    };

    ret = spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO);
    ESP_ERROR_CHECK(ret);

    spi_device_interface_config_t devcfg={
        .clock_speed_hz = 2000000,  // 2 MHz
        .mode = 0,                  //SPI mode 0
        .spics_io_num = PIN_NUM_CS,
        .queue_size = 1,
        .flags = SPI_DEVICE_HALFDUPLEX,
        .pre_cb = NULL,
        .post_cb = NULL,
    };

    ESP_ERROR_CHECK(spi_bus_add_device(SPI2_HOST, &devcfg, &spi2));
};
/*!
*	Fukcja zapisu danych konfiguracyjnych do kontrolerów MC14489
*/
void write_config_LED_disp(uint8_t config_reg_1,/*!<	argument konfiguryjący 2-gi układ MC14489 LED_LSB_PARAM_CONFIG	*/
		uint8_t config_reg_0/*!<	argument konfiguryjący 1-szy układ MC14489 LED_MSB_PARAM_CONFIG	*/
		) {
    uint8_t tx_data[4] = { config_reg_0, 0, 0, config_reg_1 };

    spi_transaction_t t = {
        .tx_buffer = tx_data,
        .length = 4 * 8
    };

    ESP_ERROR_CHECK(spi_device_polling_transmit(spi2, &t));
}
/*!
*	Fukcja zapisu danych do kontrolerów MC14489
*/
void write_data_LED_disp(uint8_t config_dots/*!<	argument konfigurujacy wyswietlanie kropek DOT_PARAM_CONFIG	*/
		, uint8_t *data/*!<	dane	*/
		) {
    uint8_t tx_data[6];

    tx_data[0]=(config_dots<<4)|*(data+7);
    tx_data[1]=(*(data+6)<<4)|*(data+5);
    tx_data[2]=(*(data+4)<<4)|*(data+3);
    tx_data[3]=(config_dots & 0xF0)|*(data+2);
    tx_data[4]=(*(data+1)<<4)|*data;
    tx_data[5]=0;

	spi_transaction_t t = {
        .tx_buffer = tx_data,
        .length = 6 * 8
    };

    ESP_ERROR_CHECK(spi_device_polling_transmit(spi2, &t));
}
