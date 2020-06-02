#ifndef APPLICATION_USER_DBG_DEBUG_H_
#define APPLICATION_USER_DBG_DEBUG_H_

#include "esp_log.h"

#ifndef DEBUG
#define dbg_printf(tag,fmt,...) ESP_LOGD(tag, fmt, ##__VA_ARGS__)
#define dbg_vprintf(fmt,vargs) esp_log_writev(ESP_LOG_DEBUG, "d", fmt, vargs)
void dbg_print_malloc_stats(const char* tag);
size_t dbg_get_max_heap_used(void);
void dbg_update_malloc_stats(void);
int dbg_printf_with_time_prefix(const char* p_format_string, ...);
int dbg_printf_with_timestamp(const char* p_format_string, ...);
void dbg_dump_bytes(const void* pData, unsigned int dataSize);
void dbg_direct_puts(const char* s);
void dbg_dump_bytes(const void* pData, unsigned int dataSize);
void dbg_direct_puts(const char* s);
#else
#define dbg_printf(...)
#define dbg_vprintf(...)
#define dbg_print_malloc_stats(...)
#define dbg_get_max_heap_used()  0
#define dbg_update_malloc_stats()
#define dbg_printf_with_time_prefix(...)
#define dbg_printf_with_timestamp(...)
#define dbg_dump_bytes(...)
#define dbg_direct_puts(...)
#endif

//#define mbt_dbg_printf(...)
//#define mbt_dbg_printf(fmt,...) printf(fmt, ##__VA_ARGS__)

//#define tlog_dbg_printf(...)
#define tlog_dbg_printf(fmt,...) dbg_printf("thermoLog.c", fmt, ##__VA_ARGS__)

#define sppt_dbg_printf(...)
//#define sppt_dbg_printf(fmt,...) dbg_printf("spp_task.c", fmt, ##__VA_ARGS__)

#define spp_dbg_printf(...)
////#define spp_dbg_printf(fmt,...) dbg_printf_with_time_prefix(fmt, ##__VA_ARGS__)
//#define spp_dbg_printf(fmt,...) dbg_printf("bt_spp.c", fmt, ##__VA_ARGS__)

//#define set_dbg_printf(...)
#define set_dbg_printf(fmt,...) dbg_printf("settings.c", fmt, ##__VA_ARGS__)

//#define runtimeCnt_dbg_printf(...)
#define runtimeCnt_dbg_printf(fmt,...) dbg_printf("runtime_counter.c", fmt, ##__VA_ARGS__)

//#define fs_dbg_printf(...)
#define fs_dbg_printf(fmt,...) dbg_printf("file_system.c", fmt, ##__VA_ARGS__)

//#define diag_dbg_printf(...)
#define diag_dbg_printf(fmt,...) dbg_printf("diagnostic.c", fmt, ##__VA_ARGS__)

#define rawFlash_dbg_printf(...)
//#define rawFlash_dbg_printf(fmt,...) dbg_printf("raw_flash.c", fmt, ##__VA_ARGS__)

//#define dio_dbg_printf(...)
#define dio_dbg_printf(fmt,...) dbg_printf("discreteInOut.c", fmt, ##__VA_ARGS__)

//#define pdm_dbg_printf(...)
#define pdm_dbg_printf(fmt,...) dbg_printf("pdm_mic.c", fmt, ##__VA_ARGS__)

#define dbg_fileio(...)
//#define dbg_fileio(fmt,...) dbg_printf("FileIO.c", fmt, ##__VA_ARGS__)

//#define thermo_dbg_printf(...)
#define thermo_dbg_printf(fmt,...) dbg_printf("thermo.c", fmt, ##__VA_ARGS__)

#define mbuart_dbg_printf(...)
//#define mbuart_dbg_printf(fmt,...) dbg_printf("ModbusUsrtClient.c", fmt, ##__VA_ARGS__)

#define MB_USE_CONSOLE_OUTPUT
//#define MB_ONLY_WARNINGS
#ifdef MB_USE_CONSOLE_OUTPUT
#ifdef MB_ONLY_WARNINGS
#define mb_printf(...)
#else
#define mb_printf(fmt, ...) dbg_printf_with_time_prefix(fmt, ##__VA_ARGS__)
#endif
#define mb_warning(fmt, ...) dbg_printf_with_time_prefix(fmt, ##__VA_ARGS__)
#else
#define mb_printf(...)
#define mb_warning(...)
#endif

#define ASSERT_WHILE_COMPILE_II(EXP, LN) typedef char _assertion_failure_in_line_##LN[-1+10*(EXP)]
#define ASSERT_WHILE_COMPILE_I(EXP, LN) ASSERT_WHILE_COMPILE_II(EXP, LN)
#define ASSERT_WHILE_COMPILE(EXP) ASSERT_WHILE_COMPILE_I(EXP, __LINE__)
/*-----------------------------------------------------------------------------------------------*/
#define ASSERT_SIZE(x,y) ASSERT_WHILE_COMPILE(sizeof(x) == (y))

#endif /* APPLICATION_USER_DBG_DEBUG_H_ */
