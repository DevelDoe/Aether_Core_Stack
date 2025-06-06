// Project: Seer
// File: include/utils.c

#include <stddef.h>
#include <stdint.h>
#include <sys/time.h>

uint64_t get_time_ms(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)tv.tv_sec * 1000 + tv.tv_usec / 1000;
}
