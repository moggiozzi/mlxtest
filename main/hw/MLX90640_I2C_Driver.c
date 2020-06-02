/**
 * @copyright (C) 2017 Melexis N.V.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include "MLX90640_I2C_Driver.h"

#include "ivisor_pins.h"

#include "driver/i2c.h"
#include "esp_console.h"
#include "esp_log.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define I2C_GPIO_SDA (IVISOR_PIN_I2C_SDA)
#define I2C_GPIO_SCL (IVISOR_PIN_I2C_SCL)
#define I2C_FREQ (400000)//(100000)
static i2c_port_t i2c_port = I2C_NUM_0;
//static const char *TAG = "MLX90640_I2C_Driver.c";

static esp_err_t i2c_master_driver_initialize(void)
{
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_GPIO_SDA,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = I2C_GPIO_SCL,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_FREQ
    };
    return i2c_param_config(i2c_port, &conf);
}

#define I2C_MASTER_TX_BUF_DISABLE 0 /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE 0 /*!< I2C master doesn't need buffer */
#define WRITE_BIT I2C_MASTER_WRITE  /*!< I2C master write */
#define READ_BIT I2C_MASTER_READ    /*!< I2C master read */
#define ACK_CHECK_EN 0x1            /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS 0x0           /*!< I2C master will not check ack from slave */
#define ACK_VAL 0x0                 /*!< I2C ack value */
#define NACK_VAL 0x1                /*!< I2C nack value */

static uint8_t i2cData[1664] = {0};

int MLX90640_I2CInit()
{   
	esp_err_t res = ESP_OK;
	res |= i2c_master_driver_initialize();
    res |= i2c_driver_install(i2c_port, I2C_MODE_MASTER, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
//    int hp=0,lp=0;
//    i2c_get_period(i2c_port,&hp,&lp);
//    ESP_LOGD(__func__, "i2c high period %d, low period %d", hp, lp);
//    i2c_get_data_timing(i2c_port,&hp,&lp);
//    ESP_LOGD(__func__, "i2c sample time %d, hold time %d", hp, lp);

    return res;
}

int MLX90640_I2CRead(uint8_t slaveAddr, uint16_t startAddress, uint16_t nMemAddressRead, uint16_t *data)
{
    uint8_t sa;
    int ack = 0;
    int cnt = 0;
    int i = 0;
    char cmd[2] = {0,0};
    uint16_t *p;
    
    p = data;
    sa = (slaveAddr << 1);
    cmd[0] = startAddress >> 8;
    cmd[1] = startAddress & 0xFF;

    i2c_cmd_handle_t i2ccmd = i2c_cmd_link_create();
    ack = i2c_master_start(i2ccmd);
    if(ack != 0)
    {
    	goto ret;
    }

	ack = i2c_master_write_byte(i2ccmd, sa | WRITE_BIT, ACK_CHECK_EN) != ESP_OK;
    if(ack != 0)
    {
        goto ret;
    }

    ack = i2c_master_write(i2ccmd, (uint8_t*)cmd, 2, ACK_CHECK_EN) != ESP_OK;
    if(ack != 0)
    {
    	goto ret;
    }

    ack = i2c_master_start(i2ccmd);
    if(ack != 0)
    {
    	goto ret;
    }

    ack = i2c_master_write_byte(i2ccmd, sa | READ_BIT, ACK_CHECK_EN) != ESP_OK;
    if(ack != 0)
    {
    	goto ret;
    }

    int len = (nMemAddressRead << 1);
    if (len > 1) {
        ack = i2c_master_read(i2ccmd, (uint8_t*)i2cData, len - 1, ACK_VAL) != ESP_OK;
        if(ack != 0)
        {
        	goto ret;
        }
    }
    ack = i2c_master_read_byte(i2ccmd, ((uint8_t*)i2cData) + len - 1, NACK_VAL);
    if(ack != 0)
    {
    	goto ret;
    }

    ack = i2c_master_stop(i2ccmd);
    if(ack != 0)
    {
    	goto ret;
    }
    ack = i2c_master_cmd_begin(i2c_port, i2ccmd, 1000 / portTICK_RATE_MS) != ESP_OK;

	for(cnt=0; cnt < nMemAddressRead; cnt++)
	{
		i = cnt << 1;
		*p++ = (uint16_t)i2cData[i]*256 + (uint16_t)i2cData[i+1];
	}
//	if (nMemAddressRead == 1) //debug log registers
//		ESP_LOGD(__func__, "addr 0x%x = 0x%x", startAddress, *data);
ret:
	i2c_cmd_link_delete(i2ccmd);
	return ack;
} 

int MLX90640_I2CWrite(uint8_t slaveAddr, uint16_t writeAddress, uint16_t data)
{
    uint8_t sa;
    int ack = 0;
    char cmd[4] = {0,0,0,0};
    //static uint16_t dataCheck;

    sa = (slaveAddr << 1);
    cmd[0] = writeAddress >> 8;
    cmd[1] = writeAddress & 0xFF;
    cmd[2] = data >> 8;
    cmd[3] = data & 0xFF;

    i2c_cmd_handle_t i2ccmd = i2c_cmd_link_create();
    // 1) write Slave address
    ack = i2c_master_start(i2ccmd);
    if(ack != 0)
    {
    	goto ret;
    }

    ack = i2c_master_write_byte(i2ccmd, sa | WRITE_BIT, ACK_CHECK_EN) != ESP_OK;
    if (ack != 0)
    {
        goto ret;
    }

    // 2) write addr & data
    ack = i2c_master_write(i2ccmd, (uint8_t*)cmd, 4, ACK_CHECK_EN) != ESP_OK;
    if (ack != 0)
    {
    	goto ret;
    }

    ack = i2c_master_stop(i2ccmd);
    if(ack != 0)
    {
    	goto ret;
    }

    ack = i2c_master_cmd_begin(i2c_port, i2ccmd, 1000 / portTICK_RATE_MS) != ESP_OK;
    if (ack != 0)
    {
    	goto ret;
    }
    
    //результат не проверяется, т.к. после записи аппаратный регистр не гарантирует чтения того же значения
//    MLX90640_I2CRead(slaveAddr, writeAddress, 1, &dataCheck);
//    if ( dataCheck != data)
//    {
//        ack = -1;
//    }
ret:
	i2c_cmd_link_delete(i2ccmd);
    return ack;
}

