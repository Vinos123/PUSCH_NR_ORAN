#include "kernels.h"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>
#include <memory>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

using Kernel=void (*)(const float*,const float*,float*,std::size_t);
constexpr float guard=-12345.0f;
volatile float benchmark_sink=0;

void require(bool ok,const char* message) {
    if (!ok) throw std::runtime_error(message);
}
void check_float(Kernel fn,std::size_t n,std::size_t offset,std::mt19937& rng) {
    // Inputs terminate exactly at n: sanitizer can detect full-vector tail overreads.
    auto a=std::make_unique<float[]>(n+offset);
    auto b=std::make_unique<float[]>(n+offset);
    auto out=std::make_unique<float[]>(n+offset+1);
    std::fill_n(out.get(),n+offset+1,guard);
    std::uniform_int_distribution<int> distribution(-4096,4096);
    std::vector<float> expected(n);
    for (std::size_t i=0; i<n; ++i) {
        a[i+offset]=float(distribution(rng))/16.0f;
        b[i+offset]=float(distribution(rng))/16.0f;
        expected[i]=a[i+offset]+b[i+offset];
    }
    fn(a.get()+offset,b.get()+offset,out.get()+offset,n);
    for (std::size_t i=0; i<n; ++i) require(out[i+offset]==expected[i],"float mismatch");
    for (std::size_t i=0; i<offset; ++i) require(out[i]==guard,"prefix overwrite");
    require(out[n+offset]==guard,"suffix overwrite");
}
std::int16_t independent_sat(std::int16_t a,std::int16_t b) {
    const std::int64_t sum=std::int64_t(a)+std::int64_t(b);
    if (sum>32767) return 32767;
    if (sum<-32768) return -32768;
    return static_cast<std::int16_t>(sum);
}
void check() {
    std::mt19937 rng(20261015);
    std::vector<Kernel> kernels{add_scalar,add_auto};
#if defined(USE_AVX2) || defined(USE_SVE)
    kernels.push_back(add_simd);
    std::cout << "SIMD float lanes=" << simd_float_lanes() << '\n';
#endif
    for (Kernel fn:kernels)
        for (std::size_t n=0; n<=257; ++n)
            for (std::size_t offset=0; offset<8; ++offset)
                check_float(fn,n,offset,rng);
    for (Kernel fn:kernels) check_float(fn,65536,1,rng);
    const std::int16_t edge[]{-32768,-32767,-30000,-1,0,1,30000,32766,32767};
    for (auto a:edge) for (auto b:edge)
        require(sat_add16(a,b)==independent_sat(a,b),"scalar saturation mismatch");
#ifdef USE_AVX2
    std::uniform_int_distribution<int> d(-32768,32767);
    for (std::size_t n=0; n<=257; ++n) for (std::size_t offset=0; offset<16; ++offset) {
        auto a=std::make_unique<std::int16_t[]>(n+offset);
        auto b=std::make_unique<std::int16_t[]>(n+offset);
        auto out=std::make_unique<std::int16_t[]>(n+offset+1);
        std::fill_n(out.get(),n+offset+1,std::int16_t(123));
        for (std::size_t i=0;i<n;++i) {
            a[i+offset]=static_cast<std::int16_t>(d(rng));
            b[i+offset]=static_cast<std::int16_t>(d(rng));
        }
        // Include every pair of numerical edges as length permits.
        for (std::size_t i=0;i<std::min<std::size_t>(n,81);++i) {
            a[i+offset]=edge[i/9]; b[i+offset]=edge[i%9];
        }
        sat_avx2(a.get()+offset,b.get()+offset,out.get()+offset,n);
        for (std::size_t i=0;i<n;++i)
            require(out[i+offset]==independent_sat(a[i+offset],b[i+offset]),"AVX saturation mismatch");
        for (std::size_t i=0;i<offset;++i) require(out[i]==123,"integer prefix overwrite");
        require(out[n+offset]==123,"integer suffix overwrite");
    }
#endif
    std::cout << "PASS: finite float, lengths 0..257/65536, offsets, saturation reference\n";
}
Kernel select_kernel(const std::string& name) {
    if (name=="scalar") return add_scalar;
    if (name=="auto") return add_auto;
#if defined(USE_AVX2) || defined(USE_SVE)
    if (name=="simd") return add_simd;
#endif
    throw std::invalid_argument("unavailable kernel (scalar/auto/simd)");
}
std::size_t positive(const char* s,std::size_t upper) {
    const std::string value(s);
    if (value.empty() || value.find_first_not_of("0123456789")!=std::string::npos)
        throw std::invalid_argument("expected positive decimal integer");
    const auto x=std::stoull(value);
    if (x==0 || x>upper) throw std::invalid_argument("argument outside lab limit");
    return static_cast<std::size_t>(x);
}
void bench(const std::string& name,std::size_t n,std::size_t repetitions) {
    Kernel fn=select_kernel(name);
    std::vector<float> a(n,1.25f),b(n,-0.5f),out(n);
    for (int i=0;i<100;++i) fn(a.data(),b.data(),out.data(),n);
    std::cout << "kernel,n,repetitions,batch,ns_per_call,checksum\n";
    for (int batch=0;batch<7;++batch) {
        const auto start=std::chrono::steady_clock::now();
        for (std::size_t r=0;r<repetitions;++r) fn(a.data(),b.data(),out.data(),n);
        const auto end=std::chrono::steady_clock::now();
        double checksum=0;
        for (float x:out) checksum+=x;
        benchmark_sink=static_cast<float>(checksum);
        require(checksum==0.75*double(n),"benchmark output mismatch");
        const double ns=std::chrono::duration<double,std::nano>(end-start).count()/double(repetitions);
        std::cout << name << ',' << n << ',' << repetitions << ',' << batch << ',' << ns << ',' << checksum << '\n';
    }
}
int main(int argc,char** argv) {
    try {
        if (argc==2 && std::string(argv[1])=="check") check();
        else if (argc==5 && std::string(argv[1])=="bench")
            bench(argv[2],positive(argv[3],4194304),positive(argv[4],1000000));
        else throw std::invalid_argument("usage: demo check | demo bench scalar|auto|simd N repetitions");
    } catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
}
