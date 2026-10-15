# T08 日志、Signal Dump、抓包、Trace与core

## 1. 为什么学、边界和前置

优化后的失败可能来自参数、数据、数值或运行时序，需要多种证据定位。三个月必学获取、关联、最小分析和符号匹配；复杂内存破坏、多节点现场和完整内部工具后续深化。前置T01内存、T05链路。

vDU log是虚拟DU的软件日志；Signal Dump是信号/中间数据转储；Real Case是真实案例，不是具体工具。nlog保留内部原词，格式、全称和获取方式待导师说明。PCAP（Packet Capture，数据包捕获文件）保存包及捕获信息，Trace log记录事件/任务过程，core dump保存崩溃现场的部分内存与寄存器。各自观察范围不同。

## 2. 获取方式与关键信息

| 证据 | 获取路径（需按项目确认） | 首要字段/限制 |
|---|---|---|
| vDU log | 项目脚本、容器日志或systemd日志 | 版本、时间域、cell/UE/frame/slot、模块、严重性；轮转和异步写入 |
| Signal Dump | 批准的触发条件/采样点/导出工具 | 阶段、天线/层、长度、I/Q布局、端序、位宽、缩放、是否压缩 |
| Real Case | 导师提供案例包和问题单 | 场景、预期、复现、原始输入、限制及处理记录 |
| nlog | 内部说明指定的导出/解码器 | 工具/格式版本、事件定义和时间单位；不能当文本随意解析 |
| PCAP | 批准接口抓取或已有文件 | 捕获点、时钟、eAxC/流标识、序列、长度、抓包丢失统计 |
| Trace | 项目低开销trace工具 | 任务开始/结束/排队、线程、依赖、参考时刻和deadline |
| core | 项目崩溃收集、systemd-coredump或受控实验 | 正确二进制/符号/库、build ID、崩溃信号、线程及映射 |

不要默认某个journal unit或容器名。先记录获取命令由谁确认、保存位置、保留范围和权限。抓取内部数据只在批准位置保存。

## 3. 时间与身份关联

关联key至少包含构建、cell、UE标识、帧/slot及相关HARQ状态；前传流标识还需映射天线/载波等，不能直接当UE ID。UE标识可被复用，frame/slot也会回绕，不可仅按slot连接所有记录。

wall clock、monotonic（单调时钟）、硬件PTP（Precision Time Protocol，精确时间协议）时间和无线帧参考可能不同。先用项目锚点得到转换关系/误差，再比较先后；记录timestamp是事件发生时还是写log时。正常CP μ=1下frame20slot，可以建立扩展slot计数，但必须在回绕附近有上下文，不能跨丢失很长区间盲推。

将事件排序为配置到达→IQ→任务→解码→上报，比较缺失、重复和延迟。PCAP缺包也可能是抓取端丢失，不直接判定网络丢包；抓包显示包到NIC时间，也不等于L1处理该IQ的时刻。

## 4. 代码阅读与实操

找log/trace字段的定义和写入位置，确认单位和触发条件；找dump producer和消费者格式，检查大小、压缩和padding。日志error文本是线索，不能替代根因。

公司Linux通用只读分析示例，实际unit/路径/接口由项目替换：

```bash
journalctl -u APPROVED_UNIT --since '2026-10-15 09:00:00' --until '2026-10-15 09:10:00'
rg -n 'PUSCH|CRC|late|timeout' case.log
tshark -r approved_case.pcap -c 10
capinfos approved_case.pcap
```

不要猜tshark字段名；先`tshark -G fields`搜索安装版本支持的eCPRI/O-RAN字段，再建立显示过滤。抓包命令仅在批准实验环境使用，如`tcpdump -i APPROVED_IFACE -s 0 -c 1000 -w case.pcap`，不得直接对生产流量执行。正式性能测试避免抓包/高频log改变负载。

IQ教学文件格式限定为“无头、little-endian、int16交织I/Q、未压缩”，真实dump未经格式确认不能用：

```bash
python3 labs/iq_inspect.py approved_iq.bin
```

脚本检查4字节复数边界，输出样本数、峰值和饱和值数量，不宣称解码。转成物理幅度还要缩放规则。频域IQ与时域IQ的能量/图形含义不同，压缩前后也不能直接比较原字节。

## 5. GDB：匹配后再相信变量

GDB（GNU Debugger，GNU调试器）需要和core对应的可执行文件、debug symbols（调试符号）及共享库。先检查build ID和部署版本，再分析。仅源码路径相同不够；同名重编译文件也不匹配。

```bash
file approved_binary approved_core
readelf -n approved_binary
gdb -q approved_binary approved_core
```

GDB交互命令：

```text
info files
info sharedlibrary
info threads
thread apply all bt
thread 3
bt full
frame 0
info args
info locals
p length
info registers
x/16hx buffer
```

线程3、length、buffer都是例子，先从真实栈确认。优化构建变量可能`optimized out`，inline帧或尾调用使栈不同；指针已损坏时其值不代表原始参数。core可能不含全部页，未读到数据不说明源程序没分配。可用`set sysroot`、`set solib-search-path`及`set substitute-path`定位正确库/源码，不能拿不匹配文件“补全”栈。官方[文件与符号说明](https://www.sourceware.org/gdb/current/onlinedocs/gdb.html/Files.html)可核对。

离线实验用`g++ -std=c++17 -O0 -g labs/crash_demo.cpp -o labs/build/crash-demo`，在批准环境用GDB运行并触发`abort`，看线程/帧/变量；它只练现场查看，不伪装成业务缺陷。需要core时仅在实验shell确认策略后设置`ulimit -c unlimited`，再查`/proc/sys/kernel/core_pattern`或团队收集方式。工具可用性需公司环境实测。

## 6. 常见问题与方法

日志关联不上：查时间域、回绕、UE复用和异步写入。dump全异常：查格式、端序、压缩、单位、阶段，不先怪算法。core栈很怪：匹配符号/库，再检查栈损坏。只看崩溃线程：其他线程可能持锁或写坏buffer，必须看全线程。日志表明晚到：结合PCAP、队列Trace找最早发生的延迟，不能直接归因网络。

## 7. 自测与三级标准

问：PCAP缺包就等于网络丢包吗？答：不一定，还可能捕获丢失。问：同名binary足够匹配core吗？答：不够，核对构建ID和部署。问：int16 dump出现许多32767说明什么？答：可能饱和，也可能格式错，先核格式再联系缩放。

入门：说清七类证据和时间域。应用：分析合成时序、合法IQ格式和受控GDB现场。独立：按批准流程获取真实vDU log，关联一个Real Case并给根因/验证；没有真实资料保持待验证。
