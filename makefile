#
#  Copyright (c) 2015-2016, Texas Instruments Incorporated
#  All rights reserved.
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions
#  are met:
#
#  *  Redistributions of source code must retain the above copyright
#     notice, this list of conditions and the following disclaimer.
#
#  *  Redistributions in binary form must reproduce the above copyright
#     notice, this list of conditions and the following disclaimer in the
#     documentation and/or other materials provided with the distribution.
#
#  *  Neither the name of Texas Instruments Incorporated nor the names of
#     its contributors may be used to endorse or promote products derived
#     from this software without specific prior written permission.
#
#  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
#  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
#  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
#  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
#  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
#  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
#  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
#  OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
#  WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
#  OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
#  EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#

TREE_ROOT = .
include $(TREE_ROOT)/products.mak
include makedefs

APP = aws_app

VPATH = ../..:$(TREE_ROOT)/aws_iot_src/protocol/mqtt/aws_iot_embedded_client_wrapper/platform_tirtos:$(TREE_ROOT)/aws_iot_src/protocol/mqtt/aws_iot_embedded_client_wrapper:$(TREE_ROOT)/aws_iot_src/shadow:$(TREE_ROOT)/aws_iot_src/utils:$(TREE_ROOT)/aws_mqtt_embedded_client_lib/MQTTClient-C/src:$(TREE_ROOT)/aws_mqtt_embedded_client_lib/MQTTPacket/src

DEFS = -DIOT_INFO -DIOT_ERROR -DIOT_WARN -DIOT_DEBUG

INCS = -I../.. -I$(TREE_ROOT)/aws_iot_src/protocol/mqtt -I$(TREE_ROOT)/aws_iot_src/utils -I$(TREE_ROOT)/aws_iot_src/shadow -I$(TREE_ROOT)/aws_iot_src/protocol/mqtt/aws_iot_embedded_client_wrapper -I$(TREE_ROOT)/aws_iot_src/protocol/mqtt/aws_iot_embedded_client_wrapper/platform_tirtos -I$(TREE_ROOT)/aws_mqtt_embedded_client_lib/MQTTClient-C/src -I$(TREE_ROOT)/aws_mqtt_embedded_client_lib/MQTTPacket/src

CFLAGS :=  $(DEFS) $(INCS) -g --cmd_file=./$(APP)/compiler.opt $(CFLAGS)

SOURCES = MQTTPacket.c \
          MQTTDeserializePublish.c \
          MQTTConnectClient.c \
          MQTTSubscribeClient.c \
          MQTTSerializePublish.c \
          MQTTUnsubscribeClient.c \
          MQTTClient.c \
          aws_iot_mqtt_embedded_client_wrapper.c \
          aws_iot_json_utils.c \
          aws_iot_shadow_actions.c \
          aws_iot_shadow.c \
          aws_iot_shadow_json.c \
          aws_iot_shadow_records.c \
          jsmn.c \
          network_sl.c \
          timer.c \
          awssh.c   

OBJECTS = $(SOURCES:.c=.o)

LIBS = aws_iot_sl.aem4

.PRECIOUS: %/compiler.opt %/linker.cmd

%/compiler.opt %/linker.cmd : %.cfg
	$(CONFIGURO) -c $(CODEGEN_INSTALL_DIR) -t $(TARGET) -p $(PLATFORM) -r release $<

%.o : %.c

%.o : %.c $(APP)/compiler.opt aws_iot_config.h
	@$(CC) $(CFLAGS) -c $<

all: $(APP).out

$(LIBS): $(OBJECTS)
	$(AR) rq $@ $^

$(APP).out: $(LIBS) $(APP).o main.o rom_pin_mux_config.o startsntp.o netwifi.o UARTUtils.o CC3200_LAUNCHXL.o CC3200_LAUNCHXL.cmd $(APP)/linker.cmd
	@echo building $@ ..
	$(LD) -o $@ $^ -x -m $@.map $(LFLAGS)

clean:
	@echo cleaning ..
	$(RM) *.o *.out *.map $(LIBS)
	$(RMDIR) $(APP)
