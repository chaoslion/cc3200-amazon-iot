/*
 * Copyright (c) 2016, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <time.h>

#include <xdc/std.h>
#include <xdc/runtime/System.h>

#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/hal/Seconds.h>
#include <ti/net/sntp/sntp.h>

#include <simplelink.h>

#define NTP_PORT          123
#define NTP_SERVERS       1
#define NTP_SERVERS_SIZE  (NTP_SERVERS * sizeof(struct sockaddr_in))
#define NTP_TIMEOUT       30000

char NTP_HOSTNAME[] = "pool.ntp.org";
char ntpServers[NTP_SERVERS_SIZE];
static Semaphore_Handle semHandle = NULL;

void startNTP(void);

/*
 *  ======== setTime ========
 */
void setTime(uint32_t t)
{
    SlDateTime_t dt;
    struct tm tm;
    time_t ts;

    Seconds_set(t);

    time(&ts);
    tm = *localtime(&ts);

    /* Set system clock on network processor to validate certificate */
    dt.sl_tm_day  = tm.tm_mday;
    /* tm.tm_mon is the month since January, so add 1 to get the actual month */
    dt.sl_tm_mon  = tm.tm_mon + 1;
    /* tm.tm_year is the year since 1900, so add 1900 to get the actual year */
    dt.sl_tm_year = tm.tm_year + 1900;
    dt.sl_tm_hour = tm.tm_hour;
    dt.sl_tm_min  = tm.tm_min;
    dt.sl_tm_sec  = tm.tm_sec;
    sl_DevSet(SL_DEVICE_GENERAL_CONFIGURATION,
            SL_DEVICE_GENERAL_CONFIGURATION_DATE_TIME,
            sizeof(SlDateTime_t), (unsigned char *)(&dt));
}

/*
 *  ======== getTime ========
 */
uint32_t getTime(void)
{
    return (Seconds_get());
}

/*
 *  ======== timeUpdateHook ========
 *  Called after NTP time sync
 */
void timeUpdateHook(void *p)
{
   Semaphore_post(semHandle);
}

/*
 *  ======== startNTP ========
 */
void startNTP(void)
{
    int ret;
    unsigned long ip;
    time_t ts;
    struct sockaddr_in ntpAddr = {0};
    Semaphore_Params semParams;

    ret = gethostbyname((signed char *)NTP_HOSTNAME, strlen(NTP_HOSTNAME), &ip,
            AF_INET);
    if (ret != 0) {
        System_printf("startNTP: NTP host cannot be resolved!", ret);
    }

    ntpAddr.sin_addr.s_addr = htonl(ip);
    ntpAddr.sin_port = htons(NTP_PORT);
    ntpAddr.sin_family = AF_INET;
    memcpy(ntpServers, &ntpAddr, sizeof(struct sockaddr_in));

    ret = SNTP_start(getTime, setTime, timeUpdateHook,
            (struct sockaddr *)&ntpServers, NTP_SERVERS, 0);
    if (ret == 0) {
        System_printf("startNTP: SNTP cannot be started!", -1);
    }

    Semaphore_Params_init(&semParams);
    semParams.mode = Semaphore_Mode_BINARY;
    semHandle = Semaphore_create(0, &semParams, NULL);
    if (semHandle == NULL) {
        System_printf("startNTP: Cannot create semaphore!", -1);
    }

    SNTP_forceTimeSync();
    if (!Semaphore_pend(semHandle, NTP_TIMEOUT)) {
        System_printf("startNTP: error, no response from NTP server\n");
    }

    ts = time(NULL);
    System_printf("Current time: %s\n", ctime(&ts));
}
