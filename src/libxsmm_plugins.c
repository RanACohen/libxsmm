//
// Created by rancohen on 5/11/2020.
//

#include "libxsmm_plugins.h"
#include "lib_utils.h"

#define MAX_FILE_PATH_LEN 65536

void libxsmm_plugins_init() {
    char plugins_dir[MAX_FILE_PATH_LEN];
    char plugin_to_use[MAX_FILE_PATH_LEN];
    size_t len;

    auto dir_env_exists = 0==getenv_s(&len, plugins_dir, MAX_FILE_PATH_LEN, "LIBXSMM_PLUGINS_DIR");
    char *const env_plugin_override = getenv("LIBXSMM_USE_PLUGIN");
    if (NULL != env_plugin_override) {
    }
}
