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
#include <ti/sysbios/knl/Clock.h>

#include "timer_interface.h"

static uint32_t getTimeInMillis(void)
{
    return ((Clock_getTicks() * Clock_tickPeriod) / 1000);
}

char expired(Timer *timer)
{
    return ((timer->end_time > 0)
             && ((getTimeInMillis() + timer->offset) > timer->end_time));
}

void countdown_ms(Timer *timer, unsigned int timeout)
{
    uint32_t timems = getTimeInMillis();

    timer->end_time = timems + timeout;

    if (timer->end_time < timems) {
        timer->offset = ~0 - timems + 1;
        timer->end_time += timer->offset;
    }
    else {
        timer->offset = 0;
    }
}

void countdown(Timer *timer, unsigned int timeout)
{
    uint32_t timems = getTimeInMillis();

    timer->end_time = timems + (timeout * 1000);

    if (timer->end_time < timems) {
        timer->offset = ~0 - timems + 1;
        timer->end_time += timer->offset;
    }
    else {
        timer->offset = 0;
    }
}

int left_ms(Timer *timer)
{
    int diff = timer->end_time - (getTimeInMillis() + timer->offset);
    return (diff > 0 ? diff : 0);
}

void InitTimer(Timer *timer)
{
    timer->end_time = 0;
    timer->offset = 0;
}
