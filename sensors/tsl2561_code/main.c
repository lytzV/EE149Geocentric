// Robot Template app
//
// Framework for creating applications that control the Kobuki robot

#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "app_error.h"
#include "app_timer.h"
#include "nrf.h"
#include "nrf_delay.h"
#include "nrf_drv_spi.h"
#include "nrf_gpio.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "nrf_pwr_mgmt.h"

#include "buckler.h"
#include "display.h"
#include "kobukiActuator.h"
#include "kobukiSensorPoll.h"
#include "kobukiSensorTypes.h"
#include "kobukiUtilities.h"
#include "tsl2561.h"

// I2C manager
NRF_TWI_MNGR_DEF(twi_mngr_instance, 5, 0);

typedef enum {
  OFF,
  INIT,
  DRIVING,
} robot_state_t;

int mean(int len, int buf[]) {
  int sum = 0;
  for (int i = 0; i < len; i++) {
    sum += buf[i] / len;
  }
  return sum;
}

int main(void) {
  ret_code_t error_code = NRF_SUCCESS;

  // initialize RTT library
  error_code = NRF_LOG_INIT(NULL);
  APP_ERROR_CHECK(error_code);
  NRF_LOG_DEFAULT_BACKENDS_INIT();
  printf("Log initialized!\n");

  // initialize LEDs
  nrf_gpio_pin_dir_set(23, NRF_GPIO_PIN_DIR_OUTPUT);
  nrf_gpio_pin_dir_set(24, NRF_GPIO_PIN_DIR_OUTPUT);
  nrf_gpio_pin_dir_set(25, NRF_GPIO_PIN_DIR_OUTPUT);

  // initialize display
  nrf_drv_spi_t spi_instance = NRF_DRV_SPI_INSTANCE(1);
  nrf_drv_spi_config_t spi_config = {
      .sck_pin = BUCKLER_LCD_SCLK,
      .mosi_pin = BUCKLER_LCD_MOSI,
      .miso_pin = BUCKLER_LCD_MISO,
      .ss_pin = BUCKLER_LCD_CS,
      .irq_priority = NRFX_SPI_DEFAULT_CONFIG_IRQ_PRIORITY,
      .orc = 0,
      .frequency = NRF_DRV_SPI_FREQ_4M,
      .mode = NRF_DRV_SPI_MODE_2,
      .bit_order = NRF_DRV_SPI_BIT_ORDER_MSB_FIRST};
  error_code = nrf_drv_spi_init(&spi_instance, &spi_config, NULL, NULL);
  APP_ERROR_CHECK(error_code);
  display_init(&spi_instance);
  display_write("Hello, Human!", DISPLAY_LINE_0);
  printf("Display initialized!\n");

  // initialize i2c master (two wire interface)
  nrf_drv_twi_config_t i2c_config = NRF_DRV_TWI_DEFAULT_CONFIG;
  i2c_config.scl = BUCKLER_SENSORS_SCL;
  i2c_config.sda = BUCKLER_SENSORS_SDA;
  i2c_config.frequency = NRF_TWIM_FREQ_100K;
  error_code = nrf_twi_mngr_init(&twi_mngr_instance, &i2c_config);
  APP_ERROR_CHECK(error_code);
  tsl2561_init(&twi_mngr_instance);
  error_code = tsl2561_config();
  APP_ERROR_CHECK(error_code);
  printf("IMU initialized!\n");

  // initialize Kobuki
  kobukiInit();
  printf("Kobuki initialized!\n");

  // configure initial state
  robot_state_t state = OFF;
  KobukiSensors_t sensors = {0};
  float angle = 90;
  char buf[16];
  static uint8_t lux_buf = 3;
  uint16_t lux_left[lux_buf];
  uint16_t lux_right[lux_buf];
  uint8_t lux_count = 0;
  uint8_t init_time = 0;
  uint32_t left_avg = 500;
  uint32_t right_avg = 500;
  uint32_t diff_max = 500;

  // loop forever, running state machine
  while (1) {
    // read sensors from robot
    kobukiSensorPoll(&sensors);
    // lux_left[lux_count] = tsl2561_read_result(TSL2561_ADDR_FLOAT);
    // lux_right[lux_count] = tsl2561_read_result(TSL2561_ADDR_HIGH);
    left_avg = tsl2561_read_result(TSL2561_ADDR_FLOAT);
    right_avg = tsl2561_read_result(TSL2561_ADDR_HIGH);
    lux_count = (lux_count + 1) / lux_buf;

    // delay before continuing
    // Note: removing this delay will make responses quicker, but will result
    //  in printf's in this loop breaking JTAG
    nrf_delay_ms(1);

    // handle states
    switch (state) {
    case OFF: {
      // transition logic
      if (is_button_pressed(&sensors)) {
        state = DRIVING;
        // kobukiDriveDirect(60 + lux_high / 300, 60 + lux_float / 300);
      } else {
        // left_avg = mean(lux_buf, lux_left);
        // right_avg = mean(lux_buf, lux_right);
        // perform state-specific actions here
        snprintf(buf, 16, "L=%d, R=%d", left_avg, right_avg);
        printf("%s\n", buf);
        display_write(buf, DISPLAY_LINE_0);
        angle = (left_avg - right_avg) / 12.5;
        snprintf(buf, 16, "Angle=%f\n", angle);
        display_write(buf, DISPLAY_LINE_1);
        printf(buf);

        state = OFF;
      }
      break; // each case needs to end with break!
    }

    case INIT: {
      if (init_time < lux_buf) {
        // wait for lux to populate
        snprintf(buf, 16, "Initiating...%d", init_time);
        display_write(buf, DISPLAY_LINE_0);
        init_time++;
      } else {
        left_avg = mean(lux_buf, lux_left);
        right_avg = mean(lux_buf, lux_right);
        state = OFF;
      }

      break;
    }

    case DRIVING: {
      // transition logic
      if (is_button_pressed(&sensors)) {
        state = OFF;
        kobukiDriveDirect(0, 0);
      } else {
        // perform state-specific actions here
        left_avg = mean(lux_buf, lux_left);
        right_avg = mean(lux_buf, lux_right);
        // kobukiDriveDirect(60 + lux_high / 300, 60 + lux_float / 300);
        snprintf(buf, 16, "L=%d, R=%d", left_avg, right_avg);
        printf("%s\n", buf);
        display_write(buf, DISPLAY_LINE_0);
        angle = 90 - (left_avg - right_avg) / 500 * 90;
        snprintf(buf, 16, "Angle=%f", angle);
        display_write(buf, DISPLAY_LINE_1);

        state = DRIVING;
      }
      break; // each case needs to end with break!
    }

      // add other cases here
    }
  }
}
