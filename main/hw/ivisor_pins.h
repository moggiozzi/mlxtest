#ifndef IVISOR_PINS_H_
#define IVISOR_PINS_H_

//в исходниках ESP-IDF gpio.c
//if ((gpio_pin_mask & ( GPIO_SEL_34 | GPIO_SEL_35 | GPIO_SEL_36 | GPIO_SEL_37 | GPIO_SEL_38 | GPIO_SEL_39))) {
//	ESP_LOGE(GPIO_TAG, "GPIO34-39 can only be used as input mode");

//#define BOARD_DEBUG_VERSION_0 (1)
#define BOARD_CACTUS_VERSION_1 (1)
#ifdef BOARD_DEBUG_VERSION_0
#define IVISOR_PIN_PDM_CLOCK (18)
#define IVISOR_PIN_PDM_DATA  (5)

#define IVISOR_PIN_I2C_SDA (26)
#define IVISOR_PIN_I2C_SCL (25)

#define IVISOR_PIN_PWM (32)

#define IVISOR_PIN_DO_2 (17)
#define IVISOR_PIN_DO_1 (27)
#define IVISOR_PIN_DI_1 (35)

#define IVISOR_PIN_DE_ASU (22)
#define IVISOR_PIN_RX_ASU (21)
#define IVISOR_PIN_TX_ASU (19)

#define IVISOR_PIN_LED (2)

#define IVISOR_PIN_ADC_5V (34)//(33)

//#define IVISOR_UART_TX_DBG ()
//#define IVISOR_UART_RX_DBG ()
#elif BOARD_CACTUS_VERSION_1

#define IVISOR_PIN_PDM_CLOCK (18)
#define IVISOR_PIN_PDM_DATA  (5)
#define IVISOR_PIN_I2C_SDA (26)
#define IVISOR_PIN_I2C_SCL (25)
#define IVISOR_PIN_PWM (32)
#define IVISOR_PIN_DO_2 (16)
#define IVISOR_PIN_DO_1 (4)
#define IVISOR_PIN_DI_1 (17)
#define IVISOR_PIN_DE_ASU (22)
#define IVISOR_PIN_RX_ASU (21)
#define IVISOR_PIN_TX_ASU (19)
#define IVISOR_PIN_LED (2)
#define IVISOR_PIN_ADC_5V (33)

#endif

#endif
