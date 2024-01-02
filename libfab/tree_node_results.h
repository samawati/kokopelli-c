#ifndef CACHE_H
#define CACHE_H

#include "util_interval.h"
#include "util_region.h"
#include "util_switches.h"

struct Node_;

/** @struct Results_
    @brief Container for intermediate calculation results
*/
typedef struct Results_
{
    float    f;
    Interval i;
    float    r[MIN_VOLUME];
} Results;


/** @brief Fills node results with a constant
    @details n->results.{f,i,r} are all set equal to the constant
    @param n Target node
    @param value Constant to fill
*/
void fill_results(struct Node_* n, float value);

#endif
