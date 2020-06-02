#ifndef IVISOR_THERMO_H_
#define IVISOR_THERMO_H_
#include "thermo_common.h"

#include <stdbool.h>
#include <stdint.h>

enum {
	T_MEAS_MIN,
	T_MEAS_MAX,
	T_MEAS_AVG,
	T_MEAS_COUNT
};
extern float measures[T_MEAS_COUNT];

enum {
	T_MEAS_VALUE_TYPE_ADC,
	T_MEAS_VALUE_TYPE_CELCIUS,
	T_MEAS_VALUE_TYPE_FAHRENHEIT,
	T_MEAS_VALUE_TYPE_COUNT
};

enum {
	T_RELAY_WARNING,
	T_RELAY_ALARM,
	T_RELAY_COUNT
};

bool thermo_getRelayState(int idx);
uint16_t thermo_getRelaysState(void);
void thermo_updateRelaySettings(void);

bool thermo_init(void);

float * thermo_getMatrix(void);
uint16_t * thermo_getMatrix16(void);
float thermo_getMatrixPixel(int pixelIdx, int measTypeIdx);

float thermo_getMeasureUserUnits(int measIdx, int measTypeIdx);

#endif
