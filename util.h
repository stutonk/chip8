#pragma once

#include <stdlib.h>

#define FAIL(name, reason)                             \
    fprintf(stderr, "%s() error: %s\n", name, reason); \
    exit(EXIT_FAILURE);
