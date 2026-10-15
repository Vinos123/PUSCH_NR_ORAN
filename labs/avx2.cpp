#include "kernels.h"
#include <immintrin.h>
void add_simd(const float* a, const float* b, float* out, std::size_t n) {
    std::size_t i=0;
    for (; n-i>=8; i+=8) {
        const __m256 x=_mm256_loadu_ps(a+i);
        const __m256 y=_mm256_loadu_ps(b+i);
        _mm256_storeu_ps(out+i,_mm256_add_ps(x,y));
    }
    for (; i<n; ++i) out[i]=a[i]+b[i];
}
std::size_t simd_float_lanes() { return 8; }
void sat_avx2(const std::int16_t* a, const std::int16_t* b,
              std::int16_t* out, std::size_t n) {
    std::size_t i=0;
    for (; n-i>=16; i+=16) {
        const __m256i x=_mm256_loadu_si256(reinterpret_cast<const __m256i*>(a+i));
        const __m256i y=_mm256_loadu_si256(reinterpret_cast<const __m256i*>(b+i));
        _mm256_storeu_si256(reinterpret_cast<__m256i*>(out+i),_mm256_adds_epi16(x,y));
    }
    for (; i<n; ++i) out[i]=sat_add16(a[i],b[i]);
}
