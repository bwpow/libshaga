/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2018, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#ifndef HEAD_shaga_lite
#define HEAD_shaga_lite

#ifdef SHAGA
#error You must include either shaga.h or shagalite.h, not both
#endif // SHAGA

#define SHAGA
#define SHAGA_LITE

#include "shaga/common.h"

#endif // HEAD_shaga_full
