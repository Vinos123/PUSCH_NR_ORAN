# T04 从标量到AVX与ARM SVE

## 1. 为什么学与学习顺序

SIMD（Single Instruction Multiple Data，单指令多数据）让一条指令处理多元素。intrinsics（编译器内建指令接口）用C/C++形式表达向量操作，实际指令和调度仍要看汇编。三个月必学加载/存储、布局、尾部、数值和验证；复杂gather/scatter、跨lane排列、AVX-512、SVE2和特定微架构深入按需继续。

前置：T01类型/内存和T03基线。顺序不能倒置：解释标量→独立参考正确→基线→自动向量化→简单intrinsics→边界和数值→性能→项目热点。手写SIMD并不天然胜过编译器。

## 2. 固定向量与可伸缩向量

AVX（Advanced Vector Extensions，高级向量扩展）使用256位向量等形式；256位可容纳8个32位float。AVX2扩展256位整数处理；AVX-512能力又需单独确认，掩码能力不能套给所有AVX指令。OS也要支持相应寄存器状态。SVE（Scalable Vector Extension，可伸缩向量扩展）让同一算法适配实现/线程的有效向量长度，不写死元素数；SVE支持不代表SVE2支持。

SVE的predicate（谓词）控制哪些lane（向量元素位置）有效。VLA（Vector-Length Agnostic，向量长度无关）循环以`svcntw()`得到32位元素数，`svwhilelt_b32(i,n)`启用仍在边界内的元素。不要把运行时向量类型随意当固定大小C++数组成员；持久存储用普通元素buffer。官方接口依据[Arm ACLE](https://arm-software.github.io/acle/main/acle.html)，编译选项见[GCC AArch64](https://gcc.gnu.org/onlinedocs/gcc/AArch64-Options.html)。

## 3. 同一个浮点加法的三种写法

教学契约：float为平台32位浮点，两路输入至少n个元素，输出至少n个；输出与输入不重叠。操作为逐元素`out[i]=a[i]+b[i]`，没有归约、FMA或跨元素依赖。有限测试数据可以逐元素比较；项目浮点算法需另外定义NaN、舍入、容限和累计误差。

```cpp
// 标量对应关系：每个i独立做一次加法
for (std::size_t i=0; i<n; ++i) out[i]=a[i]+b[i];
```

自动向量化用相同循环、开启优化和目标ISA（Instruction Set Architecture，指令集架构）。检查GCC的`-fopt-info-vec-optimized -fopt-info-vec-missed`或Clang的`-Rpass=loop-vectorize -Rpass-missed=loop-vectorize`；未向量化可能因别名、依赖或收益判断。`restrict`不是标准C++关键字；编译器扩展须写清不重叠契约，不能为消警告随意添加。

AVX例子，头文件`<immintrin.h>`，编译至少支持AVX，实验统一用`-mavx2`以覆盖整数练习：

```cpp
std::size_t i=0;
for (; n-i>=8; i+=8) {
    const __m256 x=_mm256_loadu_ps(a+i); // 读8个float，无32字节对齐要求
    const __m256 y=_mm256_loadu_ps(b+i);
    _mm256_storeu_ps(out+i, _mm256_add_ps(x,y)); // 8次独立float加法
}
for (; i<n; ++i) out[i]=a[i]+b[i]; // 最多7个尾元素
```

`loadu`允许合法的非对齐地址，但不允许越界。`n-i>=8`在保持i≤n的循环中避免`i+8`溢出。`__m256`是8个float，`__m256i`表示整数向量；不同整数intrinsic解释不同元素宽度。参考[Intel Intrinsics Guide](https://www.intel.com/content/www/us/en/docs/intrinsics-guide/index.html)，性能表不能替代目标CPU测量。

SVE例子，头文件`<arm_sve.h>`，目标为支持SVE的AArch64 Linux：

```cpp
for (std::size_t i=0; i<n; i+=svcntw()) {
    const svbool_t pg=svwhilelt_b32(i,n);
    const svfloat32_t x=svld1_f32(pg,a+i);
    const svfloat32_t y=svld1_f32(pg,b+i);
    svst1_f32(pg,out+i,svadd_f32_x(pg,x,y));
}
```

每步读/加/写当前有效元素。`_x`表示非活动结果不用于此处的逻辑，最终store也受同一pg保护；不能把未定义的inactive lane当有效结果做无掩码归约或写出。内存长度受真实数组限制。完整可运行版本在[labs](../labs/README.md)。编译参数示例为`-march=armv8.2-a+sve -msve-vector-bits=scalable`，具体CPU调优由平台确认。ARM支持NEON（Advanced SIMD固定长度扩展）不等于SVE。

## 4. 定点、饱和与布局

LLR常用有限位宽保存，不能把浮点加法练习直接当真实解调或HARQ算法。标量饱和加法先int32相加再clip；AVX2 `_mm256_adds_epi16`一次处理16个有符号int16并饱和，普通`_mm256_add_epi16`是环绕语义。尾部必须执行相同饱和规则。乘法要确认乘积位宽、移位、舍入、夹限的先后；某些narrow/pack操作还会改变lane顺序。

SVE定点方案先写“宽化→计算→明确舍入/clip→窄化”的标量契约，再从目标SVE能力选接口。不假定AVX饱和指令存在一对一SVE映射；SVE和SVE2分别核实。数值验证用极值、符号、阈值±1和随机参考；不能只喂正常幅度正数。

复数IQ的AoS布局可能需要deinterleave（解交织）；SoA可以连续访问，但改布局有搬运成本和接口影响。先评估整个模块，不为省一条指令新增一次大拷贝。对齐收益依CPU和访存模式，`loadu`可先作为正确基线。

## 5. 操作实验、代码阅读与项目迁移

在公司Linux先portable，再相应模式：

```bash
cd labs
bash build-linux.sh avx2   # x86且CPU/OS确认支持AVX2
./build/avx2/demo check
./build/avx2/demo bench scalar 65536 2000
./build/avx2/demo bench auto 65536 2000
./build/avx2/demo bench simd 65536 2000
```

ARM上把模式换`sve`。检查输出有效向量元素数，再做0、1、7、8、9、13、31、32、33和向量长度周边用例；示例自检覆盖连续0—257长度及多组随机数据，可在大多数当前SVE有效长度下覆盖其周边。另需公司真实最大规模。保持ASan/UBSan离线构建，性能只测release。

查看`objdump -d`或`g++ -S`结果：标量是否真无向量、自动循环是否使用目标向量、SVE是否有谓词控制。项目迁移前填写数值契约和长度布局，使用真实标量/已验收参考，不以自己另写的同错误SIMD作为oracle（参考判定实现）。正式项目保留回退分支和支持平台分发，按团队设计实施。

## 6. 常见问题

尾部崩溃：full load跨界、i单位错或predicate粒度错。只在极值错：环绕替代饱和、中间位宽不足、移位/round顺序错。只在另一台ARM错：写死长度或SVE2误用。更慢：小输入、搬运、排列、带宽上限、频率和函数调用；先看真实热点与汇编。没有手写向量也很快：自动向量化可能已经足够。

对齐数组通过但非对齐失败：检查是否用了需要对齐的加载。测试buffer有额外空间可能掩盖过读，自检要使用精确尾边界，另用项目guard page（保护页）用例或检查器。测试通过不是所有输入数学证明。

## 7. 自测与三级标准

问：13个float的AVX尾部多少？答：5。问：SVE如何知道一次处理多少？答：对应类型的运行时计数，本例svcntw。问：普通16位加法能代替饱和吗？答：不能。问：SVE `_x`结果的inactive lane可随意使用吗？答：不能，本例只存有效lane。

入门：逐句解释两份加法及向量元素。应用：运行正确性、自动向量化/汇编和同平台计时。独立：实际热点实现通过数值、边界、回归和系统验收，报告不夸大函数收益。
