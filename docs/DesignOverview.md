# Design Overview: Adaptive Packet Priority Router

## Architecture Overview

The Adaptive Packet Priority Router is designed as a modular OMNeT++ simulation that demonstrates dynamic Quality of Service (QoS) management in network routers. The system adapts its packet forwarding behavior based on real-time congestion monitoring.

## System Components

### 1. Network Topology

```
[Client1] ──┐
[Client2] ──┼── [AdaptiveRouter] ── [Server]
[Client3] ──┘
```

The network consists of:
- **3 Traffic Generators**: Simulate different traffic types (Voice, Video, Data)
- **1 Adaptive Router**: Central component with intelligent queue management
- **1 Server**: Destination node for metrics collection

### 2. Core Modules

#### 2.1 TrafficGenerator (`src/apps/`)

**Purpose**: Generates packets with different characteristics based on traffic type.

**Key Features**:
- Configurable packet generation rates
- Priority assignment based on traffic type
- Exponential inter-arrival time distribution
- Statistics collection for generated traffic

**Parameters**:
- `sendInterval`: Base packet generation interval
- `priorityLevel`: Packet priority (0=High, 1=Medium, 2=Low)
- `packetType`: Traffic type identifier

**Design Decisions**:
- Uses exponential distribution to model realistic traffic patterns
- Separate generators for each traffic type to enable independent control
- Built-in statistics for monitoring traffic generation rates

#### 2.2 AdaptiveRouter (`src/router/`)

**Purpose**: Core routing component with adaptive priority scheduling.

**Key Components**:

##### PriorityQueue Class
- **Three separate queues**: High, Medium, Low priority
- **Dual scheduling modes**:
  - Normal: Round-robin across all queues
  - Strict: Priority-based with high packets first
- **Congestion management**: Automatic low-priority packet dropping

##### Router Logic
- **Congestion Detection**: Monitors total queue length
- **Mode Switching**: Dynamic transition between normal/strict modes
- **Statistics Collection**: Comprehensive metrics for analysis

**Parameters**:
- `queueCapacity`: Maximum packets per priority queue (50)
- `congestionThreshold`: Trigger for strict mode (30 packets)
- `recoveryThreshold`: Return to normal mode (15 packets)
- `checkInterval`: Monitoring frequency (0.1 seconds)

**Design Decisions**:
- Separate queues prevent head-of-line blocking
- Hysteresis in threshold switching prevents oscillation
- Configurable parameters allow scenario testing

#### 2.3 Server (`src/server/`)

**Purpose**: Packet destination with comprehensive metrics collection.

**Key Features**:
- End-to-end delay calculation
- Per-traffic-type statistics
- Throughput monitoring
- Detailed logging capabilities

**Metrics Collected**:
- Packet reception counts
- Delay measurements by traffic type
- Throughput calculations
- Statistical summaries

### 3. Message Definitions

#### Packet Message (`src/messages/Packet.msg`)

```cpp
packet Packet {
    int priorityLevel;    // 0=High, 1=Medium, 2=Low
    int type;            // Traffic type identifier
    simtime_t timestamp; // Creation time for delay calculation
}
```

**Design Rationale**:
- Minimal overhead while providing necessary information
- Priority level enables queue classification
- Timestamp enables end-to-end delay measurement
- Type field allows traffic-specific analysis

## Adaptive Algorithm Design

### Congestion Detection

The router continuously monitors queue occupancy:

```cpp
int totalQueueLength = highQueue.size() + mediumQueue.size() + lowQueue.size();

if (currentMode == NORMAL && totalQueueLength >= congestionThreshold) {
    switchToStrictMode();
} else if (currentMode == STRICT && totalQueueLength <= recoveryThreshold) {
    switchToNormalMode();
}
```

### Scheduling Algorithms

#### Normal Mode (Round-Robin)
- Serves one packet from each non-empty queue in rotation
- Ensures fairness across all priority levels
- Prevents starvation of lower-priority traffic

#### Strict Mode (Priority-Based)
- Serves all high-priority packets first
- Processes medium-priority only when high queue is empty
- Handles low-priority only when both high and medium are empty
- May cause starvation but ensures critical traffic delivery

### Queue Management

#### Enqueuing Logic
```cpp
if (queue.size() < queueCapacity) {
    queue.push(packet);
} else {
    // Queue full - drop packet and record statistics
    dropPacket(packet);
}
```

#### Congestion Response
When queues approach capacity:
1. Monitor total occupancy
2. Switch to strict mode if threshold exceeded
3. Drop low-priority packets if individual queues full
4. Return to normal mode when congestion subsides

## Performance Considerations

### Memory Management
- Fixed-size queues prevent unbounded memory growth
- Efficient packet dropping when capacity exceeded
- Minimal object creation during simulation

### Computational Efficiency
- O(1) queue operations for enqueue/dequeue
- Periodic congestion checking rather than per-packet
- Optimized statistics collection

### Scalability
- Modular design allows easy extension
- Configurable parameters enable different scenarios
- Statistics framework supports detailed analysis

## Configuration Design

### Scenario-Based Configuration
The `omnetpp.ini` file defines multiple scenarios:

1. **General**: Baseline configuration
2. **Light**: Low traffic for baseline measurements
3. **Heavy**: High traffic to test congestion handling
4. **Mixed**: Variable traffic patterns
5. **Congestion**: Extreme load testing

### Parameter Hierarchy
```ini
# Global defaults
*.router.queueCapacity = 50

# Scenario-specific overrides
[Config Heavy]
*.client*.sendInterval = exponential(0.05s)  # Higher rate

[Config Light]  
*.client*.sendInterval = exponential(0.5s)   # Lower rate
```

## Statistics and Monitoring

### Router Statistics
- `queueLength`: Real-time queue occupancy
- `droppedPackets`: Packets dropped by priority
- `modeChanges`: Frequency of mode switching
- `processingDelay`: Time spent processing packets

### Traffic Statistics
- `packetsSent`: Generation rate by traffic type
- `packetsReceived`: Reception rate at server
- `endToEndDelay`: Latency measurements
- `throughput`: Data rate calculations

### System Statistics
- `networkUtilization`: Overall system load
- `congestionEvents`: Frequency and duration
- `adaptiveEffectiveness`: Performance improvement metrics

## Design Trade-offs

### Fairness vs. Performance
- Normal mode ensures fairness but may impact critical traffic
- Strict mode prioritizes important traffic but may starve others
- Adaptive switching balances both concerns

### Responsiveness vs. Stability
- Frequent congestion checking enables quick response
- Hysteresis in thresholds prevents mode oscillation
- Configurable intervals allow tuning

### Complexity vs. Maintainability
- Modular design increases complexity but improves maintainability
- Separate queue classes enable independent testing
- Clear interfaces facilitate future extensions

## Future Enhancements

### Potential Improvements
1. **Dynamic Threshold Adjustment**: Adaptive thresholds based on traffic patterns
2. **Weighted Fair Queuing**: More sophisticated scheduling algorithms
3. **Multi-path Routing**: Load balancing across multiple paths
4. **Machine Learning**: AI-based congestion prediction
5. **Real-time Adaptation**: Sub-second response to traffic changes

### Extension Points
- Additional traffic types and characteristics
- More complex network topologies
- Integration with real network protocols
- Hardware-in-the-loop simulation capabilities

This design provides a solid foundation for studying adaptive routing behaviors while maintaining simplicity and extensibility for future research directions.