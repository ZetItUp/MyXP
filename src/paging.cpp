/*
 * My Experience Operating System
 * Copyright (C) 2019-2020, Andreas Lindström
 */
#include <paging.h>

static inline void invlpg(void *m)
{
    asm volatile("invlpg (%0)" : : "b"(m) : "memory");
}