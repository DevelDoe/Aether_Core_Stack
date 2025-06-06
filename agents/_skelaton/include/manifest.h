// Project: Seer
// File: include/manifest.h

#ifndef MANIFEST_H
#define MANIFEST_H

#include <json-c/json.h>

typedef struct {
    char id[32];
    char name[64]; 
    char role[16];
    char realm[32];
    char description[128];
    char host[64];
    int priority;
} Manifest;

Manifest load_manifest(const char *path);

#endif
