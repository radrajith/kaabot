/*
 * Copyright 2014 CyberVision, Inc.
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

# include <stdint.h>
# include "kaa_movement_class.h"

# include "../avro_src/avro/io.h"

# include "../kaa_common.h"
# include "../kaa_event.h"
# include "../kaa_error.h"
# include "../utilities/kaa_mem.h"

extern kaa_error_t kaa_event_manager_add_event_to_transaction(kaa_event_manager_t *self, kaa_event_block_id trx_id, const char *fqn, const char *event_data, size_t event_data_size, kaa_endpoint_id_p target);
extern kaa_error_t kaa_event_manager_add_on_event_callback(kaa_event_manager_t *self, const char *fqn, kaa_event_callback_t callback);
extern kaa_error_t kaa_event_manager_send_event(kaa_event_manager_t *self, const char *fqn, const char *event_data, size_t event_data_size, kaa_endpoint_id_p target);
# ifdef kaa_broadcast_event
# undef kaa_broadcast_event
# endif
# define kaa_broadcast_event(context, fqn, fqn_length, event_data, event_data_size) \
    kaa_event_manager_send_event((context), (fqn), (fqn_length), (event_data), (event_data_size), NULL, 0)


typedef struct kaa_movement_class_ {

    on_kaa_movement_class_movement_direction movement_direction_listener;
    void * movement_direction_context;

    unsigned char is_movement_direction_callback_added;

} kaa_movement_class;

static kaa_movement_class listeners = {

    NULL, NULL,

    0

};

static void kaa_event_manager_movement_direction_listener(const char * event_fqn, const char *data, size_t size, kaa_endpoint_id_p event_source)
{
    (void)event_fqn;
    if (listeners.movement_direction_listener) {
        avro_reader_t reader = avro_reader_memory(data, size);
        kaa_movement_class_movement_direction_t * event = kaa_movement_class_movement_direction_deserialize(reader);
        avro_reader_free(reader);
        listeners.movement_direction_listener(listeners.movement_direction_context, event, event_source);
    }
}

kaa_error_t kaa_event_manager_set_kaa_movement_class_movement_direction_listener(kaa_event_manager_t *self, on_kaa_movement_class_movement_direction listener, void *context)
{
    KAA_RETURN_IF_NIL(self, KAA_ERR_BADPARAM);
    listeners.movement_direction_listener = listener;
    listeners.movement_direction_context = context;
    if (!listeners.is_movement_direction_callback_added) {
        listeners.is_movement_direction_callback_added = 1;
        return kaa_event_manager_add_on_event_callback(self, "com.krishna.kaabot.movementDirection", kaa_event_manager_movement_direction_listener);
    }
    return KAA_ERR_NONE;
}



