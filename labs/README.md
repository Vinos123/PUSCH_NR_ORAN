# 公司Linux ARM/x86实验

所有实验是教学练习，非已经完成的公司工作。先确认CPU、OS、GCC版本、指令支持和权限；没有macOS构建分支。脚本要求Linux、默认GCC的g++，C++17；若部门用Clang需按其选项转换，不能直接把GCC诊断flag套上。

## 1. 实验顺序和代码

`kernels.h`定义长度/所有权契约；`scalar.cpp`是禁用自动向量化的加法基线及饱和规则；`auto.cpp`使用正常优化；`avx2.cpp`做8个float并行加法和16个int16饱和加法；`sve.cpp`是VLA float加法；`main.cpp`做独立参考、精确右边界/偏移检查和分kernel基准。

步骤：先手算及解释标量→正确性→基线→自动向量化诊断→intrinsics→边界→同平台性能→真实PUSCH迁移。没有真实项目接口时不能把这个加法叫PUSCH优化。

## 2. 公司环境编译运行

```bash
cd labs
lscpu
g++ --version
bash build-linux.sh portable
./build/portable/demo check
```

x86上CPU/OS确认支持AVX2后：

```bash
bash build-linux.sh avx2
./build/avx2/demo check
./build/avx2/demo bench scalar 65536 2000
./build/avx2/demo bench auto 65536 2000
./build/avx2/demo bench simd 65536 2000
objdump -d build/avx2/demo > build/avx2/disassembly.txt
```

ARM SVE上：

```bash
bash build-linux.sh sve
./build/sve/demo check
./build/sve/demo bench scalar 65536 2000
./build/sve/demo bench auto 65536 2000
./build/sve/demo bench simd 65536 2000
objdump -d build/sve/demo > build/sve/disassembly.txt
```

`/proc/cpuinfo`检查只是初筛，不能替代平台对OS/ISA支持的确认。ISA构建的可执行文件只能在支持CPU运行，没有生产级运行时分发。主程序和ISA实现分对象，baseline也使用相同目标ISA条件但禁用向量化；无LTO，不加fast-math。`compiler.txt`与`vectorization.txt`保存工具链及自动向量化判断，原始命令需另记录。

check的预期为PASS并在SIMD模式打印float lane数。覆盖有限float值、0—257/65536长度及多组偏移，AVX2另覆盖整数饱和边界；不验证任意浮点NaN/Inf语义，不实现SVE整数饱和示例，不证明全部项目输入。缺支持则明确失败/受阻，不用portable PASS替代ISA验证。

## 3. 检查器与测量

portable离线检查：

```bash
g++ -std=c++17 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer \
    scalar.cpp auto.cpp main.cpp -o build/check-asan
./build/check-asan check
```

AVX2检查可给同一命令添加`-mavx2 -DUSE_AVX2 avx2.cpp`；SVE添加`-march=armv8.2-a+sve -msve-vector-bits=scalable -DUSE_SVE sve.cpp`。检查器支持依工具链；检查器耗时不可作性能结果。

perf示例（替换允许核心）：

```bash
perf stat -r 5 -e cycles,instructions,task-clock,context-switches,cpu-migrations \
    taskset -c 2 ./build/sve/demo bench simd 65536 2000
```

bench每次只选一个kernel，100次预热、7批计时，输出CSV。耗时范围只包含调用循环；校验和在计时外，验证数据输出；perf统计仍包含整个程序。数组重复使用属于热缓存测试；大工作集/真实IQ另做。小输入调用开销、高分位和系统deadline需独立理解。不自动给出提升目标，也不承诺SIMD更快。

## 4. 其他练习

```bash
python3 nr_example.py
python3 timing_check.py timing_example.csv
```

NR例子预期TBS1320/B1336/BG2，但仅提供限定表片段，不是完整标准实现。时序例子含一条超期：预期第二行margin=-40μs，进程退出1是教学检测结果；解析/时间异常退出2。

`iq_inspect.py`只接受已确认的无头小端int16交织I/Q文件；`race_demo.cpp`仅用于独立TSan练习；`crash_demo.cpp`有意abort，仅在批准实验环境使用。均不直接用于生产。

## 5. 练习记录与验收

填[测试模板](../templates/records.md)：版本、CPU/ISA/VL、输入、命令、真实结果和限制。区分作者本地资料检查与公司目标运行。完成应用阶段需要在公司CPU真正运行；独立阶段需要真实PUSCH项目与多小区结果。
