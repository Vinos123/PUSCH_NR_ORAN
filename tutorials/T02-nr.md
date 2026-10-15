# T02 从SDR走向基站NR

## 1. 为什么学与前置知识

你熟悉采样、同步、调制、信道编码，这些是分析PUSCH接收的支点。缺口在“谁安排资源、参数从哪里来、何时必须输出、多个小区如何共享算力”。三个月必学本章；移动性、完整RRC过程、全部频段和复杂载波聚合后续深入。

NR（New Radio，新空口）属于蜂窝系统；vRAN（virtualized Radio Access Network，虚拟化无线接入网）强调软件与运行平台解耦，O-RAN强调开放接口和架构，两者不能直接等同。L1是Layer 1物理层，L2是Layer 2数据链路层；MAC（Medium Access Control，介质访问控制）调度资源，PHY处理信号。

## 2. 从时间采样到资源栅格

OFDM（Orthogonal Frequency Division Multiplexing，正交频分复用）用多个子载波承载符号。FFT（Fast Fourier Transform，快速傅里叶变换）后按symbol和subcarrier组织频域数据。CP（Cyclic Prefix，循环前缀）用于抵抗适当范围的多径并维持正交条件；不是越长越好。

资源栅格是频率×时间×端口的索引空间。RE（Resource Element，资源元素）对应一个子载波上的一个OFDM symbol；PRB（Physical Resource Block，物理资源块）含12个子载波。numerology（参数集）用μ描述子载波间隔SCS（Subcarrier Spacing）；正常CP下常用关系为SCS=15×2^μ kHz、slot长度=1/2^μ ms，一个slot通常14个symbol。frame（无线帧）10ms，subframe（子帧）1ms。扩展CP另有适用配置，不能硬套14。

这些定义可核对[38.211第4章](https://www.etsi.org/deliver/etsi_ts/138200_138299/138211/17.09.00_60/ts_138211v170900p.pdf)。真实采样率、FFT点数、活动RB数和缓存长度来自配置，不由μ单独确定；精确symbol边界也需考虑CP差异。

教学例：μ=1，一帧20个slot；12 PRB、10个分配symbol共有12×12×10=1440个RE。若每PRB扣除12个DM-RS RE且无其他开销，数据RE=1296。单层QPSK（Quadrature Phase Shift Keying，正交相移键控）每RE映射2bit，得到2592个调制bit位置；这不是用户净载荷。

## 3. NR特有的输入和状态

DM-RS（Demodulation Reference Signal，解调参考信号）为信道估计提供已知结构。基站根据配置找到参考位置，再估计信道、均衡数据；不一定每个symbol都有参考。PT-RS（Phase Tracking Reference Signal，相位跟踪参考信号）用于相关配置下的相位跟踪，不能和DM-RS混用。

MIMO（Multiple Input Multiple Output，多输入多输出）需要区分接收天线、逻辑端口、传输层和预编码。多个接收天线不表示一定多层；多层检测需要信道矩阵和噪声信息。SDR中的单流滤波经验可以帮助理解每路处理，但不能替代多层配置和端口映射知识。

HARQ（Hybrid Automatic Repeat Request，混合自动重传请求）结合编码与重传。接收端保留soft information（软信息），如LLR（Log-Likelihood Ratio，对数似然比），按进程和重传参数进行合并。不同UE、进程、传输块的buffer不能混用；重传不意味着总发送相同bit序列。

MAC/PHY输入一般包括cell、UE（User Equipment，用户设备）标识、帧/slot、资源、MCS（Modulation and Coding Scheme，调制编码方案）、层数、参考信号和HARQ状态。输出包含数据、CRC（Cyclic Redundancy Check，循环冗余校验）及测量/状态。真实接口可能是标准化或内部形式，不能认定所有项目都使用同一FAPI（Functional Application Platform Interface，功能应用平台接口）。

和卫星SDR的类比：同步都需要统一参考，但NR的调度、slot配置、HARQ和实时回报机制有各自规范；不能把卫星帧格式、导频布局或重传机制搬来解释NR条款。

## 4. 流程与代码阅读

通用基站上行链路：调度参数准备→相应IQ到达→资源提取/参考处理→信道估计与检测→软解调→解扰与速率恢复→HARQ合并/解码→CRC和上报。FFT、前传解压等操作在哪一侧完成取决于功能切分，T05/T09会说明。

读代码先搜索配置结构，而非直接钻FFT或解码大循环。挑一个UE、一个slot，列出资源起点、长度、端口、层、MCS、HARQ process、RV（Redundancy Version，冗余版本）。追踪每个字段由谁填写、在哪使用、是否变单位。把buffer索引写成天线/层×symbol×频率，并检查实际layout是否一致。

## 5. 可操作实验

运行`python3 labs/nr_example.py`，先在纸上推算上述数量。然后复制脚本到自己的独立练习目录，把μ改成0和2、PRB改成1和24；解释slot变短为何不意味着总处理量自动下降。预计slot为1ms和0.25ms，其他RE数量只由例子的资源参数变化。

项目练习：选择一个已有通过用例，从调度文件/日志取实际参数，把每个值填入模板。对照代码中资源提取长度；若天线、层或padding使字节数不同，写清原因。实际项目未提供，当前只能作为待执行步骤。

## 6. 常见问题

RE数量差一倍：检查I/Q是不是两个数、复数还是实数，层与天线是否混淆。slot编号错：先确认相对frame还是绝对计数、numerology是否相同。CRC失败：可能来自资源、参考、数值、软bit顺序或HARQ状态，不能只怪解码器。多小区延迟增加：可能是共享核心、cache、前传和调度，不只是算法复杂度。

## 7. 自测与完成标准

问：μ=2一帧几个slot？答：正常CP时间结构下40。问：1296 RE×2是否就是TBS？答：不是，TBS还涉及码率、量化规则、CRC、编码和复用。问：四接收天线一定四层吗？答：不是。

入门：解释RE/PRB/slot、参考、层与HARQ。应用：完成资源算例并从一个配置填参数表。独立：真实用例参数→代码长度→结果全部对应，能定位一项映射或配置错误并提供测试证据。
