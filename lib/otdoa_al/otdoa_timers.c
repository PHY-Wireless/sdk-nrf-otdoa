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

static timing_t starts[10], stops[10];
static uint64_t totals[10];

// Initialize the Timer & cycle counter
void TST_initTimerAndCycleCnt(void)
{
	timing_init();
	timing_start();
    memset(starts, 0, sizeof starts);
    memset(stops, 0, sizeof stops);
    memset(totals, 0, sizeof totals);
}

void TST_timerStart(const unsigned int timer)
{
    if (timer >= 10) {
        printk("Invalid timer: %u\n", timer);
        return;
    }
    totals[timer] = 0;
	starts[timer] = timing_counter_get();
}

void TST_timerStop(const unsigned int timer)
{
    if (timer >= 10) {
        printk("Invalid timer: %u\n", timer);
        return;
    }
    stops[timer] = timing_counter_get();
	totals[timer] += timing_cycles_get(&starts[timer], &stops[timer]);
}

// log the count of cycles
void TST_logCycles( const char* const pszName, int16_t i16Count, const char* pszUnits, const unsigned int timer)
{
    if (timer >= 10) {
        printk("Invalid timer: %u\n", timer);
        return;
    }
    printk("Timer %s: %lld cycles (%d %s)\n", pszName, totals[timer], i16Count, pszUnits);
}

// Pause the timer and accumulate cycles
void TST_pauseTimer(const unsigned int timer)
{
    if (timer >= 10) {
        printk("Invalid timer: %u\n", timer);
        return;
    }
    stops[timer] = timing_counter_get();
	totals[timer] += timing_cycles_get(&starts[timer], &stops[timer]);
	starts[timer] = stops[timer];
}

void TST_resumeTimer(const unsigned int timer) {
    if (timer >= 10) {
        printk("Invalid timer: %u\n", timer);
        return;
    }
	starts[timer] = timing_counter_get();
}