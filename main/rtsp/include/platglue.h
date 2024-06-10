#pragma once

//#define ENABLE_DEBUG
#ifdef ENABLE_DEBUG
#define DEBUG_PRINT(...) printf(__VA_ARGS__)
#else
#define DEBUG_PRINT(...) /* do nothing */
#endif

//#define ENABLE_ERROR
#ifdef ENABLE_ERROR
#define ERROR_PRINT(...) printf(__VA_ARGS__)
#else
#define ERROR_PRINT(...) /* do nothing */
#endif

#include "platglue-esp-idf.h"