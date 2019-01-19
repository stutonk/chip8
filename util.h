#pragma once

#include <stdlib.h>

#define FAIL(reason)                             \
    fprintf(stderr, "%s() error: %s\n", __func__, reason); \
    exit(EXIT_FAILURE);
