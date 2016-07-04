/*
 * Copyright 2015 Texas Instruments Incorporated. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *  http://aws.amazon.com/apache2.0
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */

#ifndef SRC_PROTOCOL_MQTT_AWS_IOT_EMBEDDED_CLIENT_WRAPPER_PLATFORM_TIRTOS_COMMON_TIMER_TIRTOS_H_
#define SRC_PROTOCOL_MQTT_AWS_IOT_EMBEDDED_CLIENT_WRAPPER_PLATFORM_TIRTOS_COMMON_TIMER_TIRTOS_H_

#include <stdint.h>

struct Timer {
    uint32_t end_time;
    uint32_t offset;
};

#endif
