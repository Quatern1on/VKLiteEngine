#pragma once
#include "pch.h"

#define assertm(exp, msg)   \
do {                        \
    if (!(exp)) {           \
        LOG(FATAL) << msg;  \
    }                       \
} while(false)
