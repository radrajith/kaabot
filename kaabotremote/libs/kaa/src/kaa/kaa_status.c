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
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "platform/stdio.h"
#include "platform/ext_sha.h"
#include "platform/ext_status.h"
#include "kaa_status.h"
#include "kaa_common.h"
#include "utilities/kaa_mem.h"
#include <string.h>


#define KAA_STATUS_STATIC_SIZE      (sizeof(bool) + sizeof(bool) + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint16_t) + SHA_1_DIGEST_LENGTH * sizeof(char) * 2)

#define READ_BUFFER(FROM, TO, SIZE) \
        memcpy(TO, FROM, SIZE); \
        FROM += SIZE;

#define WRITE_BUFFER(FROM, TO, SIZE) \
        memcpy(TO, FROM, SIZE); \
        TO += SIZE;

kaa_error_t kaa_status_create(kaa_status_t ** kaa_status_p)
{
    KAA_RETURN_IF_NIL(kaa_status_p, KAA_ERR_BADPARAM);

    kaa_status_t * kaa_status = (kaa_status_t *) KAA_MALLOC(sizeof(kaa_status_t));
    KAA_RETURN_IF_NIL(kaa_status, KAA_ERR_NOMEM);

    kaa_status->is_registered = false;
    kaa_status->is_attached = false;
    kaa_status->event_seq_n = 0;
    kaa_status->config_seq_n = 0;
    kaa_status->log_bucket_id = 0;
    memset(kaa_status->endpoint_public_key_hash, 0, SHA_1_DIGEST_LENGTH);
    memset(kaa_status->profile_hash, 0, SHA_1_DIGEST_LENGTH);
    kaa_status->endpoint_access_token = NULL;

    char *  read_buf = NULL;
    char *  read_buf_head = NULL;
    size_t  read_size = 0;
    bool    needs_deallocation = false;
    ext_status_read(&read_buf, &read_size, &needs_deallocation);
    read_buf_head = read_buf;
    if (read_size >= KAA_STATUS_STATIC_SIZE + sizeof(size_t)) {
        READ_BUFFER(read_buf, &kaa_status->is_registered, sizeof(kaa_status->is_registered))
        READ_BUFFER(read_buf, &kaa_status->is_attached, sizeof(kaa_status->is_attached))
        READ_BUFFER(read_buf, &kaa_status->event_seq_n, sizeof(kaa_status->event_seq_n))
        READ_BUFFER(read_buf, &kaa_status->config_seq_n, sizeof(kaa_status->config_seq_n))
        READ_BUFFER(read_buf, &kaa_status->log_bucket_id, sizeof(kaa_status->log_bucket_id))
        READ_BUFFER(read_buf, kaa_status->endpoint_public_key_hash, SHA_1_DIGEST_LENGTH)
        READ_BUFFER(read_buf, kaa_status->profile_hash, SHA_1_DIGEST_LENGTH)

        size_t enpoint_access_token_length = 0;
        READ_BUFFER(read_buf, &enpoint_access_token_length, sizeof(enpoint_access_token_length))

        if (enpoint_access_token_length > 0) {
            kaa_status->endpoint_access_token = (char * ) KAA_MALLOC((enpoint_access_token_length + 1) * sizeof(char));
            if (!kaa_status->endpoint_access_token) {
                KAA_FREE(kaa_status);
                return KAA_ERR_NOMEM;
            }
            READ_BUFFER(read_buf, kaa_status->endpoint_access_token, enpoint_access_token_length);
            kaa_status->endpoint_access_token[enpoint_access_token_length] = '\0';
        }
    }

    if (needs_deallocation)
        KAA_FREE(read_buf_head);

    *kaa_status_p = kaa_status;
    return KAA_ERR_NONE;
}

void kaa_status_destroy(kaa_status_t *self)
{
    if (self) {
        KAA_FREE(self->endpoint_access_token);
        KAA_FREE(self);
    }
}

kaa_error_t kaa_status_set_endpoint_access_token(kaa_status_t * self, const char *token)
{
    KAA_RETURN_IF_NIL(self, KAA_ERR_BADPARAM);

    if (self->endpoint_access_token)
        KAA_FREE(self->endpoint_access_token);

    size_t len = strlen(token);
    self->endpoint_access_token = (char *) KAA_MALLOC((len + 1) * sizeof(char));
    if (!self->endpoint_access_token)
        return KAA_ERR_NOMEM;
    strcpy(self->endpoint_access_token, token);
    return KAA_ERR_NONE;
}

kaa_error_t kaa_status_save(kaa_status_t *self)
{
    KAA_RETURN_IF_NIL(self, KAA_ERR_BADPARAM);

    size_t endpoint_access_token_length = self->endpoint_access_token ? strlen(self->endpoint_access_token) : 0;
    size_t buffer_size = KAA_STATUS_STATIC_SIZE + sizeof(endpoint_access_token_length) + endpoint_access_token_length;

    char *buffer_head = (char *) KAA_MALLOC(buffer_size * sizeof(char));
    KAA_RETURN_IF_NIL(buffer_head, KAA_ERR_NOMEM);

    char *buffer = buffer_head;

    WRITE_BUFFER(&self->is_registered, buffer, sizeof(self->is_registered));
    WRITE_BUFFER(&self->is_attached, buffer, sizeof(self->is_attached));
    WRITE_BUFFER(&self->event_seq_n, buffer, sizeof(self->event_seq_n));
    WRITE_BUFFER(&self->config_seq_n, buffer, sizeof(self->config_seq_n));
    WRITE_BUFFER(&self->log_bucket_id, buffer, sizeof(self->log_bucket_id));
    WRITE_BUFFER(self->endpoint_public_key_hash, buffer, SHA_1_DIGEST_LENGTH);
    WRITE_BUFFER(self->profile_hash, buffer, SHA_1_DIGEST_LENGTH);
    WRITE_BUFFER(&endpoint_access_token_length, buffer, sizeof(endpoint_access_token_length));
    if (endpoint_access_token_length) {
        WRITE_BUFFER(self->endpoint_access_token, buffer, endpoint_access_token_length);
    }

    ext_status_store(buffer_head, buffer_size);

    KAA_FREE(buffer_head);

    return KAA_ERR_NONE;
}
