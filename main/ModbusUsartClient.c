#include <string.h>
#include <stdbool.h>
#include "time.h"
#include "sys/time.h"

#include "debug.h"
#include "driver/uart.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "ModbusUsartClient.h"
#include "soc/uart_reg.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "hw/ivisor_pins.h"
#include "task_prio.h"
/* ----------------------- Defines ------------------------------------------*/
#define LAVR_CHECK(a, ret_val, str, ...) \
    if (!(a)) { \
        ESP_LOGE(DBG_TAG, "%s(%u): " str, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
        return (ret_val); \
    }

#define MB_QUEUE_LENGTH             (256)

#define MB_SERIAL_TASK_PRIO         (MODBUS_UART_TASK_PRIO)
#define MB_SERIAL_TASK_STACK_SIZE   (4*1024)

// Set buffer size for transmission
#define MB_SERIAL_BUF_SIZE          (256)
#define MB_SERIAL_TX_TOUT_MS        (100)
#define MB_SERIAL_TX_TOUT_TICKS     pdMS_TO_TICKS(MB_SERIAL_TX_TOUT_MS) // timeout for transmission

/* ----------------------- Static variables ---------------------------------*/
#define DBG_TAG "MB_USART_CL"

#define TXD_PIN (IVISOR_PIN_TX_ASU)
#define RXD_PIN (IVISOR_PIN_RX_ASU)

#define UART_TIMEOT_EVENT UART_EVENT_MAX+1

// A queue to handle UART event.
static QueueHandle_t xMbUartQueue;
static TaskHandle_t  xMbTaskHandle;

#define MODBUS_UART (1)

// The UART hardware port number
static uint8_t uartNumber = MODBUS_UART;

static uint8_t buffer[MB_SERIAL_BUF_SIZE]; // Temporary buffer to transfer received data to modbus stack
//static uint16_t rxBufferPos = 0;    // position in the receiver buffer

void usartClientSetModeTx(void) {
	gpio_set_level(IVISOR_PIN_DE_ASU, false);
}
void usartClientSetModeRx(void) {
	gpio_set_level(IVISOR_PIN_DE_ASU, true);
}

uint16_t s_length = 0;
static void rxPoll(size_t xEventSize)
{
	if (xEventSize > 0) {
		// Get received packet into Rx buffer
		s_length = uart_read_bytes(uartNumber, &buffer[0], MB_SERIAL_BUF_SIZE, 0);
	}
}

char helloStr[] = "uart start\n";
// UART receive event task
static void vUartTask(void* pvParameters)
{
    uart_event_t xEvent;
    int event;
    usartClientSetModeTx();
    uart_send((unsigned char *)helloStr, sizeof(helloStr));
    usartClientSetModeRx();
    for(;;) {
//        ESP_LOGI(DBG_TAG, "MB_uart queue msgs: %d", uxQueueMessagesWaiting(xMbUartQueue));
        if (xQueueReceive(xMbUartQueue, (void*)&xEvent, portMAX_DELAY) == pdTRUE) { // portMAX_DELAY
            event = xEvent.type;
//            ESP_LOGI(DBG_TAG, "MB_uart event:%d", event);
            switch(event) {
                //Event of UART receiving data
                case UART_DATA:
                    ESP_LOGI(DBG_TAG,"Receive data, len: %d.", xEvent.size);
                    // Read received data and send it to modbus stack
                    rxPoll(xEvent.size);
                    mbrtuTmr_startIntercharTimer();
                    break;
                //Event of HW FIFO overflow detected
                case UART_FIFO_OVF:
                    //ESP_LOGI(DBG_TAG, "hw fifo overflow.");
//                    xQueueReset(xMbUartQueue);
                    break;
                //Event of UART ring buffer full
                case UART_BUFFER_FULL:
                    //ESP_LOGI(DBG_TAG, "ring buffer full.");
//                    xQueueReset(xMbUartQueue);
                    uart_flush_input(uartNumber);
                    break;
                //Event of UART RX break detected
                case UART_BREAK:
                	mbuart_dbg_printf("uart rx break.");
                    break;
                //Event of UART parity check error
                case UART_PARITY_ERR:
                	mbuart_dbg_printf("uart parity error.");
                    break;
                //Event of UART frame error
                case UART_FRAME_ERR:
                	mbuart_dbg_printf("uart frame error.");
                    break;
                case UART_TIMEOT_EVENT:
                	mbuart_dbg_printf("uart timeout.");
                    // stop receiving
                    usartClientSetModeTx();
					uart_send(buffer, s_length);
                    //start receiving
                    usartClientSetModeRx();
                    break;
                default:
                	mbuart_dbg_printf("uart event type: %d.", xEvent.type);
                    break;
            }
        }
        //diag_setStackUsage(IVISOR_TASK_MODBUS_UART, uxTaskGetStackHighWaterMark(NULL));
    }
    vTaskDelete(NULL);
}

portBASE_TYPE IRAM_ATTR uart_addTimeoutEventFromIsr(void)
{
    portBASE_TYPE hPTaskAwoken = 0;

    uart_event_t xEvent = {.type = UART_TIMEOT_EVENT, .size = 0};
    xQueueSendFromISR(xMbUartQueue, (void * )&xEvent, &hPTaskAwoken);

    return  hPTaskAwoken;
}

void delayMksActive(uint32_t mks) {
	struct timeval tv_now;
	gettimeofday(&tv_now, NULL);
	uint32_t t1 = (uint32_t)((int64_t) tv_now.tv_sec * 1000000L + (int64_t) tv_now.tv_usec);
	while(true) {
		gettimeofday(&tv_now, NULL);
		uint32_t t2 = (uint32_t)((int64_t) tv_now.tv_sec * 1000000L + (int64_t) tv_now.tv_usec);
		if (t2 - t1 >= mks)
			break;
	}
}

int8_t uart_send(uint8_t * pData, uint16_t dataLength)
{
    int16_t length = uart_write_bytes(uartNumber, (char *)pData, dataLength);
    if (length < 0)
    {
        return -1;
    }
    if (length != dataLength)
    {
        return -1;
    }
    
    mbuart_dbg_printf("MB_TX_buffer sent: (%d) bytes.", length);
    
	// fixme без таймаута uart_wait_tx_done() не работает
    delayMksActive(150);

    // Waits while UART sending the packet
    esp_err_t xTxStatus = uart_wait_tx_done(uartNumber, MB_SERIAL_TX_TOUT_TICKS);
    LAVR_CHECK((xTxStatus == ESP_OK), -1, "mb serial sent buffer failure.");
    return 0;//MBRTU_STATUS_OK;
}

bool uartClient_init(uint32_t baudRate)
{
    esp_err_t err;
	gpio_config_t io_conf;
	//disable interrupt
	io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
	//set as output mode
	io_conf.mode = GPIO_MODE_OUTPUT;
	//bit mask of the pins that you want to set
	io_conf.pin_bit_mask = ((uint64_t)1 << IVISOR_PIN_DE_ASU);
	//disable pull-down mode
	io_conf.pull_down_en = 0;
	//disable pull-up mode
	io_conf.pull_up_en = 0;
	//configure GPIO with the given settings
	err = gpio_config(&io_conf);
	if (err != ESP_OK) {
		ESP_LOGE(DBG_TAG, "Config GPIO_%d error %d", IVISOR_PIN_DE_ASU, err);
	}

    uart_config_t xUartConfig = {
        .baud_rate = baudRate,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 2,
    };

    // Install UART driver, and get the queue.
    err = uart_driver_install(uartNumber, MB_SERIAL_BUF_SIZE, MB_SERIAL_BUF_SIZE,
            MB_QUEUE_LENGTH, &xMbUartQueue, ESP_INTR_FLAG_LEVEL3);
    LAVR_CHECK((err == ESP_OK), false,
            "mb serial driver failure, uart_driver_install() returned (0x%x).", (uint32_t)err);

    // Set UART config
    err = uart_param_config(uartNumber, &xUartConfig);
    uart_set_pin(uartNumber, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    LAVR_CHECK((err == ESP_OK),
            false, "uart_param_config() returned (0x%x).", (uint32_t)err);

    uart_intr_config_t uart_intr = {
        .intr_enable_mask = UART_RXFIFO_FULL_INT_ENA_M
                            | UART_RXFIFO_TOUT_INT_ENA_M
                            | UART_FRM_ERR_INT_ENA_M
                            | UART_RXFIFO_OVF_INT_ENA_M
                            | UART_BRK_DET_INT_ENA_M
                            | UART_PARITY_ERR_INT_ENA_M,
        .rxfifo_full_thresh = 32,
        .rx_timeout_thresh = 2,
        .txfifo_empty_intr_thresh = 10
    };
    uart_intr_config(uartNumber, &uart_intr);

    // Create a task to handle UART events
    BaseType_t xStatus = xTaskCreate(vUartTask, "uart_queue_task", MB_SERIAL_TASK_STACK_SIZE,
                                        NULL, MB_SERIAL_TASK_PRIO, &xMbTaskHandle);
    if (xStatus != pdPASS) {
        vTaskDelete(xMbTaskHandle);
        // Force exit from function with failure
        LAVR_CHECK(false, false,
                "mb stack serial task creation error. xTaskCreate() returned (0x%x).",
                (uint32_t)xStatus);
    } else {
        //vTaskSuspend(xMbTaskHandle); // Suspend serial task while stack is not started
    }

	uint32_t uartFifoFillTime = (1000000ul * 11ul * 32ul / baudRate);
	if (baudRate > 19200) {
		// fixed: 1750us
		mbrtuTmr_init(1750 + uartFifoFillTime);
	} else {
		// 3.5 char times
		mbrtuTmr_init(
				(1000000ul * 11ul * 7ul) / (2ul * baudRate) + uartFifoFillTime);
	}

	mbuart_dbg_printf("%s Init serial.", __func__);
    return true;
}


void uart_close(void)
{
    (void)vTaskDelete(xMbTaskHandle);
    uart_driver_delete(uartNumber);
}
