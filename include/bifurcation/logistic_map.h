#ifndef LOGISTIC_MAP_H
#define LOGISTIC_MAP_H

#include "shared/parameters.h"

T logistic_map(T x, T r) {
    return r * x * ((T)1 - x);
}

#endif