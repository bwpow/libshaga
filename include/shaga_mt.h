/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2022, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#ifndef HEAD_shaga_full_mt
#define HEAD_shaga_full_mt

#ifdef SHAGA
#error You must include only one shaga*.h
#endif // SHAGA

#define SHAGA
#define SHAGA_FULL
#define SHAGA_MULTI_THREAD

#include "shaga/common.h"

#endif // HEAD_shaga_full_mt
