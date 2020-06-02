#ifndef IVISOR_TASK_PRIO_H_
#define IVISOR_TASK_PRIO_H_

//#include "freertos/task.h" // tskIDLE_PRIORITY == 0

#define SPP_WRITE_TASK_PRIO   (10)
#define SPP_READ_TASK_PRIO     (9)
#define THERMO_TASK_PRIO       (20)//(8)
#define TLOG_TASK_PRIO         (7)
#define MODBUS_UART_TASK_PRIO (10)
#define LED_TASK_PRIO          (6)

#define PDM_RECORD_TASK_PRIO  (19)
#define PDM_SAVE_TASK_PRIO    (20)
#endif
