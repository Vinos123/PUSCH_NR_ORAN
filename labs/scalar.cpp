#include "kernels.h"
#include <algorithm>
void add_scalar(const float* a, const float* b, float* out, std::size_t n) {
    for (std::size_t i=0; i<n; ++i) out[i]=a[i]+b[i];
}
std::int16_t sat_add16(std::int16_t a, std::int16_t b) {
    const std::int32_t s=std::int32_t(a)+std::int32_t(b);
    return static_cast<std::int16_t>(std::max<std::int32_t>(-32768,
                                    std::min<std::int32_t>(32767,s)));
}
