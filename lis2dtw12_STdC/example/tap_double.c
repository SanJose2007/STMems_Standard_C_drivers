/*
 ******************************************************************************
 * @file    double_tap.c
 * @author  Sensors Software Solution Team
 * @brief   This file show the simplest way to detect double tap from sensor.
 *
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT(c) 2018 STMicroelectronics</center></h2>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *   1. Redistributions of source code must retain the above copyright notice,
 *      this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *   3. Neither the name of STMicroelectronics nor the names of its
 *      contributors may be used to endorse or promote products derived from
 *      this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

/*
 * This example was developed using the following STMicroelectronics
 * evaluation boards:
 *
 * - STEVAL_MKI109V3
 * - NUCLEO_F411RE + X_NUCLEO_IKS01A2
 *
 * and STM32CubeMX tool with STM32CubeF4 MCU Package
 *
 * Used interfaces:
 *
 * STEVAL_MKI109V3    - Host side:   USB (Virtual COM)
 *                    - Sensor side: SPI(Default) / I2C(supported)
 *
 * NUCLEO_STM32F411RE + X_NUCLEO_IKS01A2 - Host side: UART(COM) to USB bridge
 *                                       - I2C(Default) / SPI(N/A)
 *
 * If you need to run this example on a different hardware platform a
 * modification of the functions: `platform_write`, `platform_read`,
 * `tx_com` and 'platform_init' is required.
 *
 */

/* STMicroelectronics evaluation boards definition
 *
 * Please uncomment ONLY the evaluation boards in use.
 * If a different hardware is used please comment all
 * following target board and redefine yours.
 */
//#define STEVAL_MKI109V3
#define NUCLEO_F411RE_X_NUCLEO_IKS01A2

#if defined(STEVAL_MKI109V3)
/* MKI109V3: Define communication interface */
#define SENSOR_BUS hspi2

/* MKI109V3: Vdd and Vddio power supply values */
#define PWM_3V3 915

#elif defined(NUCLEO_F411RE_X_NUCLEO_IKS01A2)
/* NUCLEO_F411RE_X_NUCLEO_IKS01A2: Define communication interface */
#define SENSOR_BUS hi2c1

#endif

/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include <stdio.h>
#include "stm32f4xx_hal.h"
#include "lis2dtw12_reg.h"
#include "gpio.h"
#include "i2c.h"
#if defined(STEVAL_MKI109V3)
#include "usbd_cdc_if.h"
#include "spi.h"
#elif defined(NUCLEO_F411RE_X_NUCLEO_IKS01A2)
#include "usart.h"
#endif

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
static uint8_t whoamI, rst;
static uint8_t tx_buffer[1000];

/* Extern variables ----------------------------------------------------------*/

/* Private functions ---------------------------------------------------------*/
/*
 *   WARNING:
 *   Functions declare in this section are defined at the end of this file
 *   and are strictly related to the hardware platform used.
 *
 */
static int32_t platform_write(void *handle, uint8_t reg, uint8_t *bufp,
                              uint16_t len);
static int32_t platform_read(void *handle, uint8_t reg, uint8_t *bufp,
                             uint16_t len);
static void tx_com( uint8_t *tx_buffer, uint16_t len );
static void platform_init(void);

/* Main Example --------------------------------------------------------------*/
void example_main_double_tap_lis2dtw12(void)
{
  /*
   * Initialize mems driver interface
   */
  lis2dtw12_ctx_t dev_ctx;
  lis2dtw12_reg_t int_route;

  dev_ctx.write_reg = platform_write;
  dev_ctx.read_reg = platform_read;
  dev_ctx.handle = &hi2c1;

  /*
   * Initialize platform specific hardware
   */
  platform_init();

  /*
   * Check device ID
   */
  lis2dtw12_device_id_get(&dev_ctx, &whoamI);
  if (whoamI != LIS2DTW12_ID)
    while(1)
    {
      /* manage here device not found */
    }

  /*
   * Restore default configuration
   */
  lis2dtw12_reset_set(&dev_ctx, PROPERTY_ENABLE);
  do {
    lis2dtw12_reset_get(&dev_ctx, &rst);
  } while (rst);

  /*
   * Set full scale
   */
  lis2dtw12_full_scale_set(&dev_ctx, LIS2DTW12_2g);

  /*
   * Configure power mode
   */
  lis2dtw12_power_mode_set(&dev_ctx, LIS2DTW12_CONT_LOW_PWR_LOW_NOISE_12bit);

  /*
   * Set Output Data Rate
   */
  lis2dtw12_data_rate_set(&dev_ctx, LIS2DTW12_XL_ODR_400Hz);

  /*
   * Enable Tap detection on X, Y, Z
   */
  lis2dtw12_tap_detection_on_z_set(&dev_ctx, PROPERTY_ENABLE);
  lis2dtw12_tap_detection_on_y_set(&dev_ctx, PROPERTY_ENABLE);
  lis2dtw12_tap_detection_on_x_set(&dev_ctx, PROPERTY_ENABLE);

  /*
   * Set Tap threshold on all axis
   */
  lis2dtw12_tap_threshold_x_set(&dev_ctx, 12);
  lis2dtw12_tap_threshold_y_set(&dev_ctx, 12);
  lis2dtw12_tap_threshold_z_set(&dev_ctx, 12);

  /*
   * Configure Double Tap parameter
   */
  lis2dtw12_tap_dur_set(&dev_ctx, 7);
  lis2dtw12_tap_quiet_set(&dev_ctx, 3);
  lis2dtw12_tap_shock_set(&dev_ctx, 3);

  /*
   * Enable Double Tap detection
   */
  lis2dtw12_tap_mode_set(&dev_ctx, LIS2DTW12_BOTH_SINGLE_DOUBLE);

  /*
   * Enable single tap detection interrupt
   */
  lis2dtw12_pin_int1_route_get(&dev_ctx, &int_route.ctrl4_int1_pad_ctrl);
  int_route.ctrl4_int1_pad_ctrl.int1_tap = PROPERTY_ENABLE;
  lis2dtw12_pin_int1_route_set(&dev_ctx, &int_route.ctrl4_int1_pad_ctrl);

  /*
   * Wait Events.
   */
  while(1)
  {
    lis2dtw12_all_sources_t all_source;

    /*
     * Check Double Tap events
     */
    lis2dtw12_all_sources_get(&dev_ctx, &all_source);
    if (all_source.tap_src.double_tap)
    {
      sprintf((char*)tx_buffer, "Double Tap Detected: Sign %s",
              all_source.tap_src.tap_sign ? "positive" : "negative");
      if (all_source.tap_src.x_tap)
        sprintf((char*)tx_buffer, "%s on X", tx_buffer);
      if (all_source.tap_src.y_tap)
        sprintf((char*)tx_buffer, "%s on Y", tx_buffer);
      if (all_source.tap_src.z_tap)
        sprintf((char*)tx_buffer, "%s on Z", tx_buffer);
      sprintf((char*)tx_buffer, "%s axis\r\n", tx_buffer);
      tx_com(tx_buffer, strlen((char const*)tx_buffer));
    }

    /*
     * Check Single Tap events
     */
    if (all_source.tap_src.single_tap)
    {
      sprintf((char*)tx_buffer, "Tap Detected: Sign %s",
              all_source.tap_src.tap_sign ? "positive" : "negative");
      if (all_source.tap_src.x_tap)
        sprintf((char*)tx_buffer, "%s on X", tx_buffer);
      if (all_source.tap_src.y_tap)
        sprintf((char*)tx_buffer, "%s on Y", tx_buffer);
      if (all_source.tap_src.z_tap)
        sprintf((char*)tx_buffer, "%s on Z", tx_buffer);
      sprintf((char*)tx_buffer, "%s axis\r\n", tx_buffer);
      tx_com(tx_buffer, strlen((char const*)tx_buffer));
    }
  }
}

/*
 * @brief  Write generic device register (platform dependent)
 *
 * @param  handle    customizable argument. In this examples is used in
 *                   order to select the correct sensor bus handler.
 * @param  reg       register to write
 * @param  bufp      pointer to data to write in register reg
 * @param  len       number of consecutive register to write
 *
 */
static int32_t platform_write(void *handle, uint8_t reg, uint8_t *bufp,
                              uint16_t len)
{
  if (handle == &hi2c1)
  {
    HAL_I2C_Mem_Write(handle, LIS2DTW12_I2C_ADD_L, reg,
                      I2C_MEMADD_SIZE_8BIT, bufp, len, 1000);
  }
#ifdef STEVAL_MKI109V3
  else if (handle == &hspi2)
  {
    HAL_GPIO_WritePin(CS_up_GPIO_Port, CS_up_Pin, GPIO_PIN_RESET);
    HAL_SPI_Transmit(handle, &reg, 1, 1000);
    HAL_SPI_Transmit(handle, bufp, len, 1000);
    HAL_GPIO_WritePin(CS_up_GPIO_Port, CS_up_Pin, GPIO_PIN_SET);
  }
#endif
  return 0;
}

/*
 * @brief  Read generic device register (platform dependent)
 *
 * @param  handle    customizable argument. In this examples is used in
 *                   order to select the correct sensor bus handler.
 * @param  reg       register to read
 * @param  bufp      pointer to buffer that store the data read
 * @param  len       number of consecutive register to read
 *
 */
static int32_t platform_read(void *handle, uint8_t reg, uint8_t *bufp,
                             uint16_t len)
{
  if (handle == &hi2c1)
  {
    HAL_I2C_Mem_Read(handle, LIS2DTW12_I2C_ADD_L, reg,
                     I2C_MEMADD_SIZE_8BIT, bufp, len, 1000);
  }
#ifdef STEVAL_MKI109V3
  else if (handle == &hspi2)
  {
    /* Read command */
    reg |= 0x80;
    HAL_GPIO_WritePin(CS_up_GPIO_Port, CS_up_Pin, GPIO_PIN_RESET);
    HAL_SPI_Transmit(handle, &reg, 1, 1000);
    HAL_SPI_Receive(handle, bufp, len, 1000);
    HAL_GPIO_WritePin(CS_up_GPIO_Port, CS_up_Pin, GPIO_PIN_SET);
  }
#endif
  return 0;
}

/*
 * @brief  Write generic device register (platform dependent)
 *
 * @param  tx_buffer     buffer to trasmit
 * @param  len           number of byte to send
 *
 */
static void tx_com(uint8_t *tx_buffer, uint16_t len)
{
  #ifdef NUCLEO_F411RE_X_NUCLEO_IKS01A2
  HAL_UART_Transmit(&huart2, tx_buffer, len, 1000);
  #endif
  #ifdef STEVAL_MKI109V3
  CDC_Transmit_FS(tx_buffer, len);
  #endif
}

/*
 * @brief  platform specific initialization (platform dependent)
 */
static void platform_init(void)
{
#ifdef STEVAL_MKI109V3
  TIM3->CCR1 = PWM_3V3;
  TIM3->CCR2 = PWM_3V3;
  HAL_Delay(1000);
#endif
}
