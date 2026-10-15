# PUSCH 三个月达标学习与工作资料包

版本：2026-10-15 初版。适用对象：有通信信号处理和软件无线电实践经验，需要补齐 NR、工程 C++ 和 SIMD 的基站物理层工程师。

本资料包已编写；其中的公司任务、实验、代码优化与转正验收尚未执行。公司 ARM／x86 环境已由学习者确认存在，具体能力、权限及配置仍需核验。所有命令面向公司 Linux 环境，不提供 macOS 适配。

## 从这里开始

1. 先读 [总计划与考核映射](plan/01-master.md)，把制度和技术待确认事项交给主管、导师、HR分别核实。
2. 做 [基础诊断](plan/04-diagnostic.md)，按 [日历与启动任务](plan/02-calendar.md)执行前两周。
3. 阅读教程 T01、T02、T03，并在公司环境运行 [实验说明](labs/README.md)。
4. 第二周确认实际优化对象；用 [SVE专项](plan/05-sve-project.md)和 [模板](templates/records.md)记录全过程。
5. 每周更新 [证据索引](evidence/index.md)；未执行项维持待验证，原始结果和评审记录才能支撑完成状态。

## 文档导航

| 资料 | 用途 |
|---|---|
| [总计划](plan/01-master.md) | 目标、容量、月度里程碑、考核映射、确认清单 |
| [周计划与启动](plan/02-calendar.md) | 12个日历窗口、八项周字段、前两周逐日行动 |
| [原任务覆盖与风险](plan/03-coverage-risks.md) | 原六个月逐项对应、检查点、降速方案、转正后深化 |
| [诊断](plan/04-diagnostic.md) | 12题、3个实验、参考答案和学习调整 |
| [SVE专项](plan/05-sve-project.md) | 目标选择、关口、职责、系统验收 |
| [T01 工程C++](tutorials/T01-cpp.md) | 类型、内存、接口、生命周期、并发、构建 |
| [T02 SDR到NR](tutorials/T02-nr.md) | 资源栅格、时序、调度、MIMO、HARQ |
| [T03 性能测量](tutorials/T03-performance.md) | 基线、cycles、耗时、热点、函数到系统 |
| [T04 AVX与SVE](tutorials/T04-simd.md) | 向量化、intrinsics、尾部、定点、验证 |
| [T05 L1架构与Timing](tutorials/T05-l1.md) | 接收链、调用、线程、缓冲区、期限 |
| [T06 PUSCH协议](tutorials/T06-pusch.md) | 38.211／212／214的算例与代码映射 |
| [T07 测试与定位](tutorials/T07-test.md) | 用例矩阵、复现、问题分类、贯穿案例 |
| [T08 日志与调试](tutorials/T08-debug.md) | 日志、IQ、抓包、Trace、core、GDB |
| [T09 O-RAN CUS](tutorials/T09-oran.md) | 功能切分、三平面、前传窗口 |
| [T10 AI coding](tutorials/T10-ai.md) | 提示词、学习练习、验证与应用证据 |
| [公开来源与版本](references.md) | 官方资料、核验日期和项目版本登记 |
| [实验](labs/README.md) | 标量／自动向量化／AVX／SVE代码和Linux运行步骤 |
| [记录模板](templates/records.md) | 设计、学习报告、seminar、测试、性能、AI、周报 |
| [证据索引](evidence/index.md) | 考核状态、成果目录、证据和待确认指标 |
| [作者质量检查](QUALITY_CHECK.md) | 文档与通用实验检查，和公司目标验证的边界 |

## 使用约定

“入门”表示能解释概念和简单代码；“应用”表示能运行、修改和分析实验；“独立”必须有实际任务证据和评审。教程完成不能自动算作项目能力达标。

状态只用：计划、进行中、已验证、待确认、受阻。公司标准未提供时只列建议标准。每次更新写日期、变更原因、确认人；不把旧版本结果直接归给新版本。

术语在各专题首次出现时解释；内部名称 Aerxoway、DMAP、DEC、XC、DRT、LLS、nlog 均保留原词，不猜测全称。通用示例与项目实现明确区分。这里提供的是提前达标路径，公司是否允许提前办理转正另行确认。
# PUSCH_NR_ORAN
