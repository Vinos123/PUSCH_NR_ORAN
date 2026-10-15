#include "kernels.h"
void add_auto(const float* a, const float* b, float* out, std::size_t n) {
    for (std::size_t i=0; i<n; ++i) out[i]=a[i]+b[i];
}
