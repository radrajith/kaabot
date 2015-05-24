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
#define USE_MRAA

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <execinfo.h>
#include <stddef.h>
#include <sys/select.h>
#include <unistd.h>

#ifdef USE_MRAA
#include <mraa.h>
#endif

#include "kaa/kaa.h"
#include "kaa/kaa_error.h"
#include "kaa/kaa_context.h"
#include "kaa/platform/kaa_client.h"
#include "kaa/utilities/kaa_log.h"
#include "kaa/utilities/kaa_mem.h"
#include "kaa/kaa_user.h"
#include "kaa/kaa_channel_manager.h"
#include "kaa/gen/kaa_movement_class.h"
#include "kaa/gen/kaa_movement_class_definitions.h"

#include <kaa/utilities/kaa_log.h>
#include <kaa/utilities/kaa_mem.h>

#include <kaa/platform/ext_sha.h>
#include <kaa/platform/ext_transport_channel.h>
#include <kaa/platform-impl/kaa_tcp_channel.h>



#define KAA_USER_ID             "user@email.com"
#define KAA_USER_ACCESS_TOKEN   "token"
#define KAA_USER_VERIFIER_TOKEN "16996139062968235708"


#define THERMO_REQUEST_FQN          "org.kaaproject.kaa.schema.sample.event.thermo.ThermostatInfoRequest"
#define CHANGE_DEGREE_REQUEST_FQN   "org.kaaproject.kaa.schema.sample.event.thermo.ChangeDegreeRequest"


static kaa_context_t *kaa_context_ = NULL;

static kaa_service_t BOOTSTRAP_SERVICE[] = { KAA_SERVICE_BOOTSTRAP };
static const int BOOTSTRAP_SERVICE_COUNT = sizeof(BOOTSTRAP_SERVICE) / sizeof(kaa_service_t);

static kaa_service_t OPERATIONS_SERVICES[] = { KAA_SERVICE_PROFILE
                                             , KAA_SERVICE_USER
                                             , KAA_SERVICE_EVENT};
static const int OPERATIONS_SERVICES_COUNT = sizeof(OPERATIONS_SERVICES) / sizeof(kaa_service_t);

static kaa_transport_channel_interface_t bootstrap_channel;
static kaa_transport_channel_interface_t operations_channel;

static bool is_shutdown = false;

#ifdef USE_MRAA
//Pin to connect the servo to edison
//static int left_servo = 6;
//static int right_servo = 9;
static int period_servo = 1000;


void motion_kaabot(float pulseWidthLeft, float pulseWidthRight)
{
    mraa_init();
    //! [Interesting]
    mraa_pwm_context pwm1;
    mraa_pwm_context pwm2;
    pwm1 = mraa_pwm_init(6);
    pwm2 = mraa_pwm_init(9);
    if (pwm1 == NULL||pwm2==NULL)
        return 1;

    mraa_pwm_period_us(pwm1, period_servo);
    mraa_pwm_enable(pwm1, 1);
    mraa_pwm_period_us(pwm2, period_servo);
    mraa_pwm_enable(pwm2, 1);

    //float value1 = pulseWidthLeft;
    //float value2 = pulseWidthRight;
    //value = value + .1f;
    mraa_pwm_write(pwm2, pulseWidthLeft);
    mraa_pwm_write(pwm1, pulseWidthRight);
    // if (value >= 1.0f) {
    //     value = 0.0f;
    // }
    float output1 = mraa_pwm_read(pwm1);
    float output2 = mraa_pwm_read(pwm2);
    //! [Interesting]
    printf("%.6f",output1);
}
#endif //USE_MRAA

void kaa_on_movement_class_direction(void *context, kaa_movement_class_movement_direction_t *event, kaa_endpoint_id_p source)
{
    float pulseWidthLeft = 0.0
        , pulseWidthRight = 0.0;

    switch (event->direction) {
    case ENUM_DIRECTIONT_BOT_BACKWARD:
        KAA_LOG_WARN(kaa_context_->logger, 0, "Backward!");
        pulseWidthLeft = 0.2f;
        pulseWidthRight = 0.9f;
        break;
    case ENUM_DIRECTIONT_BOT_FORWARD:
        KAA_LOG_WARN(kaa_context_->logger, 0, "Forward!");
        pulseWidthLeft = 0.9f;
        pulseWidthRight = 0.2f;
        break;
    case ENUM_DIRECTIONT_BOT_LEFT:
        KAA_LOG_WARN(kaa_context_->logger, 0, "Left!");
        pulseWidthLeft = 0.0f;
        pulseWidthRight = 0.1f;
        break;
    case ENUM_DIRECTIONT_BOT_RIGHT:
        KAA_LOG_WARN(kaa_context_->logger, 0, "Right!");
        pulseWidthLeft = 0.1f;
        pulseWidthRight = 0.0f;
        break;
    case ENUM_DIRECTIONT_BOT_STOP:
        KAA_LOG_WARN(kaa_context_->logger, 0, "Stop!");
        break;
    }
#ifdef USE_MRAA
    motion_kaabot(pulseWidthLeft, pulseWidthRight);
#endif
    event->destroy(event);
}



kaa_error_t kaa_on_event_listeners(void *context, const kaa_endpoint_id listeners[], size_t listeners_count)
{
    return KAA_ERR_NONE;
}


kaa_error_t kaa_on_event_listeners_failed(void *context)
{
    printf("Kaa Demo event listeners not found\n");
    return KAA_ERR_NONE;
}


kaa_error_t kaa_on_attached(void *context, const char *user_external_id, const char *endpoint_access_token)
{
    printf("Kaa Demo attached to user %s, access token %s\n", user_external_id, endpoint_access_token);
    return KAA_ERR_NONE;
}


kaa_error_t kaa_on_detached(void *context, const char *endpoint_access_token)
{
    printf("Kaa Demo detached from user access token %s\n", endpoint_access_token);
    return KAA_ERR_NONE;
}


kaa_error_t kaa_on_attach_success(void *context)
{
    printf("Kaa Demo attach success\n");
    const char *fqns[] = { THERMO_REQUEST_FQN, CHANGE_DEGREE_REQUEST_FQN };
    kaa_event_listeners_callback_t listeners_callback = { NULL, &kaa_on_event_listeners, &kaa_on_event_listeners_failed };
    kaa_error_t error_code = kaa_event_manager_find_event_listeners(kaa_context_->event_manager, fqns, 2, &listeners_callback);
    if (error_code) {
        KAA_LOG_ERROR(kaa_context_->logger, error_code, "Failed to find event listeners");
        return error_code;
    }
    return KAA_ERR_NONE;
}


kaa_error_t kaa_on_attach_failed(void *context, user_verifier_error_code_t error_code, const char *reason)
{
    printf("Kaa Demo attach failed\n");
    is_shutdown = true;
    return KAA_ERR_NONE;
}


/*
 * Initializes Kaa SDK.
 */
kaa_error_t kaa_sdk_init()
{
    printf("Initializing Kaa SDK...\n");
    kaa_error_t error_code = kaa_init(&kaa_context_);
    if (error_code) {
        printf("Error during kaa context creation %d\n", error_code);
        return error_code;
    }

    error_code = kaa_tcp_channel_create(&operations_channel
                                      , kaa_context_->logger
                                      , OPERATIONS_SERVICES
                                      , OPERATIONS_SERVICES_COUNT);
    KAA_RETURN_IF_ERR(error_code);

    error_code = kaa_tcp_channel_create(&bootstrap_channel
                                      , kaa_context_->logger
                                      , BOOTSTRAP_SERVICE
                                      , BOOTSTRAP_SERVICE_COUNT);
    KAA_RETURN_IF_ERR(error_code);

    error_code = kaa_channel_manager_add_transport_channel(kaa_context_->channel_manager
                                                         , &bootstrap_channel
                                                         , NULL);
    KAA_RETURN_IF_ERR(error_code);

    error_code = kaa_channel_manager_add_transport_channel(kaa_context_->channel_manager
                                                         , &operations_channel
                                                         , NULL);
    KAA_RETURN_IF_ERR(error_code);

    kaa_attachment_status_listeners_t listeners = { NULL, &kaa_on_attached, &kaa_on_detached, &kaa_on_attach_success, &kaa_on_attach_failed };
    error_code = kaa_user_manager_set_attachment_listeners(kaa_context_->user_manager, &listeners);
    KAA_RETURN_IF_ERR(error_code);

    error_code = kaa_user_manager_attach_to_user(kaa_context_->user_manager
                                               , KAA_USER_ID
                                               , KAA_USER_ACCESS_TOKEN
                                               , KAA_USER_VERIFIER_TOKEN);
    KAA_RETURN_IF_ERR(error_code);


    // Set motion direction listener
    error_code = kaa_event_manager_set_kaa_movement_class_movement_direction_listener(kaa_context_->event_manager
            , &kaa_on_movement_class_direction, NULL);
    KAA_RETURN_IF_ERR(error_code);

    return KAA_ERR_NONE;
}

/*
 * Kaa demo lifecycle routine.
 */
kaa_error_t kaa_demo_init()
{
    kaa_error_t error_code = kaa_sdk_init();
    if (error_code) {
        printf("Failed to init Kaa SDK. Error code : %d\n", error_code);
        return error_code;
    }
    return KAA_ERR_NONE;
}

void kaa_demo_destroy()
{
    kaa_tcp_channel_disconnect(&operations_channel);
    kaa_deinit(kaa_context_);
}

int kaa_demo_event_loop()
{
    kaa_error_t error_code = kaa_start(kaa_context_);
    if (error_code) {
        printf("Failed to start Kaa workflow\n");
        return -1;
    }

    uint16_t select_timeout;
    error_code = kaa_tcp_channel_get_max_timeout(&operations_channel, &select_timeout);
    if (error_code) {
        printf("Failed to get Operations channel keepalive timeout\n");
        return -1;
    }

    if (select_timeout > 3) {
        select_timeout = 3;
    }

    fd_set read_fds, write_fds, except_fds;
    int ops_fd = 0, bootstrap_fd = 0;
    struct timeval select_tv = { 0, 0 };
    int max_fd = 0;

    while (!is_shutdown) {
        FD_ZERO(&read_fds);
        FD_ZERO(&write_fds);
        FD_ZERO(&except_fds);

        max_fd = 0;

        kaa_tcp_channel_get_descriptor(&operations_channel, &ops_fd);
        if (max_fd < ops_fd)
            max_fd = ops_fd;
        kaa_tcp_channel_get_descriptor(&bootstrap_channel, &bootstrap_fd);
        if (max_fd < bootstrap_fd)
            max_fd = bootstrap_fd;

        if (kaa_tcp_channel_is_ready(&operations_channel, FD_READ))
            FD_SET(ops_fd, &read_fds);
        if (kaa_tcp_channel_is_ready(&operations_channel, FD_WRITE))
            FD_SET(ops_fd, &write_fds);

        if (kaa_tcp_channel_is_ready(&bootstrap_channel, FD_READ))
            FD_SET(bootstrap_fd, &read_fds);
        if (kaa_tcp_channel_is_ready(&bootstrap_channel, FD_WRITE))
            FD_SET(bootstrap_fd, &write_fds);

        select_tv.tv_sec = select_timeout;
        select_tv.tv_usec = 0;

        int poll_result = select(max_fd + 1, &read_fds, &write_fds, NULL, &select_tv);
        if (poll_result == 0) {
            kaa_tcp_channel_check_keepalive(&operations_channel);
            kaa_tcp_channel_check_keepalive(&bootstrap_channel);
        } else if (poll_result > 0) {
            if (bootstrap_fd >= 0) {
                if (FD_ISSET(bootstrap_fd, &read_fds)) {
                    KAA_LOG_DEBUG(kaa_context_->logger, KAA_ERR_NONE,"Processing IN event for the Bootstrap client socket %d", bootstrap_fd);
                    error_code = kaa_tcp_channel_process_event(&bootstrap_channel, FD_READ);
                    if (error_code)
                        KAA_LOG_ERROR(kaa_context_->logger, KAA_ERR_NONE,"Failed to process IN event for the Bootstrap client socket %d", bootstrap_fd);
                }
                if (FD_ISSET(bootstrap_fd, &write_fds)) {
                    KAA_LOG_DEBUG(kaa_context_->logger, KAA_ERR_NONE,"Processing OUT event for the Bootstrap client socket %d", bootstrap_fd);
                    error_code = kaa_tcp_channel_process_event(&bootstrap_channel, FD_WRITE);
                    if (error_code)
                        KAA_LOG_ERROR(kaa_context_->logger, error_code,"Failed to process OUT event for the Bootstrap client socket %d", bootstrap_fd);
                }
            }
            if (ops_fd >= 0) {
                if (FD_ISSET(ops_fd, &read_fds)) {
                    KAA_LOG_DEBUG(kaa_context_->logger, KAA_ERR_NONE,"Processing IN event for the Operations client socket %d", ops_fd);
                    error_code = kaa_tcp_channel_process_event(&operations_channel, FD_READ);
                    if (error_code)
                        KAA_LOG_ERROR(kaa_context_->logger, error_code,"Failed to process IN event for the Operations client socket %d", ops_fd);
                }
                if (FD_ISSET(ops_fd, &write_fds)) {
                    KAA_LOG_DEBUG(kaa_context_->logger, KAA_ERR_NONE,"Processing OUT event for the Operations client socket %d", ops_fd);
                    error_code = kaa_tcp_channel_process_event(&operations_channel, FD_WRITE);
                    if (error_code)
                        KAA_LOG_ERROR(kaa_context_->logger, error_code,"Failed to process OUT event for the Operations client socket %d", ops_fd);
                }
            }
        } else {
            KAA_LOG_ERROR(kaa_context_->logger, KAA_ERR_BAD_STATE,"Failed to poll descriptors: %s", strerror(errno));
            return -1;
        }
    }
    return 0;
}


int main(/*int argc, char *argv[]*/)
{
    printf("Event demo started\n");
    kaa_error_t error_code = kaa_demo_init();
    if (error_code) {
        printf("Failed to initialize Kaa demo. Error code: %d\n", error_code);
        return error_code;
    }

    int rval = kaa_demo_event_loop();
    kaa_demo_destroy();
    printf("Event demo stopped\n");
    return rval;
}

