/*
 * Copyright (c) 2025 PHY Wireless, Inc.
 *
 * SPDX-License-Identifier: LicenseRef-PHYW
 */

#include <zephyr/kernel.h>
#include <zephyr/timing/timing.h>

#include <stdint.h>
#include <otdoa_al/otdoa_al2otdoa_api.h>

struct k_timer otdoa_timers[OTDOA_MAX_TIMERS];

/* Timer "user data".  For now, it is just an array of uint32's that
 * holds the timer number.
 */
typedef uint32_t tTIMER_USER_DATA;

tTIMER_USER_DATA otdoa_timer_user_data[OTDOA_MAX_TIMERS];

/*
 * Expiry function
 */
void otdoa_timer_expiry_function(struct k_timer *pTimer)
{
	if (pTimer) {
		tTIMER_USER_DATA *pUserData = k_timer_user_data_get(pTimer);

		otdoa_handle_timeout(*pUserData);
	}
}

/*
 * Initialize the timers
 *
 */
int otdoa_timers_init(void)
{
	for (unsigned int ui = 0; ui < OTDOA_MAX_TIMERS; ui++) {
		otdoa_timer_user_data[ui] = (uint8_t)ui;
		k_timer_init(otdoa_timers + ui, otdoa_timer_expiry_function, NULL);
		/* set the timer user data to point to user data array */
		k_timer_user_data_set(otdoa_timers + ui, (void *)(otdoa_timer_user_data + ui));
	}
	return 0;
}

/*
 * Start / Stop the timers
 */
int otdoa_timer_start(unsigned int u_timer_no, unsigned int u_duration_ms)
{
	if (u_timer_no >= OTDOA_MAX_TIMERS) {
		return -1;
	}

	/* timer is started as one-shot, so period=K_NO_WAIT */
	k_timer_start(otdoa_timers + u_timer_no, K_MSEC(u_duration_ms), K_NO_WAIT);
	return 0;
}

int otdoa_timer_stop(unsigned int u_timer_no)
{
	if (u_timer_no >= OTDOA_MAX_TIMERS) {
		return -1;
	}

	k_timer_stop(otdoa_timers + u_timer_no);
	return 0;
}

/*
 * Check if a timer is running
 */
int otdoa_timer_active(unsigned int u_timer_no)
{
	if (u_timer_no >= OTDOA_MAX_TIMERS) {
		return 0;
	}
	if (k_timer_remaining_ticks(otdoa_timers + u_timer_no) > 0) {
		return 1;
	}
	return 0;
}

static timing_t start_time, stop_time;
static uint64_t total_cycles;

// Initialize the Timer & cycle counter
void TST_initTimerAndCycleCnt(void)
{
	timing_init();
	total_cycles = 0;
}

void TST_timerStart(void)
{
	timing_start();
	start_time = timing_counter_get();
}

void TST_timerStop(void)
{
	stop_time = timing_counter_get();
	timing_stop();
	total_cycles += timing_cycles_get(&start_time, &stop_time);
}

// log the count of cycles
void TST_logCycles( const char* const pszName, int16_t i16Count, const char* pszUnits)
{
	printk("Timer %s: %lld cycles (%d %s)\n", pszName, total_cycles, i16Count, pszUnits);
}

// Pause the timer and accumulate cycles
void TST_pauseTimer(void)
{
	stop_time = timing_counter_get();
	total_cycles += timing_cycles_get(&start_time, &stop_time);
	start_time = stop_time;
}

static timing_t dwt_start_time, dwt_stop_time;
static uint64_t dwt_total_cycles;

// DWT timer functions
void TST_InitDWT()
{
	timing_init();
	dwt_total_cycles = 0;
}

void TST_DWTStart(void)
{
	timing_start();
	dwt_start_time = timing_counter_get();
}

void TST_DWTStop(void)
{
	dwt_stop_time = timing_counter_get();
	timing_stop();
	dwt_total_cycles += timing_cycles_get(&dwt_start_time, &dwt_stop_time);
}

void TST_DWTlogCycles(const char *const pszName, int16_t i16Count, const char *pszUnits)
{
	printk("DWT %s: %lld cycles (%d %s)\n", pszName, dwt_total_cycles, i16Count, pszUnits);
}