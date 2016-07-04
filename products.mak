# Copyright 2015-2016 Texas Instruments Incorporated. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License").
# You may not use this file except in compliance with the License.
# A copy of the License is located at
#
# http://aws.amazon.com/apache2.0
#
# or in the "license" file accompanying this file. This file is distributed
# on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
# express or implied. See the License for the specific language governing
# permissions and limitations under the License.
#
#
#  ======== products.mak ========
#

###### User Step: Update install paths to XDCtools, TI-RTOS, and Compiler ######
XDC_INSTALL_DIR       = C:\ti\xdctools_3_32_00_06_core
TIRTOS_INSTALL_DIR    = C:\ti\tirtos_cc32xx_2_16_01_14
TI_ARM_CODEGEN_INSTALL_DIR = C:\ti\ccsv6\tools\compiler\arm_15.12.2.LTS

###### Below dependencies are determined automatically ######
TIDRIVERS_INSTALL_DIR := $(wildcard $(TIRTOS_INSTALL_DIR)/products/tidrivers_*)
BIOS_INSTALL_DIR    := $(wildcard $(TIRTOS_INSTALL_DIR)/products/bios_*)
UIA_INSTALL_DIR     := $(wildcard $(TIRTOS_INSTALL_DIR)/products/uia_*)
NS_INSTALL_DIR      := $(wildcard $(TIRTOS_INSTALL_DIR)/products/ns_*)

CCWARE_INSTALL_DIR := $(wildcard $(TIRTOS_INSTALL_DIR)/products/CC3200_driverlib*)
CC3200SDK_INSTALL_DIR := $(TIDRIVERS_INSTALL_DIR)/packages/ti/mw/wifi/cc3x00
