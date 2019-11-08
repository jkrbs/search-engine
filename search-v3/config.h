#pragma once

// #define DEBUG_PREPARE_MATH_QRY
// #define DEBUG_MATH_SEARCH

#include "math-index-v3/config.h"
#define MAX_MATCHED_PATHS MAX_MATH_PATHS

// #define DEBUG_BIN_LP

// #define DEBUG_MATH_PRUNING

/* choose math pruning strategy */
//#define MATH_PRUNING_STRATEGY_NONE
//#define MATH_PRUNING_STRATEGY_MAXREF
//#define MATH_PRUNING_STRATEGY_GBP_NUM
  #define MATH_PRUNING_STRATEGY_GBP_LEN

#define MATH_SCORE_ETA 0.75f

#define MAX_MATH_OCCURS 8

#define SNIPPET_PADDING 320
#define MAX_SNIPPET_SZ 8192

#define DEFAULT_N_TOP_RESULTS 100
#define DEFAULT_RES_PER_PAGE  10
