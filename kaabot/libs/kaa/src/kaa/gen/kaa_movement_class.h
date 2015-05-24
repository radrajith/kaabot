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
 * @brief Listener of movement_direction events.
 */
typedef void (* on_kaa_movement_class_movement_direction)(void *context, kaa_movement_class_movement_direction_t *event, kaa_endpoint_id_p source);


/**
 * @brief Set listener for movement_direction events.
 * 
 * @param[in]       self        Valid pointer to event manager.
 * @param[in]       listener    Listener callback.
 * @param[in]       context     Listener's context.
 * @return Error code.
 */
kaa_error_t kaa_event_manager_set_kaa_movement_class_movement_direction_listener(kaa_event_manager_t *self, on_kaa_movement_class_movement_direction listener, void *context);



# ifdef __cplusplus
}      /* extern "C" */
# endif

# endif // KAA_MOVEMENT_CLASS_H