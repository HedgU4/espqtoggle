
/*
 * Copyright 2019 The qToggle Team
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <user_interface.h>

#include "common.h"
#include "rtc.h"
#include "sleep.h"

#ifdef _OTA
#include "ota.h"
#endif


#define WAKE_SHORT_DURATION     60      /* seconds */
#define MAX_SLEEP_DURATION      1800    /* seconds */
#define RTC_SLEEP_COUNT_ADDR    130     /* 130 * 4 bytes = 520 */
#define RTC_SLEEP_REM_ADDR      134     /* 131 * 4 bytes = 524 */


static uint16                   wake_interval = 0;  /* minutes, 0 - sleep disabled */
static uint16                   wake_duration = 0;  /* seconds */
static os_timer_t               sleep_timer;
static bool                     about_to_sleep = FALSE;


ICACHE_FLASH_ATTR static void   on_sleep(void *arg);


void sleep_init(void) {
    /* we might still have a few rounds to sleep,
     * as part of the long sleep mechanism */

    uint32 sleep_count = rtc_get_value(RTC_SLEEP_COUNT_ADDR);
    uint32 sleep_rem = rtc_get_value(RTC_SLEEP_REM_ADDR);

    DEBUG_SLEEP("sleep_count = %d, sleep_rem = %d", sleep_count, sleep_rem);

    if (sleep_count > 0) {
        DEBUG_SLEEP("sleeping for another %d seconds", MAX_SLEEP_DURATION);
        rtc_set_value(RTC_SLEEP_COUNT_ADDR, sleep_count - 1);
        system_deep_sleep_instant(MAX_SLEEP_DURATION * 1000 * 1000);
        return;
    }

    if (sleep_rem > 0) {
        DEBUG_SLEEP("sleeping for another %d seconds", sleep_rem);
        rtc_set_value(RTC_SLEEP_REM_ADDR, 0);
        system_deep_sleep_instant(sleep_rem * 1000 * 1000);
        return;
    }
}

void sleep_reset(void) {
    os_timer_disarm(&sleep_timer);

    if (!wake_interval) {
        DEBUG_SLEEP("sleep mode disabled");
        return;
    }

    os_timer_setfn(&sleep_timer, on_sleep, NULL);
    os_timer_arm(&sleep_timer, wake_duration * 1000, /* repeat = */ TRUE /* just in case... */);

    DEBUG_SLEEP("will go to sleep in %d seconds", wake_duration);
}

bool sleep_pending(void) {
    return about_to_sleep;
}

bool sleep_is_short_wake(void) {
    return (wake_interval > 0) && (wake_duration <= WAKE_SHORT_DURATION);
}

int sleep_get_wake_interval(void) {
    return wake_interval;
}

void sleep_set_wake_interval(int interval) {
    wake_interval = interval;
    DEBUG_SLEEP("wake interval set to %d minutes", interval);
}

int sleep_get_wake_duration(void) {
    return wake_duration;
}

void sleep_set_wake_duration(int duration) {
    wake_duration = duration;
    DEBUG_SLEEP("wake duration set to %d seconds", duration);
}


void on_sleep(void *arg) {
#ifdef _OTA
     if (ota_busy()) {
         DEBUG_SLEEP("refusing to sleep while upgrading");
         return;
     }
#endif

    uint32 wake_interval_seconds = wake_interval * 60;
    uint32 sleep_count = wake_interval_seconds / MAX_SLEEP_DURATION;
    uint32 sleep_rem = wake_interval_seconds % MAX_SLEEP_DURATION;

    DEBUG_SLEEP("about to sleep for a total of %d seconds (count = %d, rem = %d)",
                wake_interval_seconds, sleep_count, sleep_rem);
    about_to_sleep = TRUE;

    if (sleep_count > 0) {
        rtc_set_value(RTC_SLEEP_COUNT_ADDR, sleep_count - 1);
        rtc_set_value(RTC_SLEEP_REM_ADDR, sleep_rem);
        DEBUG_SLEEP("sleeping for %d seconds", MAX_SLEEP_DURATION);
        system_deep_sleep(MAX_SLEEP_DURATION * 1000 * 1000);
    }
    else {
        rtc_set_value(RTC_SLEEP_COUNT_ADDR, 0);
        rtc_set_value(RTC_SLEEP_REM_ADDR, 0);
        DEBUG_SLEEP("sleeping for %d seconds", sleep_rem);
        system_deep_sleep(sleep_rem * 1000 * 1000);
    }
}
