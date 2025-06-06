// Project: Seer
// File: src/manifest.c

#include "manifest.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "logger.h"

static const char *get_str_or_default(struct json_object *obj, const char *key, const char *fallback) {
    struct json_object *val = NULL;
    if (json_object_object_get_ex(obj, key, &val)) {
        return json_object_get_string(val);
    }
    return fallback;
}

Manifest load_manifest(const char *path) {
    Manifest m = {0};
    FILE *file = fopen(path, "r");
    if (!file) {
        char cwd[256] = {0};
        if (getcwd(cwd, sizeof(cwd)) == NULL) {
            strncpy(cwd, "unknown", sizeof(cwd) - 1);
        }
        log_error("[manifest] ❌ Failed to open %s (cwd: %s)\n", path, cwd);
        strncpy(m.id, "unknown", sizeof(m.id) - 1);
        return m;
    }

    char buffer[1024];
    fread(buffer, 1, sizeof(buffer) - 1, file);
    fclose(file);

    struct json_object *json = json_tokener_parse(buffer);
    if (!json) {
        log_error("[manifest] ❌ Failed to parse %s\n", path);
        strncpy(m.id, "unknown", sizeof(m.id) - 1);
        return m;
    }

    strncpy(m.id, get_str_or_default(json, "id", "unknown"), sizeof(m.id) - 1);
    strncpy(m.name, get_str_or_default(json, "name", ""), sizeof(m.name) - 1);

    strncpy(m.role, get_str_or_default(json, "role", ""), sizeof(m.role) - 1);
    strncpy(m.realm, get_str_or_default(json, "realm", ""), sizeof(m.realm) - 1);
    strncpy(m.description, get_str_or_default(json, "description", ""), sizeof(m.description) - 1);
    strncpy(m.host, get_str_or_default(json, "host", ""), sizeof(m.host) - 1);

    struct json_object *prio_obj = NULL;
    if (json_object_object_get_ex(json, "priority", &prio_obj)) {
        m.priority = json_object_get_int(prio_obj);
    }

    json_object_put(json);
    return m;
}
