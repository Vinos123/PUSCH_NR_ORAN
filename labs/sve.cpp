#include "kernels.h"
#include <arm_sve.h>
void add_simd(const float* a, const float* b, float* out, std::size_t n) {
    const std::size_t lanes=svcntw();
    for (std::size_t i=0; i<n;) {
        const svbool_t pg=svwhilelt_b32(i,n);
        const svfloat32_t x=svld1_f32(pg,a+i);
        const svfloat32_t y=svld1_f32(pg,b+i);
        svst1_f32(pg,out+i,svadd_f32_x(pg,x,y));
        // Avoid even theoretical size_t overflow on the last iteration.
        if (n-i<=lanes) break;
        i+=lanes;
    }
}
std::size_t simd_float_lanes() { return svcntw(); }
