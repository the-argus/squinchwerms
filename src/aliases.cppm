module;

#include <cstdint>

export module aliases;
export using u64 = uint64_t;
export using u32 = uint32_t;
export using u16 = uint16_t;
export using u8 = uint8_t;
export using i64 = int64_t;
export using i32 = int32_t;
export using i16 = int16_t;
export using i8 = int8_t;
export using f32 = float;
export using f64 = double;

static_assert(sizeof(f32) == 4);
static_assert(sizeof(f64) == 8);
