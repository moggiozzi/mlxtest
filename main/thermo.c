#include "thermo.h"
#include "thermo_common.h"
#include "hw/MLX90640_I2C_Driver.h"
#include "hw/MLX90640_API.h"
#include "debug.h"
#include "task_prio.h"
#include "time.h"
#include "sys/time.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include <stdint.h>

#define THERMO_TASK_PERIOD_MS (10)

static uint16_t eeMLX90640[832];
static uint16_t mlx90640Frame[835];
paramsMLX90640 mlx90640;
static float mlx90640To[MLX_MATR_PIXEL_COUNT];
static uint16_t s_matrix[MLX_MATR_PIXEL_COUNT];

uint32_t time_ms(void) {
	struct timeval tv_now;
	gettimeofday(&tv_now, NULL);
	int64_t time_us = (int64_t) tv_now.tv_sec * 1000000L
			+ (int64_t) tv_now.tv_usec;
	return (uint32_t)(time_us / 1000);
}

#define TA_SHIFT 8 //the default shift for a MLX90640 device in open air
float tr = 0;
uint16_t subpage = 0;
void matrix_start_(void) {
	MLX90640_StartMeasurement();
}
float vdd = 0;
bool matrix_read_(void) {
	bool res = true;;
	float emissivity = 1;//0.95; // todo ust
	static uint32_t t1 = 0;
	static uint32_t t1_prev = 0;
	static uint32_t t2 = 0;
	static uint32_t t3 = 0;
	t1 = time_ms();

	if(MLX90640_GetFrameData(mlx90640Frame) != 0)
		res = false;
	t2 = time_ms();
	if (!res) {
		thermo_dbg_printf("ERROR can't read MLX90640 data");
	} else {
		tr = MLX90640_GetTa(mlx90640Frame, &mlx90640) - TA_SHIFT;
		//The object temperatures//for all 768 pixels in a//frame are stored in the//mlx90640To array
		MLX90640_CalculateTo(mlx90640Frame, &mlx90640, emissivity, tr, mlx90640To);
		vdd = MLX90640_GetVdd(mlx90640Frame, &mlx90640); //vdd = 3.3
		//todo
//		MLX90640_BadPixelsCorrection((&mlx90640)->brokenPixels, mlx90640To, 1, &mlx90640);
//		MLX90640_BadPixelsCorrection((&mlx90640)->outlierPixels, mlx90640To, 1, &mlx90640);

		for(int i=0;i<768;i++)
			s_matrix[i] = (uint16_t)mlx90640To[i];
		t3 = time_ms();
	}
	thermo_dbg_printf(" period:%d,read:%d,calc:%d,vdd %.2f", t1 - t1_prev, t2-t1, t3-t2,vdd);
	t1_prev = t1;
	return res;
}

bool isDetect = false;
void calc_measures(void) {
	float min_val = MLX_TEMP_MAX;
	float max_val = MLX_TEMP_MIN;
	for (int idx = 0; idx < MLX_MATR_PIXEL_COUNT; idx++) {
		if (mlx90640To[idx] < min_val)
			min_val = mlx90640To[idx];
		if (mlx90640To[idx] > max_val)
			max_val = mlx90640To[idx];
	}

	thermo_dbg_printf("min %.0f, max %.0f %d", min_val, max_val, (int)isDetect);
	if (max_val > 40 || min_val < 10) {
		isDetect = true;
		thermo_dbg_printf("ERR_DETECT %f %f", min_val, max_val);
#if 1
		for (int i = 0; i < MLX_MATR_HEIGHT; i++) {
			for (int j = 0; j < MLX_MATR_WIDTH; j++) {
				int idx = i * MLX_MATR_WIDTH + j;
				printf("%4x(%4.0f),", mlx90640Frame[idx], mlx90640To[idx]);
			}
			printf("\n");
		}
#endif
		thermo_dbg_printf("ctrl:0x%x,stat:0x%x", mlx90640Frame[832],	mlx90640Frame[834]);
	}
}

void readAfterStart(void) {
	int res;
	res = -1;
	for(int retryCounter = 3; retryCounter > 0 && res != 0; retryCounter--) {
		res = MLX90640_DumpEE(eeMLX90640);
		if (res != 0) {
			thermo_dbg_printf("ERROR DumpEE %d", res);
		}
	}
	res = -1;
	for(int retryCounter = 3; retryCounter > 0 && res != 0; retryCounter--) {
		res = MLX90640_ExtractParameters(eeMLX90640, &mlx90640);
		if (res != 0) {
			thermo_dbg_printf("ERROR ExtractParameters %d", res);
		}
	}
	matrix_read_();
}

void task_thermo(void *arg)
{
	vTaskDelay(pdMS_TO_TICKS(1500));
	readAfterStart();
	while(1)
	{
		if(matrix_read_()) {
			calc_measures();
		}else{
			thermo_dbg_printf("can't read");
		}
		int delayMs = rand() % 1000;
		vTaskDelay(pdMS_TO_TICKS(delayMs));
//		vTaskDelay(pdMS_TO_TICKS(10));
	}
}

bool thermo_init(void)
{
	bool res = true;

	if (MLX90640_I2CInit() != 0)
		res = false;

	uint16_t ctrl = 0;
	MLX90640_GetControl(&ctrl);
	thermo_dbg_printf("ctrl: 0x%04x", ctrl); // 0x1901
    //MLX90640_SetSubPageRepeat(1); // отключить автоматическое переключение страниц (todo разобраться: почему-то бит ломает вычисление температуры)
    //MLX90640_SetDataHold(1);// перезапись данных только по флагу overwrite
    //MLX90640_SetDataHold(0);
    //MLX90640_SetChessMode();
    //MLX90640_SetInterleavedMode();
    MLX90640_SetRefreshRate(MLX_FREQ_2);

    thermo_dbg_printf("RefreshRate: %d", MLX90640_GetRefreshRate());

    MLX90640_GetControl(&ctrl);
    thermo_dbg_printf("ctrl: 0x%04x", ctrl);

	if (pdPASS != xTaskCreate(task_thermo, "task_thermo", 6*1024, NULL, THERMO_TASK_PRIO, NULL))
		res = false;

	return res;
}
