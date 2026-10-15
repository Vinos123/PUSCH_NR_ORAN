# T03 Cycle测试、热点与系统性能

## 1. 任务关系与前置

优化必须先知道“当前慢在哪里”和“变化是否真实”。三个月必学全部基础；复杂微架构事件、NUMA（Non-Uniform Memory Access，非统一内存访问）调优及完整roofline模型后续深入。前置是会运行程序、理解循环和输入长度。

## 2. 指标不是同一回事

CPU cycle是处理器周期计数口径；wall time（墙钟耗时）是观察到的经过时间；CPU time是线程实际占用CPU时间。throughput（吞吐率）是单位时间处理量，latency（时延）是一次任务从约定起点到终点的时间；deadline（期限）是必须完成的时刻。函数ns/element、slot关键路径和目标多小区持续吞吐分别测。

x86 TSC（Time Stamp Counter，时间戳计数器）在很多平台以参考频率推进，不应直接标成核心cycles；ARM generic timer（通用计时器）计数也不是执行周期。PMU的cycles事件有硬件/权限/统计范围限制。不要用标称GHz简单将任意计数换成实测核心周期。

速度倍数=baseline/optimized；耗时下降百分比=(baseline-optimized)/baseline×100%。教学例100ns到80ns是1.25倍、下降20%，两个表述不同。若热点占总串行耗时20%，热点快2倍，Amdahl估计上限为1/(0.8+0.2/2)=1.111；这只是串行同条件模型，实时系统还取决于关键路径和等待。

## 3. 可信基线的步骤

先固定版本、编译器、优化参数、CPU、数据、频率/功耗策略、NUMA/绑核、测试负载和统计边界，再验证正确性。预热代码和数据，测一批而非单次；重复独立批次，保留中位数和离散度，也保存最差/高分位。microbenchmark（微基准）中位数不能代表系统最坏时延。

本教程demo用steady_clock（单调时钟）输出批次ns/调用；warmup不计时，输出校验和让结果被消费。函数在独立翻译单元，无LTO（Link Time Optimization，链接时优化）；标量参考显式禁用自动向量化，另有自动向量化版本。此实验以热缓存为主，另需用多个工作集和真实数据测缓存压力。

函数调用和循环开销会支配n很小的结果。可运行空操作对照估计开销，但不机械减去两个噪声数。过多PMU事件可能multiplex（复用计数）并被缩放；先少量事件，核对统计质量。测试框架本身耗时也要分开。

## 4. Linux实验与代码阅读

公司Linux先运行[labs](../labs/README.md)相应模式，正确性通过后：

```bash
lscpu
g++ --version
perf list
taskset -c 2 ./labs/build/avx2/demo bench simd 65536 2000
perf stat -r 5 -e cycles,instructions,task-clock,context-switches,cpu-migrations \
    taskset -c 2 ./labs/build/avx2/demo bench simd 65536 2000
perf record -g -- taskset -c 2 ./labs/build/avx2/demo bench simd 65536 2000
perf report
```

示例CPU2必须替换成允许的核心，路径/模式按环境改。`perf stat`统计整个进程，包括初始化、自检（若执行）和预热，不直接等于kernel cycles；此bench模式不跑完整check，但仍有初始化/预热/打印。增加测量批次使这些占比下降，正式函数cycle需项目插桩或限定统计区间。不要把混合运行多个kernel的结果当单个kernel数据。[perf-stat工具手册](https://man7.org/linux/man-pages/man1/perf-stat.1.html)说明统计选项。

读热点代码问三件事：是否计算受限、带宽/缓存受限、等待/同步受限。IPC（Instructions Per Cycle，每周期指令数）不是独立成绩；低IPC可能是依赖或cache miss，高IPC也可能执行了太多无用指令。采样栈要有匹配符号，内联/优化会影响呈现。平台权限不够交由负责人，不自行改系统全局策略。

## 5. 比较与正确性

交替运行baseline和新版本，控制温度和其他负载；不要只挑最好一次。长度至少包括13、典型项目长度和大工作集。每个比较保存原始批次、计时范围、版本、输入hash、构建命令和correctness状态。增加线程数不能隐藏单任务期限问题。

教学加法读取两路、写一路float，逻辑流量约12字节/元素；实际内存流量受cache、write allocate等影响。达到缓存或内存带宽上限后，更多算术SIMD未必再快。小输入更可能受调用与尾部开销影响，报告按长度分组。

实际L1验证固定单小区及目标多小区配置，记录slot耗时分布、超期、吞吐、CRC/BLER（Block Error Rate，块错误率）、IQ丢失和资源占用。性能变好但BLER变坏不能接受；阈值与持续时长必须由公司确认。

## 6. 常见问题

第一次很慢：初始化/冷cache，不能混入稳定态。重复忽快忽慢：查频率、迁核、温度、干扰和数据集；写原始数据。perf cycles不可用：先记录墙钟和环境，cycles待验证。函数收益系统消失：查看关键路径占比、等待、跨核/NUMA和其他热点；不承诺继续同一函数就能解决。

在仿真环境或不同CPU上测到加速，只能证明该环境结果；目标多小区仍需目标硬件。小幅收益落在噪声内，结论是证据不足，增加控制与重复，不宣称达标。

## 7. 自测与标准

问：100→80ns是快20%还是1.25倍？答：耗时减少20%，速度1.25倍。问：TSC差值就是核心cycles吗？答：需确认口径，通常不能直接等同。问：函数快2倍为何系统可能不变？答：占比、关键路径、等待和共享资源不同。

入门：准确区分四种指标。应用：运行同平台三版本对照并解释噪声、自动向量化、cache。独立：真实热点、回归、单/多小区和限制形成完整报告，公司确认统计口径与阈值。
