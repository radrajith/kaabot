/*
 * Copyright 2014-2015 CyberVision, Inc.
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

# ifndef KAA_MOVEMENT_CLASS_DEFINITIONS_H_
# define KAA_MOVEMENT_CLASS_DEFINITIONS_H_

# include "../kaa_common_schema.h"
# include "../collections/kaa_list.h"

# ifdef __cplusplus
extern "C" {
# endif



typedef enum {
    ENUM_DIRECTIONT_BOT_BACKWARD,
    ENUM_DIRECTIONT_BOT_FORWARD,
    ENUM_DIRECTIONT_BOT_LEFT,
    ENUM_DIRECTIONT_BOT_RIGHT,
    ENUM_DIRECTIONT_BOT_STOP,
} kaa_movement_class_directiont_t;

#ifdef GENC_ENUM_DEFINE_ALIASES
#define BOT_BACKWARD ENUM_DIRECTIONT_BOT_BACKWARD
#define BOT_FORWARD ENUM_DIRECTIONT_BOT_FORWARD
#define BOT_LEFT ENUM_DIRECTIONT_BOT_LEFT
#define BOT_RIGHT ENUM_DIRECTIONT_BOT_RIGHT
#define BOT_STOP ENUM_DIRECTIONT_BOT_STOP
# endif // GENC_ENUM_DEFINE_ALIASES

#ifdef GENC_ENUM_STRING_LITERALS
const char* KAA_MOVEMENT_CLASS_DIRECTIONT_SYMBOLS[5] = {
    "BOT_BACKWARD",
    "BOT_FORWARD",
    "BOT_LEFT",
    "BOT_RIGHT",
    "BOT_STOP"};
# endif // GENC_ENUM_STRING_LITERALS


typedef struct {
    kaa_movement_class_directiont_t direction;

    serialize_fn serialize;
    get_size_fn  get_size;
    destroy_fn   destroy;
} kaa_movement_class_movement_direction_t;

kaa_movement_class_movement_direction_t *kaa_movement_class_movement_direction_create();
kaa_movement_class_movement_direction_t *kaa_movement_class_movement_direction_deserialize(avro_reader_t reader);

#ifdef __cplusplus
}      /* extern "C" */
#endif
#endif