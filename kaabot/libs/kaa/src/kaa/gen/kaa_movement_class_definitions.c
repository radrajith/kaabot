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

# include <inttypes.h>
# include <string.h>
# include "../platform/stdio.h"
# include "kaa_movement_class_definitions.h"
# include "../avro_src/avro/io.h"
# include "../avro_src/encoding.h"
# include "../utilities/kaa_mem.h"

/*
 * AUTO-GENERATED CODE
 */



static void kaa_movement_class_movement_direction_serialize(avro_writer_t writer, void *data)
{
    if (data) {
        kaa_movement_class_movement_direction_t *record = (kaa_movement_class_movement_direction_t *)data;

        kaa_enum_serialize(writer, &record->direction);
    }
}

static size_t kaa_movement_class_movement_direction_get_size(void *data)
{
    if (data) {
        size_t record_size = 0;
        kaa_movement_class_movement_direction_t *record = (kaa_movement_class_movement_direction_t *)data;

        record_size += kaa_enum_get_size(&record->direction);

        return record_size;
    }

    return 0;
}

kaa_movement_class_movement_direction_t *kaa_movement_class_movement_direction_create()
{
    kaa_movement_class_movement_direction_t *record = 
            (kaa_movement_class_movement_direction_t *)KAA_CALLOC(1, sizeof(kaa_movement_class_movement_direction_t));

    if (record) {
        record->serialize = kaa_movement_class_movement_direction_serialize;
        record->get_size = kaa_movement_class_movement_direction_get_size;
        record->destroy = kaa_data_destroy;
    }

    return record;
}

kaa_movement_class_movement_direction_t *kaa_movement_class_movement_direction_deserialize(avro_reader_t reader)
{
    kaa_movement_class_movement_direction_t *record = 
            (kaa_movement_class_movement_direction_t *)KAA_MALLOC(sizeof(kaa_movement_class_movement_direction_t));

    if (record) {
        record->serialize = kaa_movement_class_movement_direction_serialize;
        record->get_size = kaa_movement_class_movement_direction_get_size;
        record->destroy = kaa_data_destroy;

        int64_t direction_value;
        avro_binary_encoding.read_long(reader, &direction_value);
        record->direction = direction_value;
    }

    return record;
}

