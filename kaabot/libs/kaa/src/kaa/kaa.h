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

/**
 * @file kaa.h
 * @brief Kaa C endpoint library
 *
 * Supplies API for initializing and de-initializing general Kaa endpoint context.
 */

#ifndef KAA_H_
#define KAA_H_

#include "kaa_context.h"
#include "kaa_error.h"

#ifdef __cplusplus
extern "C" {
#endif



/**
 * @brief Creates and initializes general Kaa endpoint context.
 *
 * The endpoint SDK assumes ownership of the memory allocated for the context and nested data structures.
 *
 * @param[in,out]   kaa_context_p   Pointer to return the address of initialized Kaa endpoint context to.
 *
 * @return Error code.
 */
kaa_error_t kaa_init(kaa_context_t **kaa_context_p);



/**
 * @brief Starts Kaa's workflow.
 *
 * @b NOTE: Should be called after @link kaa_init() @endlink.
 *
 * @param[in]   kaa_context    Pointer to an initialized Kaa endpoint context.
 *
 * @return Error code.
 */
kaa_error_t kaa_start(kaa_context_t *kaa_context);



/**
 * @brief De-initializes and destroys general Kaa endpoint context.
 *
 * After a successful call @c kaa_context pointer becomes invalid.
 *
 * @param[in]       kaa_context     Pointer to an initialized Kaa endpoint context.
 *
 * @return Error code.
 */
kaa_error_t kaa_deinit(kaa_context_t *kaa_context);

#ifdef __cplusplus
}      /* extern "C" */
#endif
#endif /* KAA_H_ */
