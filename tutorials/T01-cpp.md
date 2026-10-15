# T01 工程C++：从PHY代码阅读到可靠修改

## 1. 为什么学、先学什么

PHY（Physical Layer，物理层）优化不是只换几条指令。地址失效、整数溢出和并发竞争都会使算法看似正确却偶发失败。三个月必学：本章全部基础，尤其接口、生命周期、数值和边界。复杂模板元编程、继承体系、无锁队列实现遇到再补。

前置只需会写循环和知道IQ数组。先能描述“这个buffer是什么类型、谁拥有、多少元素、何时有效”，再修改热点。阅读目标是把每个函数写成输入→计算→输出的契约。

## 2. 类型、整数与内存原理

`std::int16_t`若平台提供就是16位有符号整数，范围[-32768,32767]；`std::uint16_t`为[0,65535]。`std::size_t`适合合法对象的元素数/字节数，通常无符号；用它做倒序循环要防下溢。`int`不是所有平台都保证32位，不把协议字段或文件格式直接写成int。

表达式先计算再赋值：`int32_t z = int32_t(a) * int32_t(b)`让乘法在宽类型执行；`int32_t z = a*b`是否安全还取决于整数提升及int宽度。32位乘32位需分析是否要64位。超过有符号类型范围的运算是 UB（Undefined Behavior，未定义行为）；无符号运算按模环绕，但不一定符合算法。窄化转换不等于饱和，不能靠`static_cast<int16_t>`实现clip。

```cpp
std::int16_t sat_add16(std::int16_t a, std::int16_t b) {
    const std::int32_t s = std::int32_t(a) + std::int32_t(b);
    const auto clipped = std::max<std::int32_t>(-32768,
                         std::min<std::int32_t>(32767, s));
    return static_cast<std::int16_t>(clipped);
}
```

这是教学规则：加法饱和。项目可能要求回绕、先移位再舍入或其他顺序，必须确认。对Q15（15位小数的定点表示）乘法，两个int16数乘积可用int32保存，但复杂乘法中两项相加/相减可能超过int32，端点应考虑int64。负数右移及舍入规则要与项目标准和语言版本一致，不凭直觉。

指针存地址，引用通常用于表达“已有对象的别名”。`float* p`的`p+i`按元素移动；只能在同一数组及其末尾后一位范围内做相应指针运算，末尾后一位不可解引用。指针非空也不能证明长度足够。C风格数组不会携带长度，函数必须接收`n`或受控view。下面的接口是教学示例：

```cpp
void add(const float* a, const float* b, float* out, std::size_t n);
// a/b各至少n元素，out至少n元素；数据均在调用期间有效。
// 本教程约定输出与输入不重叠；n=0不解引用。
```

AoS（Array of Structures，结构数组）如`IQ{I,Q}`连续存IQ对；SoA（Structure of Arrays，数组结构）分别存I和Q。向量化取决于实际访问模式，不能为了SIMD擅改外部格式。`sizeof(struct)`包含padding（填充），字段顺序/ABI（Application Binary Interface，应用二进制接口）会影响布局；不能用裸struct写网络报文，需明确字节序和字段编码。

## 3. 接口、生命周期、并发与构建

`struct`默认public，`class`默认private；二者都可以有构造函数和方法。头文件给声明、类型和必要模板定义，源文件给普通函数定义。编译把每个源文件变成对象文件，链接解析函数/变量引用；重复定义与未定义引用是不同问题。模板需在实例化处可见；宏是文本替换，先看宏展开和`#if`分支，确认所读的是实际构建分支。

`const float*`限制通过该指针修改元素；`float* const`限制指针本身改变。const不阻止其他别名修改，不提供线程同步。返回局部对象地址、保存临时对象引用、vector扩容后使用旧`data()`都是生命周期风险。

RAII（Resource Acquisition Is Initialization，资源获取即初始化）让对象构造/析构管理资源。`std::vector`拥有内存，`std::unique_ptr`表达独占所有权；裸指针在高性能接口中可以只是借用，仍须写清所有者和归还时刻。实时热路径通常不希望每slot分配；预分配和buffer池是项目策略，RAII并不要求热路径频繁分配。

多线程中两个冲突访问、至少一个写且无适当同步，会出现data race（数据竞争）。`volatile`不是线程同步。mutex（互斥锁）保护完整状态，atomic（原子操作）保护指定变量且需要正确内存顺序。例如“先写buffer，后发布ready”可以在严格单生产者/消费者协议中使用release/acquire，但复用、所有权转移和计数仍须完整设计，不能一个atomic标志解决整个环形队列。false sharing（伪共享）是不同线程更新同一cache line导致一致性开销，即使没有数据竞争也会变慢。

对齐是地址满足某个字节倍数，不意味着可访问更多元素。`alignas(64)`给静态/对象对齐，动态数组按相应分配机制处理。严格别名规则禁止随意把float存储作为int指针读写；C++17可用`memcpy`复制位表示。加载intrinsics使用其规定的类型/接口，不把任意cast当作消除别名问题的许可证。

## 4. 代码阅读方法

在公司仓库先读AGENTS.md及构建说明。只读追踪示例：

```bash
rg --files -g AGENTS.md -g '*pusch*' -g '*Pusch*'
rg -n '目标函数名' src tests
rg -n '#if|#define|typedef|using ' path/to/target.cpp
```

路径是占位，不是当前项目事实。依次检查声明、调用者、长度单位、数据布局、数值范围、线程、失败返回及释放。再找测试参考输出。用构建的实际宏/编译参数确认AVX/SVE分支；必要时用预处理输出理解宏（仅在独立实验目录生成）。记录“事实/推断/待确认”三栏。

## 5. 可操作实验

从[labs](../labs/README.md)运行portable自检，逐句解释饱和函数，再在副本增加一个边界测试。公司Linux、C++17、GCC示例：

```bash
cd labs
bash build-linux.sh portable
./build/portable/demo check
g++ -std=c++17 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer \
    scalar.cpp auto.cpp main.cpp -o build/check-asan
./build/check-asan check
```

ASan（AddressSanitizer，地址检查器）与UBSan（UndefinedBehaviorSanitizer，未定义行为检查器）用于离线正确性，不用其耗时作为优化收益。并发小实验单独编译`race_demo.cpp`并比较`unsafe`/`safe`：

```bash
g++ -std=c++17 -O1 -g -fsanitize=thread -pthread race_demo.cpp -o build/race-tsan
./build/race-tsan unsafe
./build/race-tsan safe
```

TSan（ThreadSanitizer，线程检查器）与ASan使用独立构建。预期unsafe可能报竞争，safe不应有对应竞争；计数偶然正确不能证明unsafe安全。工具/平台不支持则记录环境限制，不伪造通过。[ASan](https://clang.llvm.org/docs/AddressSanitizer.html)、[TSan](https://clang.llvm.org/docs/ThreadSanitizer.html)提供官方使用要求。

## 6. 常见问题与定位

优化版本偶发崩溃：先检查长度、尾部、对齐假设、buffer复用，再看算法。链接找不到函数：核对声明/定义签名、命名空间、参与链接对象、宏分支及C/C++ linkage。debug正常release异常：优先排查UB、初始化、竞争及对象生命周期，不先归咎编译器。模板报错：从首次实例化和最早错误看起，不盲改大量类型。

实时程序加锁后性能恶化：确认是否热路径、锁竞争和阻塞，先遵循项目调度/所有权设计；不在基础不足时贸然自写无锁替代。检查器无报告只能说明已覆盖路径没有捕获，不能证明所有输入安全。

## 7. 自测与三级完成标准

问：为什么`const`不保证线程安全？答：它只限制该路径的修改，其他线程仍可能写同一存储，需要同步。

问：对齐32字节的n=3数组能加载8个float吗？答：不能，必须保证实际可访问范围；对齐不改变长度。

问：如何解释一个PHY函数？答：列类型、长度单位、布局、数值、所有权、并发、返回值和测试，再解释每个计算阶段。

入门：正确解释上述接口和饱和例子。应用：运行检查器、增加边界测试、解释构建和链接。独立：真实模块设计/修改通过项目评审，附数值和生命周期证据；诊断及练习不代替此证据。
