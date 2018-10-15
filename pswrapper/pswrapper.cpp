/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "Sensors"
#define SENSOR_LIB "/system/lib64/hw/sensors.stockmsm8916.so"

#include <hardware/sensors.h>
#include <errno.h>
#include <dlfcn.h>
#include <string>

#include <utils/Atomic.h>
#include <utils/Log.h>

#include <cutils/log.h>

/*****************************************************************************/
static struct {
	void *old_handle;	/* DL open handle */
	int (*old_poll)(struct sensors_poll_device_t *dev,
		sensors_event_t* data, int count);
	int (*get_sensors_list)(struct sensors_module_t* module,
		struct sensor_t const** list);
} ChainHALinfo = {
	NULL, NULL, NULL
};

static int chain_open_sensors(const struct hw_module_t* module, const char* id,
                        struct hw_device_t** device);

// Copy of the original sensors list with the fixed properties
static const struct sensor_t *global_sensors_list = NULL;

static int chain_sensors__get_sensors_list(struct sensors_module_t* module,
                                     struct sensor_t const** list)
{
/*
	int old_status;
	LOGE("%s:%i", __func__, __LINE__);
	old_status = (ChainHALinfo.get_sensors_list)(module, list);
	LOGE("%s:%i status = %i", __func__, __LINE__, old_status);
	return old_status;
*/
    int sensors_count;
    sensors_count = (ChainHALinfo.get_sensors_list)(module, list);

    // sensors_list is full of consts - Manipulate this non-const list,
    // and point the const one to it when we're done.
    sensor_t *mutable_sensor_list = new sensor_t[sensors_count];
    memcpy(mutable_sensor_list, *list,
        sizeof(struct sensor_t) * sensors_count);
    global_sensors_list = mutable_sensor_list;
    *list = global_sensors_list;

    // fix sensor properties
    for (int i = 0; i < sensors_count; ++i) {
        sensor_t *sensor = &mutable_sensor_list[i];
        if (sensor->type == SENSOR_TYPE_PROXIMITY) {
		sensor->flags = SENSOR_FLAG_WAKE_UP | SENSOR_FLAG_ON_CHANGE_MODE;
		ALOGV("pswrapper: forcing SENSOR_FLAG_WAKE_UP for proximity sensor...");
	}
    }

    return sensors_count;
}

static struct hw_module_methods_t chain_sensors_module_methods = {
        open: chain_open_sensors
};

struct sensors_module_t HAL_MODULE_INFO_SYM = {
        common: {
                tag: HARDWARE_MODULE_TAG,
                version_major: 1,
                version_minor: 0,
                id: SENSORS_HARDWARE_MODULE_ID,
                name: "Sensor module",
                author: "HY Research LLC",
                methods: &chain_sensors_module_methods,
        },
        get_sensors_list: chain_sensors__get_sensors_list,
};

/*****************************************************************************/
#if 0
static int chain_poll__poll(struct sensors_poll_device_t *dev,
        sensors_event_t* data, int count)
{
	int old_ret;
	int i;

	/* Acquire data */
	old_ret = ChainHALinfo.old_poll(dev, data, count);

	/* Modify data if needed */
	if (old_ret > 0) {
		/* There is data! */
		for (i = 0; i < old_ret; i++) {
			if (data[i].type == SENSOR_TYPE_ACCELEROMETER) {
				data[i].data[0] = -data[i].data[0];
				data[i].data[1] = -data[i].data[1];
				data[i].data[2] = -data[i].data[2];
			}
		}
	}

	return old_ret;
}
#endif
/*****************************************************************************/

static int chain_open_sensors(const struct hw_module_t* module, const char* id,
                        struct hw_device_t** device)

{
	//char old_sensorHAL_path[2048];
	struct sensors_module_t *old_hmi;
	struct sensors_poll_device_t *old_device;
	void *handle;
	const char *sym = HAL_MODULE_INFO_SYM_AS_STR;
	int old_status;

	//LOGE("%s:%i", __func__, __LINE__);

	/* First load the old HAL */

	//snprintf(old_sensorHAL_path, 2048, "%s/sensors.old.so", "/system/lib/hw");
	//handle = dlopen(old_sensorHAL_path, RTLD_NOW);
	handle = dlopen(SENSOR_LIB, RTLD_NOW);
	if (handle == NULL)
		return -EINVAL;

	old_hmi = (struct sensors_module_t *)dlsym(handle, sym);
	if (old_hmi == NULL) {
		dlclose(handle);
		return -EINVAL;
	}

	if (strcmp(SENSORS_HARDWARE_MODULE_ID, old_hmi->common.id) != 0) {
		dlclose(handle);
		return -EINVAL;
	}
	ChainHALinfo.old_handle = handle;
	
	ChainHALinfo.get_sensors_list = old_hmi->get_sensors_list;

	/* Call old HAL's open */
	old_status = (old_hmi->common.methods->open)(&(old_hmi->common),
							id, device);
#if 0
	/* Save the old poll method and replace it with ours */
	old_device = (struct sensors_poll_device_t *)*device;
	ChainHALinfo.old_poll = old_device->poll;
	old_device->poll = chain_poll__poll;
#endif
	return old_status;
}



#if 0
static int chain_get_sensors_list(struct sensors_module_t *module __unused,
    struct sensor_t const** sensors_list)
{
    int ret = open_lib();
    if (ret < 0)
        return 0;
    ALOGV("pswrapper: requested sensors list...");
    int sensors_count =
        lib_sensors_module->get_sensors_list(lib_sensors_module, sensors_list);

    // sensors_list is full of consts - Manipulate this non-const list,
    // and point the const one to it when we're done.
    sensor_t *mutable_sensor_list = new sensor_t[sensors_count];
    memcpy(mutable_sensor_list, *sensors_list,
        sizeof(struct sensor_t) * sensors_count);
    global_sensors_list = mutable_sensor_list;
    *sensors_list = global_sensors_list;

    // fix sensor properties
    for (int i = 0; i < sensors_count; ++i) {
        sensor_t *sensor = &mutable_sensor_list[i];
        if (sensor->type == SENSOR_TYPE_PROXIMITY) {
		sensor->flags = SENSOR_FLAG_WAKE_UP | SENSOR_FLAG_ON_CHANGE_MODE;
		ALOGV("pswrapper: forcing PS WAKE_UP flag...");
	}
    }

    return sensors_count;
}
#endif
