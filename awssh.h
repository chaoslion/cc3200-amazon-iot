/*
 * aws_nodes.h
 *
 *  Created on: 29.06.2016
 *      Author: lion
 */

#ifndef AWSSH_H_
#define AWSSH_H_

#define MAX_LENGTH_OF_UPDATE_JSON_BUFFER 512
#define MAX_CLOUD_STATES 12

struct aws_smarthome_s {
	MQTTClient_t client;
	ShadowParameters_t sp;
	char json_doc[MAX_LENGTH_OF_UPDATE_JSON_BUFFER];
	size_t json_doc_size;
	IoT_Error_t last_error;
	jsonStruct_t cloud_state[MAX_CLOUD_STATES];
	uint32_t cloud_states;
};

typedef struct aws_smarthome_s aws_smarthome_t;

bool AWSSH_Init(aws_smarthome_t *awssh, const char *, uint32_t);
bool AWSSH_IsAlive(aws_smarthome_t*);
bool AWSSH_NotReady(aws_smarthome_t*);
bool AWSSH_UpdateCloud(aws_smarthome_t*, fpActionCallback_t);
void AWSSH_Shutdown(aws_smarthome_t*);
bool AWSSH_AddCloudState(aws_smarthome_t*, const char *, void *, JsonPrimitiveType, jsonStructCallback_t, bool);

#endif /* AWSSH_H_ */
