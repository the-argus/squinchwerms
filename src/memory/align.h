#pragma once
#include "assert.h"
#include "intshorthand.h"
#include <stdio.h>

u8 *mem_align_exponent(u8 *input, u8 align_exponent);

u8 get_alignment_exponent_from_alignment(u64 alignment);

u8 *mem_align(u8 *addr, u8 alignment);
