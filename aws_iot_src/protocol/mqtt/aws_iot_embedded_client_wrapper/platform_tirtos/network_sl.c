/*
 * Copyright 2015-2016 Texas Instruments Incorporated. All Rights Reserved.
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
#include "network_interface.h"
#include "aws_iot_error.h"

#include <stdlib.h>
#include <ti/net/tls.h>
#include <ti/net/ssock.h>

#define IOT_TLS_CERT_PATH "/cert"

struct TlsContext {
    Ssock_Handle ssock;
    TLS_Handle tlsH;
};

extern uint32_t NetWiFi_isConnected(void);

int iot_tls_init(Network *pNetwork)
{
    if (pNetwork == NULL) {
        return (NULL_VALUE_ERROR);
    }

    pNetwork->my_socket = 0;
    pNetwork->connect = iot_tls_connect;
    pNetwork->mqttread = iot_tls_read;
    pNetwork->mqttwrite = iot_tls_write;
    pNetwork->disconnect = iot_tls_disconnect;
    pNetwork->isConnected = iot_tls_is_connected;
    pNetwork->destroy = iot_tls_destroy;

    return (NONE_ERROR);
}

int iot_tls_connect(Network *pNetwork, TLSConnectParams TLSParams)
{
    IoT_Error_t ret = NONE_ERROR;
    unsigned long ip;
    int skt = 0;
    struct sockaddr_in address;
    struct TlsContext *tlsContext;

    if (pNetwork == NULL) {
        return (NULL_VALUE_ERROR);
    }

    tlsContext = (struct TlsContext *)malloc(sizeof(struct TlsContext));
    if (tlsContext == NULL) {
        return (GENERIC_ERROR);
    }
    memset(tlsContext, 0, sizeof(struct TlsContext));

    /*
     *  Create TLS context.  For this release, all cert files must exist in
     *  the "/cert" directory and be named "ca.der", "cert.der" and "key.der".
     *  The ability to change this will be added in a future release.
     */
    tlsContext->tlsH = TLS_create(TLS_METHOD_CLIENT_TLSV1_2, NULL,
            IOT_TLS_CERT_PATH);
    if (tlsContext->tlsH == NULL) {
        ret = SSL_INIT_ERROR;
        goto QUIT;
    }

    if (gethostbyname((signed char *)TLSParams.pDestinationURL,
            strlen(TLSParams.pDestinationURL), &ip, AF_INET) < 0) {
        ret = GENERIC_ERROR;
        goto QUIT;
    }

    skt = socket(AF_INET, SOCK_STREAM, SL_SEC_SOCKET);
    if (skt < 0) {
        ret = TCP_SETUP_ERROR;
        goto QUIT;
    }

    tlsContext->ssock = Ssock_create(skt);
    if (tlsContext->ssock == NULL) {
        ret = TCP_SETUP_ERROR;
        goto QUIT;
    }

    if (Ssock_startTLS(tlsContext->ssock, tlsContext->tlsH) != 0) {
        ret = SSL_CERT_ERROR;
        goto QUIT;
    }

    address.sin_family = AF_INET;
    address.sin_port = htons(TLSParams.DestinationPort);
    address.sin_addr.s_addr = htonl(ip);
    if (connect(skt, (struct sockaddr  *)&address,
            sizeof(address)) < 0) {
        ret = SSL_CONNECT_ERROR;
        goto QUIT;
    }

    /* Use pNetwork to store both the socket and TLS context */
    pNetwork->my_socket = (int)tlsContext;

QUIT:

    if (ret != NONE_ERROR) {
        if (tlsContext->ssock) {
            Ssock_delete(&tlsContext->ssock);
        }

        if (tlsContext->tlsH) {
            TLS_delete((TLS_Handle *)&tlsContext->tlsH);
        }

        if (skt >= 0) {
            close(skt);
        }

        free(tlsContext);
    }

    return (ret);
}

int iot_tls_is_connected(Network *pNetwork)
{
    return ((int)NetWiFi_isConnected());
}

int iot_tls_write(Network *pNetwork, unsigned char *pMsg, int len,
            int timeout_ms)
{
    Ssock_Handle ssock = NULL;
    int bytes = 0;

    if (pNetwork == NULL || pMsg == NULL || pNetwork->my_socket == 0 ||
            ((struct TlsContext *)pNetwork->my_socket)->ssock == 0 || timeout_ms == 0) {
        return (NULL_VALUE_ERROR);
    }

    ssock = ((struct TlsContext *)pNetwork->my_socket)->ssock;

    bytes = Ssock_send(ssock, pMsg, len, 0);
    if (bytes >= 0) {
       return (bytes);
    }

    return (SSL_WRITE_ERROR);
}

int iot_tls_read(Network *pNetwork, unsigned char *pMsg, int len,
        int timeout_ms)
{
    int bytes = 0;
    int skt;
    struct timeval tv;
    Ssock_Handle ssock = NULL;

    if (pNetwork == NULL || pMsg == NULL || pNetwork->my_socket == 0 ||
            ((struct TlsContext *)pNetwork->my_socket)->ssock == 0 ||
            timeout_ms == 0) {
        return (NULL_VALUE_ERROR);
    }

    ssock = ((struct TlsContext *)pNetwork->my_socket)->ssock;

    tv.tv_sec = 0;
    tv.tv_usec = timeout_ms * 1000;

    skt = Ssock_getSocket(ssock);

    if (setsockopt(skt, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv,
            sizeof(struct timeval)) == 0) {
        bytes = Ssock_recvall(ssock, pMsg, len, 0);
        if (bytes >= 0) {
            return (bytes);
        }
    }

    return (SSL_READ_ERROR);
}

void iot_tls_disconnect(Network *pNetwork)
{
    int skt;
    Ssock_Handle ssock = NULL;

    if (pNetwork == NULL || pNetwork->my_socket == 0 ||
            ((struct TlsContext *)pNetwork->my_socket)->ssock == 0) {
        return;
    }

    TLS_delete((TLS_Handle *)&((struct TlsContext *)pNetwork->my_socket)->tlsH);
    ssock = ((struct TlsContext *)pNetwork->my_socket)->ssock;
    skt = Ssock_getSocket(ssock);
    Ssock_delete(&ssock);
    close(skt);
    free((void *)pNetwork->my_socket);
}

int iot_tls_destroy(Network *pNetwork)
{
    if (pNetwork == NULL) {
        return (NULL_VALUE_ERROR);
    }

    pNetwork->my_socket = 0;
    pNetwork->mqttread = NULL;
    pNetwork->mqttwrite = NULL;
    pNetwork->disconnect = NULL;

    return (NONE_ERROR);
}
