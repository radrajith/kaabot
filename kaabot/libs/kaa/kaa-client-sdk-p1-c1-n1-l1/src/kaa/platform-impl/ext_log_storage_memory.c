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

#ifndef KAA_DISABLE_FEATURE_LOGGING

#include "../platform/platform.h"

#include "../platform/ext_log_storage.h"

#include "../collections/kaa_list.h"
#include "../utilities/kaa_mem.h"
#include "../utilities/kaa_log.h"



typedef struct {
    char       *data;       /**< Serialized data */
    size_t      size;       /**< Size of data */
    uint16_t    bucket_id;  /**< Bucket ID */
} ext_log_record_t;

typedef struct {
    kaa_list_t     *logs;                  /**< List of @link ext_log_record_t @endlink */
    kaa_list_t     *last_log_it;           /**< Point to the last record in the storage */
    kaa_list_t     *first_unmarked;        /**< Pointer to the first unmarked record position (with zero bucket_id) */
    size_t          max_storage_size;      /**< Max size of the log storage */
    size_t          total_occupied_size;   /**< Volume occupied by all logs */
    size_t          unmarked_occupied_size;/**< Volume occupied by unmarked logs */
    size_t          unmarked_record_count; /**< Number of unmarked logs */
    size_t          force_removal_to_size; /**< Percent of elder logs to delete in case max log storage size will be exceeded. */
    kaa_logger_t   *logger;                /**< Logger instance */
} ext_log_storage_memory_t;



/**
 * @brief Creates the size-unlimited instance of the memory log storage.
 *
 * @param[out]    log_storage_context_p    The pointer to the new storage instance.
 * @param[in]     logger                   The logger.
 *
 * @return    Error code.
 */
kaa_error_t ext_unlimited_log_storage_create(void **log_storage_context_p, kaa_logger_t *logger);



/**
 * @brief Creates the size-limited instance of the memory log storage.
 *
 * @param[out]    log_storage_context_p    The pointer to the new storage instance.
 * @param[in]     logger                   The logger.
 * @param[in]     max_storage_size             The maximum storage size.
 * @param[in]     percent_to_delete        The percentage of elder logs to delete if the maximum storage size.
 *
 * @return    Error code.
 */
kaa_error_t ext_limited_log_storage_create(void **log_storage_context_p
                                         , kaa_logger_t *logger
                                         , size_t storage_size
                                         , size_t percent_to_delete);



/**
 * @brief Destroys the instance of the memory log storage.
 *
 * @param[in]   context The log storage context.
 * @return    Error code.
 */
kaa_error_t ext_log_storage_destroy(void *context);



static void log_record_destroy(void *record_p)
{
    if (record_p) {
        KAA_FREE(((ext_log_record_t*)record_p)->data);
        KAA_FREE(record_p);
    }
}



kaa_error_t ext_unlimited_log_storage_create(void **log_storage_context_p, kaa_logger_t *logger)
{
    KAA_RETURN_IF_NIL2(log_storage_context_p, logger, KAA_ERR_BADPARAM);

    ext_log_storage_memory_t *log_storage = (ext_log_storage_memory_t *) KAA_MALLOC(sizeof(ext_log_storage_memory_t));
    KAA_RETURN_IF_NIL(log_storage, KAA_ERR_NOMEM);

    log_storage->logger                 = logger;
    log_storage->logs                   = NULL;
    log_storage->last_log_it            = NULL;
    log_storage->first_unmarked         = NULL;
    log_storage->max_storage_size       = 0;
    log_storage->total_occupied_size    = 0;
    log_storage->unmarked_occupied_size = 0;
    log_storage->unmarked_record_count  = 0;
    log_storage->force_removal_to_size  = 0;

    *log_storage_context_p = (void *)log_storage;
    return KAA_ERR_NONE;
}



kaa_error_t ext_limited_log_storage_create(void **log_storage_context_p
                                         , kaa_logger_t *logger
                                         , size_t storage_size
                                         , size_t percent_to_delete)
{
    KAA_RETURN_IF_NIL4(log_storage_context_p, logger, storage_size, percent_to_delete, KAA_ERR_BADPARAM);

    if (percent_to_delete > 100) {
        KAA_LOG_WARN(logger, KAA_ERR_BADPARAM, "Failed to create log storage: percentage of logs "
                                                    "to remove is more than 100%% (%u%%)", percent_to_delete);
        return KAA_ERR_BADPARAM;
    }

    kaa_error_t error_code = ext_unlimited_log_storage_create(log_storage_context_p, logger);
    KAA_RETURN_IF_ERR(error_code);

    ext_log_storage_memory_t *log_storage = (ext_log_storage_memory_t *)*log_storage_context_p;

    log_storage->max_storage_size      = storage_size;
    log_storage->force_removal_to_size = (log_storage->max_storage_size * (100 - percent_to_delete)) / 100;

    return KAA_ERR_NONE;
}



kaa_error_t ext_log_storage_allocate_log_record_buffer(void *context, kaa_log_record_t *record)
{
    KAA_RETURN_IF_NIL2(record, record->size, KAA_ERR_BADPARAM);

    record->data = (char *) KAA_MALLOC(record->size * sizeof(char));
    if (!record->data)
        return KAA_ERR_NOMEM;

    return KAA_ERR_NONE;
}



static kaa_error_t shrink_to_size(void *context, size_t size)
{
    KAA_RETURN_IF_NIL(context, KAA_ERR_BADPARAM);
    ext_log_storage_memory_t *self = (ext_log_storage_memory_t *)context;
    size_t removed_record_count = 0;

    while (self->total_occupied_size > size && self->logs) {
        // May delete records already marked with bucket_id. C'est la vie...
        ext_log_record_t *log_record = (ext_log_record_t *)kaa_list_get_data(self->logs);

        self->total_occupied_size -= log_record->size;
        ++removed_record_count;

        if (!log_record->bucket_id) {
            self->unmarked_occupied_size -= log_record->size;
            self->unmarked_record_count--;
            self->first_unmarked = kaa_list_next(self->logs);
        }

        kaa_list_remove_at(&self->logs, self->logs, &log_record_destroy);
    }

    if (!self->logs) {
        self->first_unmarked = NULL;
        self->last_log_it = NULL;
    }

    KAA_LOG_INFO(self->logger, KAA_ERR_NONE, "%zu records forcibly removed", removed_record_count);

    return KAA_ERR_NONE;
}



kaa_error_t ext_log_storage_add_log_record(void *context, kaa_log_record_t *record)
{
    KAA_RETURN_IF_NIL2(context, record, KAA_ERR_BADPARAM);
    ext_log_storage_memory_t *self = (ext_log_storage_memory_t *)context;

    if (self->max_storage_size && (self->total_occupied_size + record->size) > self->max_storage_size) {
        KAA_LOG_INFO(self->logger, KAA_ERR_NONE, "Log storage is full (occupied %zu, max %zu, record size %zu). "
                            "Going to delete elder logs", self->total_occupied_size, self->max_storage_size, record->size);

        shrink_to_size(self, self->force_removal_to_size);
    }

    ext_log_record_t *new_record = (ext_log_record_t *) KAA_MALLOC(sizeof(ext_log_record_t));
    KAA_RETURN_IF_NIL(new_record, KAA_ERR_NOMEM);

    new_record->data = record->data;
    new_record->size = record->size;
    new_record->bucket_id = 0;

    kaa_list_t *it = NULL;

    if (self->last_log_it) {
        it = kaa_list_insert_after(self->last_log_it, new_record);
    } else if (self->logs) {
        it = kaa_list_push_back(self->logs, new_record);
    } else {
        self->logs = it = kaa_list_create(new_record);
    }

    if (!it) {
        KAA_FREE(new_record);
        return KAA_ERR_NOMEM;
    }

    self->last_log_it = it;
    self->total_occupied_size += new_record->size;
    self->unmarked_occupied_size += new_record->size;
    self->unmarked_record_count++;

    record->data = NULL;
    record->size = 0;

    return KAA_ERR_NONE;
}



kaa_error_t ext_log_storage_deallocate_log_record_buffer(void *context, kaa_log_record_t *record)
{
    KAA_RETURN_IF_NIL2(record, record->data, KAA_ERR_BADPARAM);

    KAA_FREE(record->data);
    record->data = NULL;
    return KAA_ERR_NONE;
}



bool find_by_bucket_id(void *log_record_p, void *bucket_id_p)
{
    return ((ext_log_record_t *)log_record_p)->bucket_id == *((uint16_t *)bucket_id_p);
}



kaa_error_t ext_log_storage_write_next_record(void *context
                                            , char *buffer
                                            , size_t buffer_len
                                            , uint16_t bucket_id
                                            , size_t *record_len)
{
    KAA_RETURN_IF_NIL5(context, buffer, buffer_len, bucket_id, record_len, KAA_ERR_BADPARAM);
    ext_log_storage_memory_t *self = (ext_log_storage_memory_t *)context;

    uint16_t zero_bucket_id = 0;
    kaa_list_t *record_position = kaa_list_find_next(self->first_unmarked ? self->first_unmarked : self->logs
                                                   , &find_by_bucket_id
                                                   , &zero_bucket_id);
    if (!record_position) {
        *record_len = 0;
        return KAA_ERR_NOT_FOUND;
    }

    ext_log_record_t *record = (ext_log_record_t *) kaa_list_get_data(record_position);
    *record_len = record->size;
    if (*record_len > buffer_len)
        return KAA_ERR_INSUFFICIENT_BUFFER;

    memcpy((void *)buffer, record->data, record->size);
    record->bucket_id = bucket_id;

    self->first_unmarked = kaa_list_next(record_position);
    self->unmarked_record_count--;
    self->unmarked_occupied_size -= record->size;

    return KAA_ERR_NONE;
}



kaa_error_t ext_log_storage_remove_by_bucket_id(void *context, uint16_t bucket_id)
{
    KAA_RETURN_IF_NIL(context, KAA_ERR_BADPARAM);
    ext_log_storage_memory_t *self = (ext_log_storage_memory_t *)context;

    kaa_list_t *record_position = kaa_list_find_next(self->logs, find_by_bucket_id, &bucket_id);
    if (!record_position)
        return KAA_ERR_NOT_FOUND;

    while (record_position) {
        self->total_occupied_size -= ((ext_log_record_t *)kaa_list_get_data(record_position))->size;

        record_position = kaa_list_remove_at(&self->logs, record_position, &log_record_destroy);
        record_position = kaa_list_find_next(record_position, &find_by_bucket_id, &bucket_id);
    }

    // It's need to add a new element in the right place.
    self->last_log_it = NULL;

    if (!self->logs) {
        self->first_unmarked = NULL;
    }

    return KAA_ERR_NONE;
}



kaa_error_t ext_log_storage_unmark_by_bucket_id(void *context, uint16_t bucket_id)
{
    KAA_RETURN_IF_NIL(context, KAA_ERR_BADPARAM);
    ext_log_storage_memory_t *self = (ext_log_storage_memory_t *)context;

    kaa_list_t *record_position = kaa_list_find_next(self->logs, find_by_bucket_id, &bucket_id);
    if (!record_position)
        return KAA_ERR_NOT_FOUND;

    while (record_position) {
        ext_log_record_t *log_record = (ext_log_record_t *)kaa_list_get_data(record_position);

        log_record->bucket_id = 0;
        self->unmarked_record_count++;
        self->unmarked_occupied_size += log_record->size;

        record_position = kaa_list_find_next(record_position, &find_by_bucket_id, &bucket_id);
    }

    self->first_unmarked = NULL;

    return KAA_ERR_NONE;
}



size_t ext_log_storage_get_total_size(const void *context)
{
    KAA_RETURN_IF_NIL(context, 0);
    return ((ext_log_storage_memory_t *)context)->unmarked_occupied_size;
}



size_t ext_log_storage_get_records_count(const void *context)
{
    KAA_RETURN_IF_NIL(context, 0);
    return ((ext_log_storage_memory_t *)context)->unmarked_record_count;
}



kaa_error_t ext_log_storage_destroy(void *context)
{
    KAA_RETURN_IF_NIL(context, KAA_ERR_BADPARAM);
    ext_log_storage_memory_t *self = (ext_log_storage_memory_t *)context;
    if (self) {
        kaa_list_destroy(self->logs, &log_record_destroy);
        KAA_FREE(self);
    }
    return KAA_ERR_NONE;
}

#endif
