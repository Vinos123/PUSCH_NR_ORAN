# 公开来源、教学假设与项目版本登记

核验日期：2026-10-15。以下选的是明确版本参考资料，非声称最新版本，非公司采用版本。标准以官方PDF的条件、公式和表为准；正文是自主教学解释及限定练习，不复制整份规范。

| 来源 | 教学用途 | 项目状态 |
|---|---|---|
| [TS 38.211 V17.9.0 Rel-17](https://www.etsi.org/deliver/etsi_ts/138200_138299/138211/17.09.00_60/ts_138211v170900p.pdf) | 第4章、§6.3.1、§6.4.1.1、§6.4.1.2资源/信号/参考 | Release/版本待确认 |
| [TS 38.212 V17.9.0 Rel-17](https://www.etsi.org/deliver/etsi_ts/138200_138299/138212/17.09.00_60/ts_138212v170900p.pdf) | §6.2及第5章UL-SCH编码/速率匹配，相关UCI | 版本/支持模式待确认 |
| [TS 38.214 V17.9.0 Rel-17](https://www.etsi.org/deliver/etsi_ts/138200_138299/138214/17.09.00_60/ts_138214v170900p.pdf) | 第6章，§6.1.4.2引用§5.1.3.2步骤2—4/TBS表 | 版本/配置待确认 |
| [TS 38.213 V17.9.0 Rel-17](https://www.etsi.org/deliver/etsi_ts/138200_138299/138213/17.09.00_60/ts_138213v170900p.pdf) | §4.2、§7.1、§9.3时间调整/功控/UCI关联 | 版本待确认 |
| [ETSI TS 103 859 V12.0.1 / CUS v12.01](https://www.etsi.org/deliver/etsi_ts/103800_103899/103859/12.00.01_60/ts_103859v120001p.pdf) | 三平面、7-2x、§4.4延迟管理 | CUS/profile/RU类别待确认 |
| [Arm ACLE](https://arm-software.github.io/acle/main/acle.html) | SVE类型、谓词、intrinsics语义 | 工具链支持及版本待核 |
| [GCC AArch64选项](https://gcc.gnu.org/onlinedocs/gcc/AArch64-Options.html) | SVE/scalable目标与编译条件 | 公司GCC版本待核 |
| [Intel Intrinsics Guide](https://www.intel.com/content/www/us/en/docs/intrinsics-guide/index.html) | AVX/AVX2指令和数据类型 | CPU/OS能力待核 |
| [LLVM向量化](https://llvm.org/docs/Vectorizers.html) | 自动向量化及诊断 | 公司编译器实际flag待核 |
| [GDB文件/符号](https://www.sourceware.org/gdb/current/onlinedocs/gdb.html/Files.html) | core、binary、符号和库 | 项目收集流程待核 |
| [AddressSanitizer](https://clang.llvm.org/docs/AddressSanitizer.html) | 离线地址检查 | 平台/工具链支持待核 |
| [ThreadSanitizer](https://clang.llvm.org/docs/ThreadSanitizer.html) | 并发检查 | 支持与项目使用方式待核 |
| [perf-stat手册](https://man7.org/linux/man-pages/man1/perf-stat.1.html) | PMU/进程统计、重复和事件 | 事件/权限待核 |

## 版本切换流程

导师提供项目Release/各TS版本/CUS/profile→登记文件名和内部批准位置→核对目标条款、表及参数变化→重算案例→检查代码/测试映射→报告注明新版本。不要只改页眉的Release数字而保留旧表。首次使用其他规范如38.331/38.321时核验相应项目版本和条款，再增加来源登记。

## 内部资料登记（当前未获得）

| 名称 | 待提供内容 | 真实路径/版本/权限/owner |
|---|---|---|
| L1 Timing、架构、PUSCH外部接口 | 图、参数、调度和deadline | 待导师提供 |
| DMAP/DEC | Design、代码入口、buffer和参考 | 待提供；不猜全称 |
| XC/DRT/LLS | 测试脚本、配置、数据、输出和标准 | 待提供；不猜角色 |
| Aerxoway/nlog | 定义、文档、工具和实验 | 待提供；原词保留 |
| logs/dump/PCAP/Trace/core | 获取、格式、关联、符号和实际案例 | 待提供 |
| 岗位/AI/性能标准 | 正式list、等级、配置、阈值和流程 | 待主管确认 |
