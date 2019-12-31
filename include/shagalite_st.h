/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2020, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#ifndef HEAD_shaga_lite_st
#define HEAD_shaga_lite_st

#ifdef SHAGA
#error You must include only one shaga*.h
#endif // SHAGA

#define SHAGA
#define SHAGA_LITE
#define SHAGA_SINGLE_THREAD

#include "shaga/common.h"

#endif // HEAD_shaga_lite_st
