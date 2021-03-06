#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "app_timer.h"
#include "nrf_delay.h"
#include "nrf_drv_clock.h"
#include "nrf_gpio.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "nrf_power.h"
#include "nrf_serial.h"
#include "nrf_uarte.h"

#include "buckler.h"
#include "display.h"
#include "kobukiActuator.h"
#include "kobukiSensorPoll.h"
#include "kobukiSensorTypes.h"
#include "kobukiUtilities.h"
#include "mpu9250.h"

// Config
#define BUCKLER_UART_RX 15
#define BUCKLER_UART_TX 16
#define SERIAL_FIFO_TX_SIZE 512
#define SERIAL_FIFO_RX_SIZE 512
#define SERIAL_BUFF_TX_SIZE 128
#define SERIAL_BUFF_RX_SIZE 128

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

float data;
uint8_t *data_array = (uint8_t *)&data;
uint32_t r_error = 0;

// Error Handler for UART
static void ser_event_handler(nrf_serial_t const *p_serial, nrf_serial_event_t event) {
  switch (event) {
  case NRF_SERIAL_EVENT_TX_DONE: {
    break;
  }
  case NRF_SERIAL_EVENT_RX_DATA: {
    data = 0;
    size_t read;
    nrf_serial_read(&serial_uart, &data_array, sizeof(data_array), &read, 0);
    printf("Reading %f\n", data);
    break;
  }
  case NRF_SERIAL_EVENT_DRV_ERR: {
    nrf_serial_rx_drain(&serial_uart);
    nrf_serial_uninit(&serial_uart);
    nrf_serial_init(&serial_uart, &uart_config, &serial_config);
    break;
  }
  case NRF_SERIAL_EVENT_FIFO_ERR: {
    break;
  }
  }
}

int main(void) {
  ret_code_t error_code = NRF_SUCCESS;

  printf("Initializing\n");
  nrf_serial_init(&serial_uart, &uart_config, &serial_config);

  // initialize kobuki
  kobukiInit();
  printf("Kobuki initialized!\n");

  while (1) {
    nrf_delay_ms(1);
  }
}
