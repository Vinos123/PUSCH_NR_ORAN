#pragma once
#include <cstddef>
#include <cstdint>

// Teaching contract: arrays have at least n valid elements; output does not overlap input.
// n == 0 performs no access. The caller owns all buffers throughout the call.
void add_scalar(const float*, const float*, float*, std::size_t);
void add_auto(const float*, const float*, float*, std::size_t);
std::int16_t sat_add16(std::int16_t, std::int16_t);
#if defined(USE_AVX2) || defined(USE_SVE)
void add_simd(const float*, const float*, float*, std::size_t);
std::size_t simd_float_lanes();
#endif
#ifdef USE_AVX2
void sat_avx2(const std::int16_t*, const std::int16_t*, std::int16_t*, std::size_t);
#endif
