/*
 * aws_home.c
 *
 *  Created on: 29.06.2016
 *      Author: lion
 */

#include <string.h>
#include <ti/sysbios/knl/Task.h>

#include "aws_iot_log.h"
#include "aws_iot_version.h"
#include "aws_iot_mqtt_interface.h"
#include "aws_iot_shadow_interface.h"
#include "aws_iot_config.h"
#include "aws_iot_shadow_json.h"

#include "awssh.h"


IoT_Error_t aws_iot_shadow_add_reported_array(char *, size_t, uint8_t, jsonStruct_t*);

bool AWSSH_Init(aws_smarthome_t *awssh, const char *host, uint32_t port) {
	awssh->last_error = NONE_ERROR;
	awssh->cloud_states = 0;

	awssh->json_doc_size = sizeof(awssh->json_doc) / sizeof(char);

	aws_iot_mqtt_init(&awssh->client);

	awssh->sp = ShadowParametersDefault;
	awssh->sp.pMyThingName = AWS_IOT_MY_THING_NAME;
	awssh->sp.pMqttClientId = AWS_IOT_MQTT_CLIENT_ID;

	strcpy(awssh->sp.pHost, host);
	awssh->sp.port = port;

	awssh->sp.pClientCRT = AWS_IOT_CERTIFICATE_FILENAME;
	awssh->sp.pClientKey = AWS_IOT_PRIVATE_KEY_FILENAME;
	awssh->sp.pRootCA = AWS_IOT_ROOT_CA_FILENAME;

	INFO("Shadow Init");
	awssh->last_error = aws_iot_shadow_init(&awssh->client);
	if (NONE_ERROR != awssh->last_error) {
		ERROR("Shadow Initialization Error (%d)", awssh->last_error);
		return false;
	}

	INFO("Shadow Connect");
	awssh->last_error = aws_iot_shadow_connect(&awssh->client, &awssh->sp);
	if (NONE_ERROR != awssh->last_error) {
		ERROR("Shadow Connection Error (%d)", awssh->last_error);
		return false;
	}

	awssh->last_error = awssh->client.setAutoReconnectStatus(true);
	if (NONE_ERROR != awssh->last_error) {
		ERROR("Unable to set Auto Reconnect to true - %d", awssh->last_error);
		return false;
	}

	return true;
}

bool AWSSH_IsAlive(aws_smarthome_t* awssh) {
	return (
		NETWORK_ATTEMPTING_RECONNECT == awssh->last_error ||
		RECONNECT_SUCCESSFUL == awssh->last_error ||
		NONE_ERROR == awssh->last_error
	);
}

bool AWSSH_NotReady(aws_smarthome_t* awssh) {
	awssh->last_error = aws_iot_shadow_yield(&awssh->client, 200);
	return (NETWORK_ATTEMPTING_RECONNECT == awssh->last_error);
}




bool AWSSH_UpdateCloud(aws_smarthome_t *awssh, fpActionCallback_t callback) {

	// do not report if we have no values
	if(!awssh->cloud_states) {
		return true;
	}

	awssh->last_error = aws_iot_shadow_init_json_document(
		awssh->json_doc,
		awssh->json_doc_size
	);

	if (awssh->last_error == NONE_ERROR) {
		awssh->last_error = aws_iot_shadow_add_reported_array(
			awssh->json_doc,
			awssh->json_doc_size,
			awssh->cloud_states,
			awssh->cloud_state
		);

		if (awssh->last_error == NONE_ERROR) {
			awssh->last_error = aws_iot_finalize_json_document(
				awssh->json_doc,
				awssh->json_doc_size
			);

			if (awssh->last_error == NONE_ERROR) {
				awssh->last_error = aws_iot_shadow_update(
					&awssh->client,
					AWS_IOT_MY_THING_NAME,
					awssh->json_doc,
					callback,
					NULL,
					4,
					true
				);
				// INFO("cloud shadow update send: %s", JsonDocBuffer);
				INFO("cloud shadow update send");
				return true;
			}
		}
	}
	return false;
}

void AWSSH_Shutdown(aws_smarthome_t* awssh) {

    if (NONE_ERROR != awssh->last_error) {
        ERROR("An error occurred in the loop %d", awssh->last_error);
    }

    INFO("Disconnecting");
    awssh->last_error = aws_iot_shadow_disconnect(&awssh->client);

    if (NONE_ERROR != awssh->last_error) {
        ERROR("Disconnect error %d", awssh->last_error);
    }
}


bool AWSSH_AddCloudState(
		aws_smarthome_t* awssh,
		const char *key,
		void *phyval,
		JsonPrimitiveType type,
		jsonStructCallback_t callback,
		bool delta
) {

	if(awssh->cloud_states >= MAX_CLOUD_STATES) {
		ERROR("failed to register '%s': too many states", key);
		return false;
	}

	jsonStruct_t* state = &awssh->cloud_state[awssh->cloud_states++];

	state->cb = callback;
	state->pData = phyval;
	state->pKey = key;
	state->type = type;

	if(!delta) {
		return true;
	}

	awssh->last_error = aws_iot_shadow_register_delta(&awssh->client, state);

	if (NONE_ERROR != awssh->last_error) {
		ERROR("failed to register '%s': (%d)", key, awssh->last_error);
		return false;
	}
	return true;
}
