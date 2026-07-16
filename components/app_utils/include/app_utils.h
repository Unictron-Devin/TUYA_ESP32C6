#ifndef _APP_UTILS_H_
#define _APP_UTILS_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize application utilities
 */
void app_utils_init(void);

/**
 * @brief Get system uptime in milliseconds
 * @return Uptime in milliseconds
 */
uint64_t app_utils_get_uptime_ms(void);

/**
 * @brief Print memory statistics
 */
void app_utils_print_mem_stats(void);

/**
 * @brief Get free heap size
 * @return Free heap size in bytes
 */
uint32_t app_utils_get_free_heap(void);

#ifdef __cplusplus
}
#endif

#endif // _APP_UTILS_H_
