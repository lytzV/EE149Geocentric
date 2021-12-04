#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "nrf_uarte.h"
#include "nrf_power.h"
#include "nrf_drv_clock.h"
#include "nrf_serial.h"
#include "app_timer.h"

// Config
#define BUCKLER_UART_RX 17
#define BUCKLER_UART_TX 16
#define SERIAL_FIFO_TX_SIZE 512
#define SERIAL_FIFO_RX_SIZE 512
#define SERIAL_BUFF_TX_SIZE 128
#define SERIAL_BUFF_RX_SIZE 128
#define uart_config "uart_config"
#define serial_queue "serial_queue"
#define serial_buff "serial_buff"
#define serial_config "serial_config"
#define serial_uart "serial_uart"

// Config Definition
NRF_SERIAL_DRV_UART_CONFIG_DEF(uart_config,
                               BUCKLER_UART_RX, BUCKLER_UART_TX,
                               0, 0,
                               NRF_UART_HWFC_DISABLED, NRF_UART_PARITY_EXCLUDED,
                               NRF_UART_BAUDRATE_115200,
                               UART_DEFAULT_CONFIG_IRQ_PRIORITY);
NRF_SERIAL_QUEUES_DEF(serial_queue, SERIAL_FIFO_TX_SIZE, SERIAL_FIFO_RX_SIZE);
NRF_SERIAL_BUFFERS_DEF(serial_buff, SERIAL_BUFF_TX_SIZE, SERIAL_BUFF_RX_SIZE);
static void ser_event_handler(nrf_serial_t const *p_serial, nrf_serial_event_t event);
NRF_SERIAL_CONFIG_DEF(serial_config, NRF_SERIAL_MODE_DMA,
                      &serial_queue, &serial_buff, ser_event_handler, NULL);

NRF_SERIAL_UART_DEF(serial_uart, 0);

// Error Handler for UART
static void ser_event_handler(nrf_serial_t const *p_serial, nrf_serial_event_t event)
{
    switch (event)
    {
        case NRF_SERIAL_EVENT_TX_DONE:
        {
            break;
        }
        case NRF_SERIAL_EVENT_RX_DATA:
        {
            size_t read;
            uint8_t buffer[16];
            nrf_serial_read(&serial_uart, &buffer, sizeof(buffer), &read, 0);
            ser_rx_data(buffer, read);
            break;
        }
        case NRF_SERIAL_EVENT_DRV_ERR:
        {
            nrf_serial_rx_drain(&serial_uart);
            nrf_serial_uninit(&serial_uart);
            nrf_serial_init(&serial_uart, &uart_config, &serial_config);
            break;
        }
        case NRF_SERIAL_EVENT_FIFO_ERR:
        {
            break;
        }
    }
}

void ser_rx_data(uint8_t *data, size_t size) {
    // Do something useful with recieved data
    printf("%.*s", size, data);
}

int main(void) {
    ret_code_t error_code = NRF_SUCCESS;

    printf("Initializing\n");
    nrf_serial_init(&serial_uart, &uart_config, &serial_config);

    while (1) {
        nrf_delay_ms(1);
    }
}