/****************************************************************************
* Создает SPP-сервер. После открытия соединения транслирует данные вежду UART0 и  SPP в обе стороны
* Основано на примере "bt_spp_vfs_acceptor demo". 
****************************************************************************/

#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include "nvs.h"
#include "nvs_flash.h"
#include "esp_log.h"

#include "thermo.h"

#define DBG_TAG "MAIN"

void app_main(void)
{
	ESP_LOGI(DBG_TAG, "%s START", __func__);

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK( ret );

    ESP_LOGI(DBG_TAG, "Thermo init...");
    if (!thermo_init())
    	ESP_LOGE(DBG_TAG, "%s Thermo init failed", __func__);
}


