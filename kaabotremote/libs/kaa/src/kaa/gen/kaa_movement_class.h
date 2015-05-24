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


# ifndef KAA_MOVEMENT_CLASS_H
# define KAA_MOVEMENT_CLASS_H

# include "kaa_movement_class_definitions.h" 
# include "../kaa_event.h"
# include "../kaa_error.h"

# ifdef __cplusplus
extern "C" {
# endif




/**
 * @brief Send event of type movement_direction.
 * 
 * @param[in]       self        Valid pointer to event manager.
 * @param[in]       event       Pointer to the event object.
 * @param[in]       target      The target endpoint of the event (null-terminated string). The size of
 *                              the target parameter should be equal to @link KAA_ENDPOINT_ID_LENGTH @endlink .
 *                              If @code NULL @endcode event will be broadcasted.
 * 
 * @return Error code.
 */
kaa_error_t kaa_event_manager_send_kaa_movement_class_movement_direction(kaa_event_manager_t *self, kaa_movement_class_movement_direction_t *event, kaa_endpoint_id_p target);


/**
 * @brief Add event of type movement_direction to the events block.
 * 
 * @param[in]       self        Valid pointer to event manager.
 * @param[in]       event       Pointer to the event object.
 * @param[in]       target      Target of the event (null-terminated string). If NULL - event will be broadcasted.
 * @param[in]       trx_id      Event block id.
 *
 * @return Error code.
 */
kaa_error_t kaa_event_manager_add_kaa_movement_class_movement_direction_event_to_block(kaa_event_manager_t *self, kaa_movement_class_movement_direction_t *event, kaa_endpoint_id_p target, kaa_event_block_id trx_id);

# ifdef __cplusplus
}      /* extern "C" */
# endif

# endif // KAA_MOVEMENT_CLASS_H