
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

#ifndef _ESPGOODIES_RTC_H
#define _ESPGOODIES_RTC_H

#include <c_types.h>


#ifdef _DEBUG_RTC
#define DEBUG_RTC(fmt, ...)             DEBUG("[rtc           ] " fmt, ##__VA_ARGS__)
#else
#define DEBUG_RTC(...)                  {}
#endif


ICACHE_FLASH_ATTR void                  rtc_init(void);
ICACHE_FLASH_ATTR void                  rtc_reset(void);

ICACHE_FLASH_ATTR bool                  rtc_is_full_boot(void);
ICACHE_FLASH_ATTR uint32                rtc_get_boot_count(void);

ICACHE_FLASH_ATTR uint32                rtc_get_value(uint8 addr);
ICACHE_FLASH_ATTR bool                  rtc_set_value(uint8 addr, uint32 value);


#endif /* _ESPGOODIES_RTC_H */
