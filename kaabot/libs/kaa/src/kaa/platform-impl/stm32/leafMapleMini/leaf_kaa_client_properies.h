/*
 * Copyright 2015 CyberVision, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * leaf_kaa_client_properies.h
 *
 *  Created on: Apr 16, 2015
 *      Author: Andriy Panasenko <apanasenko@cybervisiontech.com>
 */

#ifndef LEAF_KAA_CLIENT_PROPERIES_H_
#define LEAF_KAA_CLIENT_PROPERIES_H_

typedef struct {
    unsigned long max_update_time;
    esp8266_serial_t *serial;
    const char *wifi_ssid;
    const char *wifi_pswd;
    char *kaa_public_key;
    size_t kaa_public_key_length;
} kaa_client_props_t;

#endif /* LEAF_KAA_CLIENT_PROPERIES_H_ */
