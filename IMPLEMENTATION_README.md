# 广播回声协议实现说明

## 实现概述

本项目实现了一个用于移动自组织网络（MANET）的广播回声协议。该协议使用序列号去重机制来控制广播风暴，确保根节点发送的消息能够可靠地传播到网络中的所有移动节点。

## 已实现的功能

### 核心功能
- ✅ 根节点识别和定期广播
- ✅ 消息接收和处理
- ✅ 基于序列号的消息去重机制
- ✅ 消息转发
- ✅ 统计信息收集和输出
- ✅ 错误处理和内存管理

### 关键特性
- **广播风暴控制**: 使用 (源ID, 序列号) 对作为消息唯一标识符，防止重复转发
- **移动性支持**: 协议不依赖静态拓扑，适应节点移动
- **统计收集**: 记录接收、转发、丢弃的消息数量
- **可配置参数**: 支持配置节点数量、移动速度、传输范围、广播间隔

## 文件结构

```
assignment/
├── src/
│   ├── LogicalLayer.h      # 逻辑层头文件（包含 BroadcastMessage 类）
│   ├── LogicalLayer.cc     # 逻辑层实现文件
│   ├── Phy.h              # 物理层头文件（不可修改）
│   └── Phy.cc             # 物理层实现文件（不可修改）
├── simulations/
│   └── omnetpp.ini        # 仿真配置文件（包含8个测试配置）
└── IMPLEMENTATION_README.md  # 本文件
```

## 类设计

### BroadcastMessage 类
自定义消息类，继承自 `cMessage`，包含：
- `sourceNodeId`: 源节点ID
- `sequenceNumber`: 序列号
- `timestamp`: 时间戳（用于延迟计算）
- `hopCount`: 跳数计数器（预留用于TTL机制）

### LogicalLayer 类
协议的核心实现类，主要方法：
- `initialize()`: 初始化节点，识别根节点，启动定时器
- `handleMessage()`: 处理接收到的消息（定时器或广播消息）
- `handleBroadcastTimer()`: 根节点创建并发送广播消息
- `handleReceivedMessage()`: 处理接收到的广播消息
- `isMessageDuplicate()`: 检查消息是否重复
- `recordMessage()`: 记录消息到去重集合
- `forwardMessage()`: 转发消息
- `finish()`: 输出统计信息

## 协议工作流程

1. **初始化阶段**:
   - 节点0被识别为根节点
   - 根节点启动广播定时器
   - 其他节点等待接收消息

2. **广播阶段**（根节点）:
   - 定时器触发
   - 创建 BroadcastMessage，设置源ID和序列号
   - 发送到物理层（自动广播给范围内的邻居）
   - 重新调度定时器

3. **接收和转发阶段**（所有节点）:
   - 接收到广播消息
   - 检查消息是否重复（查找去重集合）
   - 如果是新消息：记录并转发
   - 如果是重复消息：丢弃

4. **统计输出阶段**:
   - 仿真结束时输出统计信息
   - 记录到 .sca 结果文件

## 测试配置

在 `simulations/omnetpp.ini` 中定义了8个测试配置：

| 配置 | 节点数 | 移动速度 | 传输范围 | 广播间隔 | 用途 |
|------|--------|----------|----------|----------|------|
| Test1_SmallStatic | 10 | 0 | 200 | 2s | 基准测试（静止） |
| Test2_MediumMobile | 20 | 10 | 200 | 2s | 中等规模移动 |
| Test3_LargeFastMobile | 50 | 50 | 200 | 2s | 大规模高速移动 |
| Test4_ShortRange | 20 | 10 | 100 | 2s | 短传输范围 |
| Test5_LongRange | 20 | 10 | 300 | 2s | 长传输范围 |
| Test6_HighFrequency | 20 | 10 | 200 | 0.5s | 高频广播 |
| Test7_LowFrequency | 20 | 10 | 200 | 5s | 低频广播 |
| Test8_Stress | 100 | 100 | 200 | 1s | 压力测试 |

## 如何编译和运行

### 在 macOS 本地运行（推荐）

详细的环境配置请参考 `MACOS_SETUP_GUIDE.md`

```bash
# 1. 设置环境变量
source ~/omnetpp-6.0.3/setenv

# 2. 生成 Makefile（首次）
cd src
opp_makemake -f --deep
cd ..

# 3. 编译
make clean && make

# 4. 运行测试
cd simulations
../src/assignment -u Cmdenv -c Test1_SmallStatic
```

### 在 Linux 服务器上运行

如果有 Linux 服务器访问权限：

```bash
cd assignment
source ~/omnetpp-6.0.3/setenv  # 设置环境变量
cd src && opp_makemake -f --deep && cd ..
make clean
make
```

### 运行仿真

**使用图形界面（Qtenv）**:
```bash
cd simulations
../src/assignment -u Qtenv
```

**运行特定配置**:
```bash
../src/assignment -u Qtenv -c Test1_SmallStatic
```

**使用命令行（Cmdenv）批量测试**:
```bash
for config in Test1_SmallStatic Test2_MediumMobile Test3_LargeFastMobile; do
    ../src/assignment -u Cmdenv -c $config
done
```

### 查看结果

结果文件位于 `simulations/results/` 目录：
- `.sca` 文件：标量统计数据
- `.vec` 文件：向量时间序列数据

使用 OMNeT++ IDE 的 Analysis 工具查看和分析结果。

## 性能指标

协议收集以下统计数据：

- **messagesReceived**: 节点接收到的消息总数（包括重复）
- **messagesForwarded**: 节点转发的消息数量
- **duplicatesDropped**: 丢弃的重复消息数量
- **messagesGenerated**: 根节点生成的消息数量
- **duplicateRatio**: 重复消息比例 = duplicatesDropped / messagesReceived

### 评估指标

1. **消息成功率** = (接收到消息的节点数) / (总节点数 - 1)
2. **流量开销** = (所有节点的总转发次数) / (根节点发送的消息数)
3. **平均延迟** = 消息从根节点到达其他节点的平均时间

## 设计决策

### 为什么使用序列号去重？
- **简单高效**: O(log n) 查找复杂度
- **可靠**: 保证每条消息只转发一次
- **无需拓扑信息**: 适应移动环境

### 为什么选择节点0作为根节点？
- **确定性**: 避免根节点选举的复杂性
- **简单**: 符合作业要求（ID最小的节点）

### 为什么立即转发而不是延迟？
- **低延迟**: 消息快速传播
- **简单**: 无需维护转发队列
- **适合广播**: 不需要等待确认

## 与现有技术的对比

| 特性 | 简单泛洪 | 本协议 | 概率泛洪 | AODV |
|------|----------|--------|----------|------|
| 可靠性 | 高 | 高 | 中 | 高 |
| 流量开销 | 很高 | 中 | 中 | 低 |
| 延迟 | 低 | 低 | 低 | 中 |
| 移动鲁棒性 | 高 | 高 | 高 | 低 |
| 实现复杂度 | 很低 | 低 | 低 | 高 |

## 可能的优化方向

如果时间允许，可以考虑以下优化：

1. **TTL机制**: 限制消息传播跳数
2. **概率转发**: 以一定概率转发消息，进一步减少流量
3. **延迟转发**: 添加随机延迟，避免冲突
4. **消息过期**: 定期清理旧消息标识符，减少内存占用

## 注意事项

1. **不要修改物理层**: Phy.h 和 Phy.cc 文件不应被修改
2. **内存管理**: 确保所有接收到的消息都被删除或转发
3. **定时器管理**: 定时器消息应该重复使用，不要删除
4. **参数配置**: broadcastInterval 参数需要在 .ned 文件或 omnetpp.ini 中定义

## 故障排除

### 编译错误
- 确保设置了 OMNeT++ 环境变量：`source ~/omnetpp-6.0.3/setenv`
- 检查是否有语法错误
- 尝试 `make clean && make`

### 运行时错误
- 检查 omnetpp.ini 中的参数是否正确
- 确保 broadcastInterval 参数已定义
- 查看日志输出中的错误信息

### 消息不传播
- 检查传输范围（limit）是否足够大
- 检查节点数量和网络密度
- 使用 Qtenv 可视化查看节点位置和消息传递

## 参考资料

- OMNeT++ 官方文档: https://omnetpp.org/doc/omnetpp/manual/
- OMNeT++ API 参考: https://omnetpp.org/doc/omnetpp/api/
- 广播风暴问题: 搜索 "Broadcast Storm Problem in MANET"
- 泛洪协议: 搜索 "Flooding in Mobile Ad Hoc Networks"

## 作者

本实现基于 CO6203 课程作业要求完成。

## 许可

本代码仅用于教育目的。
