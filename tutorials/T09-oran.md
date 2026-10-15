# T09 O-RAN WG4 CUS与PUSCH前传

## 1. 为什么学与版本

PUSCH高PHY要拿到正确IQ和参数，还要及时拿到。O-RAN（Open Radio Access Network，开放无线接入网）WG4（Work Group 4，第四工作组）CUS（Control, User and Synchronization planes，控制、用户和同步平面）定义相关前传交互。三个月必学三平面、项目功能切分、UL消息关联和时间窗口；全部section类型、压缩/互通及扩展功能后续按项目范围深化。

教学采用[ETSI TS 103 859 V12.0.1，对应CUS v12.01](https://www.etsi.org/deliver/etsi_ts/103800_103899/103859/12.00.01_60/ts_103859v120001p.pdf)。项目版本、RU类别和profile待确认；不能用参考版本替项目作合规判断。前置T02、T05及基础网络报文。

## 2. 三平面与功能切分

O-DU（O-RAN Distributed Unit，分布式单元）与O-RU（O-RAN Radio Unit，无线单元）之间的fronthaul（前传）在7-2x低层功能切分下承载控制和频域IQ等。上行CP移除/FFT通常在RU侧，高PHY在DU侧；波束/天线处理还需按类别确认，不能从通用图推实际模块。

C-plane（Control plane，控制面）告诉RU何时、在哪些频率/天线相关资源做相应传输/接收，例如UL section资源安排；U-plane（User plane，用户面）携带IQ样本，上行主要由RU到DU；S-plane（Synchronization plane，同步面）提供频率/时间同步关系。S-plane不等于每slot软件tick，控制包也不是用户数据包。

eCPRI（enhanced Common Public Radio Interface，增强通用公共无线接口）等承载机制有自己的消息、序号和流标识。eAxC（extended Antenna-Carrier，扩展天线载波标识）按配置标识前传流，不能直接解释成UE。PTP和SyncE（Synchronous Ethernet，同步以太网）分别涉及时间/频率同步方案，项目具体实现和精度由系统配置确定。

## 3. 消息与时间窗口

读一条UL链路时要关联：控制资源安排→RU采集/低PHY→U-plane IQ→DU接收/重组/解压→PUSCH高PHY→上报。关键字段包括frame/subframe/slot/symbol、section、资源起点/数量、流标识、序列、IQ格式和压缩参数；字段布局必须从当前版本和解码器确认，不能凭名称猜字节偏移。

早到/晚到窗口有明确参考点与方向。参考规范§4.4讨论延迟管理；其RU内部/前传参考点不能与L1函数入口混用。项目需提供窗口参数和时钟校准，不能自行设置固定微秒阈值。

教学窗口例（非规范阈值）：把某symbol参考时间定义为0，项目示例要求DU入口IQ在[20,80]μs到达。70μs为窗口内，90μs为晚到；但若70μs之后排队到120μs才开始，仍可能错过L1期限。反之，函数计算加速不能修复没有到达的IQ。必须分开网络/重组/排队/计算时间。

## 4. 代码阅读与实验

先看项目功能切分图、CUS profile、流配置，再找报文解析→section/流匹配→buffer重组→解压→目标模块入口。记录IQ是时域/频域、天线/beam/层维度、定点/浮点格式；SVE优化的输入布局需和该链路一致。

在公司已有获批PCAP上：

```bash
capinfos approved_ul.pcap
tshark -G fields > supported_fields.txt
rg -i 'ecpri|oran' supported_fields.txt
tshark -r approved_ul.pcap -c 20 -V
```

命令中的文件需真实存在；输出只留批准位置。解码器版本不同字段名不同，先查再用`-T fields -e ...`导出，不能编造通用字段命令。建立“包时刻、流、section、symbol、资源、长度、序号、相关L1事件”表，核对是否缺段、重复、乱序或格式不符。抓包点不同，时间结论要写误差与参考点。

无PCAP时用T05合成CSV实验理解时间关系，报告明确仅合成练习；不登记CUS实际互通通过。导师提供真实配置后完成一个UL数据链路和窗口关联图。

## 5. 常见问题与分析

IQ错误但报文解析成功：检查压缩、端序、符号维度、流映射和长度。晚到：分别查同步状态、RU处理、前传、NIC（Network Interface Controller，网络接口控制器）、重组和队列。多个cell混流：检查eAxC映射和buffer key；不能只靠section ID全局关联。硬件时钟偏移：先修时间参考，不能用偏移数据算系统超期。

协议是接口约束，实际DU/RU内部线程实现并非规范唯一指定。seminar要说明“规范要求/项目实现/待确认”三个类别。

## 6. 自测与三级标准

问：C-plane UL资源安排与U-plane UL IQ方向一样吗？答：前者通常DU到RU，后者RU到DU。问：S-plane就是L1 tick吗？答：不是，前者提供同步基础，软件时序机制另由项目实现。问：收到包就等于PUSCH数据就绪吗？答：还需重组、解压和布局准备。

入门：解释三平面和切分。应用：读真实或明确合成报文/时序表，说明参考点和字段。独立：对项目版本的UL配置、IQ和Timing完成关联，报告/seminar经导师确认；完整规范覆盖范围需主管签认。
