/*
 * Copyright (C) 2018 Unlegacy Android Project
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

#include <cutils/log.h>
#include <hardware/sensors.h>
#include <string>
#include <dlfcn.h>
#include <errno.h>

// The "real" sensor lib to wrap around
#define SENSOR_LIB "sensors.msm8916.so"

static void *lib_handle = NULL;
static struct sensors_module_t *lib_sensors_module;

// Copy of the original sensors list with the fixed properties
static const struct sensor_t *global_sensors_list = NULL;

static int open_lib()
{
    if (lib_handle != NULL)
        return 0;

    lib_handle = dlopen(SENSOR_LIB, RTLD_LAZY);
    if (lib_handle == NULL) {
        ALOGW("dlerror(): %s", dlerror());
        return -EINVAL;
    } else {
        const char *sym = HAL_MODULE_INFO_SYM_AS_STR;
        ALOGV("Loaded library %s", SENSOR_LIB);
        ALOGV("Opening symbol \"%s\"", sym);
        // clear old errors
        dlerror();
        lib_sensors_module = (sensors_module_t *) dlsym(lib_handle, sym);
        const char* error;
        if ((error = dlerror()) != NULL) {
            ALOGE("Error calling dlsym: %s", error);
            dlclose(lib_handle);
            return -EINVAL;
        } else {
            ALOGV("Loaded symbols from \"%s\"", sym);
        }
    }

    return 0;
}

//static int sensors_get_sensors_list(struct sensors_module_t *module __unused,
static int get_sensors_list(struct sensors_module_t *module __unused,
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

