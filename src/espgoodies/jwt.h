
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

#ifndef _ESPGOODIES_JWT_H
#define _ESPGOODIES_JWT_H


#include <c_types.h>

#include "json.h"


#define JWT_ALG_HS256           1


typedef struct {

    uint8           alg;
    json_t        * claims;

} jwt_t;


ICACHE_FLASH_ATTR jwt_t       * jwt_new(uint8 alg, json_t *claims);
ICACHE_FLASH_ATTR void          jwt_free(jwt_t *jwt);

ICACHE_FLASH_ATTR char        * jwt_dump(jwt_t *jwt, char *secret);
ICACHE_FLASH_ATTR jwt_t       * jwt_parse(char *jwt_str);
ICACHE_FLASH_ATTR bool          jwt_validate(char *jwt_str, uint8 alg, char *secret);


#endif /* _ESPGOODIES_JWT_H */
