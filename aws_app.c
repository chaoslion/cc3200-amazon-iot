#include <string.h>
#include <math.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/drivers/GPIO.h>
#include <ti/drivers/I2C.h>


#include "aws_iot_log.h"
#include "aws_iot_version.h"
#include "aws_iot_mqtt_interface.h"
#include "aws_iot_shadow_interface.h"
#include "aws_iot_config.h"
#include "awssh.h"
#include "Board.h"
#include "driver.h"



/*
 ACHTUNG:

 das aktuelle  format des shadow state ist:

 {
  "desired": {
    "rgb_light": 1337,
    "status_led": false
  },
  "reported": {
    "rgb_light": 1337,
    "ambient_temp": 30.3,
    "ambient_light": 14,
    "status_led": true
  }
}

 */

// ==========globals
aws_smarthome_t awssh;


// ==========physical states
uint32_t rgb_light = 0;
uint32_t ambient_light = 0;
bool status_led = false;
float ambient_temp = 0.0f;
float acc_xdir = 0.0f;
float acc_ydir = 0.0f;
float acc_zdir = 0.0f;
bool earthquake_alarm = false;

// ==========state callbacks
static void callback_rgb_light(const char *, uint32_t, jsonStruct_t *);
static void callback_status_led(const char *, uint32_t, jsonStruct_t *);

// ==========cloud callbacks
static void callback_shadow_update(const char *, ShadowActions_t, Shadow_Ack_Status_t, const char*, void *);

static bool init_cloud_states() {
	bool success = true;

	// ===========rgb_light
	// read+set
	success = success && AWSSH_AddCloudState(
		&awssh,
		"rgb_light",
		&rgb_light,
		SHADOW_JSON_UINT32,
		callback_rgb_light,
		true
	);

    // ===========ambient_light
    // read
	success = success && AWSSH_AddCloudState(
		&awssh,
		"ambient_light",
		&ambient_light,
		SHADOW_JSON_UINT32,
		NULL,
		false
	);

    // ===========status_led
	// read+set
	success = success && AWSSH_AddCloudState(
		&awssh,
		"status_led",
		&status_led,
		SHADOW_JSON_BOOL,
		callback_status_led,
		true
	);

	// ===========ambient_temp
	// read
	success = success && AWSSH_AddCloudState(
		&awssh,
		"ambient_temp",
		&ambient_temp,
		SHADOW_JSON_FLOAT,
		NULL,
		false
	);
/*
	// ===========acc_xdir
	// read
	success = success && AWSSH_AddCloudState(
		&awssh,
		"acc_xdir",
		&acc_xdir,
		SHADOW_JSON_FLOAT,
		NULL,
		false
	);

	// ===========acc_ydir
	// read
	success = success && AWSSH_AddCloudState(
		&awssh,
		"acc_ydir",
		&acc_ydir,
		SHADOW_JSON_FLOAT,
		NULL,
		false
	);

	// ===========acc_zdir
	// read
	success = success && AWSSH_AddCloudState(
		&awssh,
		"acc_zdir",
		&acc_zdir,
		SHADOW_JSON_FLOAT,
		NULL,
		false
	);
*/
	// ===========earthquake_alarm
	// read
	success = success && AWSSH_AddCloudState(
		&awssh,
		"earthquake_alarm",
		&earthquake_alarm,
		SHADOW_JSON_BOOL,
		NULL,
		false
	);
    return success;
}

static void init_hardware_state() {
	// status led
	GPIO_write(Board_LED0, (status_led?Board_LED_ON:Board_LED_OFF));

	//init led strip
	ledInit();
	ledSetColor(0,0,0);
	ledShow();

	//init light sensor
	lightSensorInit();
}

static bool hw_i2c_transfer(uint8_t address, uint8_t *tx, uint8_t *rx, uint32_t txcnt, uint32_t rxcnt) {
	I2C_Handle i2c;
	I2C_Params i2cParams;
	I2C_Transaction i2cTransaction;
	i2cTransaction.slaveAddress = address;
	I2C_Params_init(&i2cParams);
	i2cParams.bitRate = I2C_400kHz;

	i2c = I2C_open(Board_I2C_TMP, &i2cParams);
	if (i2c == NULL) {
		return false;
	}

	i2cTransaction.writeBuf = tx;
	i2cTransaction.writeCount = txcnt;
	i2cTransaction.readBuf = rx;
	i2cTransaction.readCount = rxcnt;
	I2C_transfer(i2c, &i2cTransaction);

	I2C_close(i2c);
	return true;
}


bool tempsensor_read(float* temp) {
	uint8_t tx[1] = {0x01};
	uint8_t rx[2];

	if(!hw_i2c_transfer(0x41, tx, rx, 1, 2)) {
		return false;
	}
	int32_t value = ((int32_t)rx[0] << 8) | (int32_t)rx[1];
	*temp = (float)(value >> 2) / 32.0f;
	return true;
}


// chip ist ein bme222e, nicht bme222
#define ACC_CONV 15.63f
bool accsensor_read(float *x, float *y, float *z) {
	uint8_t tx[1] = {0x02};
	uint8_t rx[6];

	// set register pointer
	if(!hw_i2c_transfer(0x18, tx, rx, 1, 6)) {
		return false;
	}

	*x = (int8_t)rx[1] * ACC_CONV / 1000.0f;
	*y = (int8_t)rx[3] * ACC_CONV / 1000.0f;
	*z = (int8_t)rx[5] * ACC_CONV / 1000.0f;

	return true;
}



static void task_get_temperature() {
	tempsensor_read(&ambient_temp);
}

static void task_get_ambient_light() {
	ambient_light = lightSensorGetValue(10);
}

#define EARTHQUAKE_THRESHOLD 0.1
static void task_detect_earthquake() {
	static float lastx = 0, lasty = 0;
	accsensor_read(&acc_xdir, &acc_ydir, &acc_zdir);

	float dx = acc_xdir - lastx;
	lastx = acc_xdir;

	float dy = acc_ydir - lasty;
	lasty = acc_ydir;
	// z: aus board raus
	// x: zu SW1 stecker
	// y: zu quartz
	earthquake_alarm = (
		(fabs(dx) >= EARTHQUAKE_THRESHOLD) ||
		(fabs(dy) >= EARTHQUAKE_THRESHOLD)
	);
}

void runAWSClient(void) {

	if(!AWSSH_Init(&awssh, AWS_IOT_MQTT_HOST, AWS_IOT_MQTT_PORT)) {
		return;
	}

	init_hardware_state();

    if(!init_cloud_states()) {
    	return;
    }
    
    while( AWSSH_IsAlive(&awssh) ) {

        if(AWSSH_NotReady(&awssh)) {
            Task_sleep(1000);
            continue;
        }

        // PERIODIC TASKS
        task_get_ambient_light();
        task_get_temperature();
        task_detect_earthquake();

        // UPDATE CLOUD SHADOW
        AWSSH_UpdateCloud(&awssh, callback_shadow_update);
        Task_sleep(1000);
    }

    AWSSH_Shutdown(&awssh);
}

void callback_rgb_light(const char *json_str, uint32_t json_len, jsonStruct_t *ctx) {
    if (ctx != NULL) {
        uint32_t data = *(uint32_t *)(ctx->pData);
        uint8_t r = data & 0xFF;
        uint8_t g = ((data >> 8) & 0xFF);
        uint8_t b = ((data >> 16) & 0xFF);        
        ledSetColor(r,g,b);
        ledShow();
    }
}

void callback_status_led(const char *json_str, uint32_t json_len, jsonStruct_t *ctx) {
    if (ctx != NULL) {
        bool data= *(bool *)(ctx->pData);
        GPIO_write(Board_LED0, (data?Board_LED_ON:Board_LED_OFF));
    }
}

void callback_shadow_update(const char *pThingName, ShadowActions_t action,
        Shadow_Ack_Status_t status, const char *pReceivedJsonDocument,
        void *pContextData)
{
    if (status == SHADOW_ACK_TIMEOUT) {
        INFO("Update Timeout--");
    } else if (status == SHADOW_ACK_REJECTED) {
        INFO("Update RejectedXX");
    } else if (status == SHADOW_ACK_ACCEPTED) {
        INFO("Update Accepted !!");
    }
}
