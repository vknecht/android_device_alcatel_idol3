# Copyright (C) 2015 The CyanogenMod Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

$(call inherit-product, device/alcatel/idol3/full_crackling.mk)

# Inherit some common LineageOS stuff.
$(call inherit-product, vendor/cm/config/common_full_phone.mk)

# Must define platform variant before including any common things
TARGET_BOARD_PLATFORM_VARIANT := msm8916

PRODUCT_NAME := lineage_idol3
BOARD_VENDOR := alcatel
PRODUCT_DEVICE := idol3

PRODUCT_GMS_CLIENTID_BASE := android-alcatel

PRODUCT_MANUFACTURER := Alcatel
PRODUCT_MODEL := Alcatel Idol3

PRODUCT_BRAND := Alcatel
TARGET_VENDOR := alcatel
TARGET_VENDOR_PRODUCT_NAME := Alcatel
TARGET_VENDOR_DEVICE_NAME := idol3
