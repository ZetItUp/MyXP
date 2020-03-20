/*
 * My Experience Operating System
 * Copyright (C) 2019, Andreas Lindström
 */
#ifndef __MYXP__SYSTEM_H
#define __MYXP__SYSTEM_H

#include <console.h>

#define ASSERT(b) ((b) ? (void)0 : panic_assert(__FILE__, __LINE__, #b))

#endif