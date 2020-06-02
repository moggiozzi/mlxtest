#include <malloc.h>
#include <stdarg.h>
#include <stddef.h>
#include <time.h>
#include "hw/rtc_module.h"
#include "debug.h"

static size_t s_max_heap_used = 0;
static const char s_p_null_string[] = "(null)";

#define DBG_TAG "debug.c"

/*=========================================== ФУНКЦИИ ===========================================*/
void dbg_print_malloc_stats(const char* tag)
{
	struct mallinfo info = mallinfo();

	if (info.uordblks > s_max_heap_used)
	{
		s_max_heap_used = info.uordblks;
	}

	dbg_printf(DBG_TAG, "%s total space=%u, used=%u, free=%u, max used=%u\r\n",
		tag, info.arena, info.uordblks, info.fordblks, s_max_heap_used);
} /* dbg_print_malloc_stats() */
/*-----------------------------------------------------------------------------------------------*/
size_t dbg_get_max_heap_used(void)
{
	return s_max_heap_used;
} /* dbg_get_max_heap_used() */
/*-----------------------------------------------------------------------------------------------*/
void dbg_update_malloc_stats(void)
{
	struct mallinfo info = mallinfo();

	if (info.uordblks > s_max_heap_used)
	{
		s_max_heap_used = info.uordblks;
	}
} /* dbg_update_malloc_stats() */
/*-----------------------------------------------------------------------------------------------*/
int dbg_printf_with_time_prefix(const char* p_format_string, ...)
{
  int string_length = 0;
  va_list p_arg;
  int64_t tms = time_ms();
  time_t t = (time_t)(tms / 1000ULL);
  struct tm tm_s;
  localtime_r(&t, &tm_s);
  dbg_printf(DBG_TAG, "[%02d:%02d:%02d.%03d]> ", tm_s.tm_hour, tm_s.tm_min, tm_s.tm_sec, (uint16_t)(tms % 1000));

  va_start(p_arg, p_format_string);
  dbg_vprintf(p_format_string, p_arg);
  va_end(p_arg);

  return string_length;
}
/*-----------------------------------------------------------------------------------------------*/
int dbg_printf_with_timestamp(const char* p_format_string, ...)
{
	int string_length = 0;
	va_list p_arg;
	int64_t tms = time_ms();
	time_t t = (time_t)(tms / 1000ULL);
	struct tm tm_s;
	localtime_r(&t, &tm_s);
	dbg_printf(DBG_TAG, "[%4u-%02u-%02u %02u:%02u:%02u.%03u]> ", tm_s.tm_year+1900, tm_s.tm_mon+1, tm_s.tm_mday,
		tm_s.tm_hour, tm_s.tm_min, tm_s.tm_sec, (uint16_t)(tms % 1000ULL));

	va_start(p_arg, p_format_string);
	dbg_vprintf(p_format_string, p_arg);
	va_end(p_arg);

	return string_length;
}
/*-----------------------------------------------------------------------------------------------*/
void dbg_dump_bytes(const void* pData, unsigned int dataSize)
{
	unsigned int bytesPerLine = 16;

	for (unsigned int i = 0; i < dataSize; i++)
	{
		/* Начало строки */
		if ((i & (bytesPerLine - 1)) == 0)
		{
			dbg_printf(DBG_TAG, "\t%04X : ", i);
		}

		dbg_printf(DBG_TAG, "%02X ", ((const uint8_t*)pData)[i]);

		/* Конец строки */
		if (((i + 1) & (bytesPerLine - 1)) == 0)
		{
			dbg_printf(DBG_TAG, "\n");
		}
	}

	dbg_printf(DBG_TAG, "\n");
}
/*-----------------------------------------------------------------------------------------------*/
const char* dbg_safe_str(const char* pString)
{
	return pString == NULL ? s_p_null_string : pString;
}
/*-----------------------------------------------------------------------------------------------*/
void dbg_direct_puts(const char* s)
{
//	const char* p = s;
//
//	SET_BIT(USART1->CR1, USART_CR1_TE);
//	while (*p != '\0')
//	{
//		while ((USART1->ISR & USART_ISR_TXE) == 0)
//		{
//			/* Пустое тело цикла. */
//			__HAL_IWDG_RELOAD_COUNTER(&hiwdg);
//		}
//
//		USART1->TDR = *p++;
//	}
}
/*-----------------------------------------------------------------------------------------------*/

/*@}*/
