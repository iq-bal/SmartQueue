# Results Analysis Guide

This document provides comprehensive guidance on analyzing simulation results from the Adaptive Packet Priority Router project.

## Overview of Collected Metrics

The simulation collects extensive performance data across multiple dimensions:

### Router-Level Metrics
- Queue occupancy over time
- Mode switching behavior
- Packet dropping statistics
- Processing delays

### Traffic-Level Metrics
- End-to-end delays by traffic type
- Throughput measurements
- Packet loss rates
- Generation vs. reception statistics

### System-Level Metrics
- Overall network utilization
- Congestion frequency and duration
- Adaptive algorithm effectiveness

## Result File Types

### Scalar Results (*.sca)
Contains aggregate statistics and final values:
- Mean, standard deviation, min/max values
- Total counts and rates
- Final simulation state

### Vector Results (*.vec)
Contains time-series data:
- Queue lengths over time
- Delay measurements per packet
- Throughput variations
- Mode switching events

### Log Files (*_log.txt)
Contains detailed execution information:
- Simulation progress
- Error messages
- Debug information
- Configuration validation

## Key Performance Indicators (KPIs)

### 1. Adaptive Behavior Effectiveness

#### Mode Switching Frequency
```
Metric: routerMode (vector)
How to analyze:
- Export `routerMode` and count transitions between 0 (NORMAL) and 1 (CONGESTED)
- Few transitions indicate stable traffic; frequent transitions indicate active adaptation
Tip: Use a small script to count changes in the `routerMode` vector
```

#### Congestion Response Time
```
Metric: Time between congestion detection and mode switch
Analysis:
- Should be minimal (< checkInterval)
- Delays indicate system responsiveness issues
```

### 2. Quality of Service Metrics

#### End-to-End Delay by Priority
```
Metrics (scalars):
- server.voiceAverageDelay (High priority)
- server.videoAverageDelay (Medium priority)
- server.dataAverageDelay (Low priority)

Expected Behavior:
- Voice < Video < Data delays
- Strict mode should reduce high-priority delays
- Normal mode should show more balanced delays
```

#### Packet Loss by Priority
```
Metrics (scalars):
- router.packetsDropped (total)
- router.lowPriorityDropped (low priority)

Analysis:
- Low-priority packets should be dropped first
- High-priority drops indicate severe congestion
- Zero drops in light traffic scenarios
```

### 3. Throughput Analysis

#### Overall System Throughput
```
Metric: Derived from per-type throughput
Use:
- server.voiceThroughput
- server.videoThroughput
- server.dataThroughput
Analysis:
- High-priority traffic should maintain throughput under congestion
- Overall throughput may decrease when congestion is severe
```

#### Per-Traffic-Type Throughput
```
Metrics (scalars):
- server.voiceThroughput
- server.videoThroughput
- server.dataThroughput

Expected Patterns:
- High-priority traffic maintains throughput under congestion
- Low-priority traffic shows throughput reduction first
```

## Scenario-Specific Analysis

### Light Traffic Scenario
**Expected Results:**
- Minimal queue occupancy (< 10% capacity)
- Rare or no mode switching
- Low, consistent delays across all traffic types
- Zero packet drops
- High overall throughput

**Key Metrics to Check:**
```
router.queueLength:mean ~ 0–3
routerMode:vector transitions ≈ 0
server.*AverageDelay < 0.05s
router.packetsDropped = 0
```

### Heavy Traffic Scenario
**Expected Results:**
- High queue occupancy (approaching capacity)
- Frequent mode switching
- Differentiated delays (voice < video < data)
- Some packet drops (primarily low priority)
- Reduced but stable throughput

**Key Metrics to Check:**
```
router.queueLength:mean typically < 10
routerMode:vector transitions = small (or 0)
server.voiceAverageDelay < server.dataAverageDelay
router.packetsDropped = 0 (at current heavy settings)
```

To stress-test congestion:
- Decrease `**.router.queueCapacity` (e.g., to 50)
- Decrease `sendInterval` values further (e.g., `client[0]=0.005s`, `client[1]=0.004s`, `client[2]=0.003s`)

### Congestion Scenario
**Expected Results:**
- Queue saturation periods
- Extended strict mode operation
- Significant delay differentiation
- Substantial low-priority packet drops
- Throughput prioritization for high-priority traffic

**Analysis Focus:**
- System stability under extreme load
- Effectiveness of priority enforcement
- Recovery behavior when load decreases

## Comparative Analysis

### Before/After Adaptation
Compare metrics between normal and strict modes:

```
Normal Mode Metrics:
- More balanced delays across priorities
- Higher overall throughput
- Lower packet drops

Strict Mode Metrics:
- Reduced high-priority delays
- Increased low-priority delays
- More packet drops (low priority)
```

### Cross-Scenario Comparison
Analyze how the same metrics vary across scenarios:

```
Delay Progression: Light → Heavy → Congestion
Throughput Stability: Light → Heavy → Congestion  
Adaptation Frequency: Light → Heavy → Congestion
```

## Statistical Analysis Techniques

### 1. Time Series Analysis

#### Queue Length Trends
```bash
# Extract queue length vector data
opp_scavetool export -F CSV-S -T v -o results/queue_data.csv -f 'name(queueLength)' results/*.vec

# Analysis points:
- Identify congestion periods
- Measure congestion duration
- Analyze recovery patterns
```

#### Delay Distribution Analysis
```bash
# Extract per-packet delay vectors
opp_scavetool export -F CSV-S -T v -o results/delay_data.csv -f 'name(delay) || name(~*Delay*)' results/*.vec

# Statistical measures:
- Mean, median, 95th percentile delays
- Delay variance and jitter
- Traffic type comparisons
```

### 2. Correlation Analysis

#### Mode Switching vs. Queue Length
Analyze correlation between queue occupancy and mode changes:
- Strong positive correlation indicates responsive adaptation
- Weak correlation suggests tuning needed

#### Throughput vs. Congestion
Examine throughput stability during congestion:
- High-priority traffic should maintain throughput
- Overall throughput may decrease but should recover

### 3. Performance Benchmarking

#### Baseline Comparisons
Compare adaptive router performance against:
- Fixed priority scheduling
- Pure FIFO scheduling
- Weighted fair queuing

#### Efficiency Metrics
```
Adaptation Efficiency = (High Priority Performance Improvement) / (Low Priority Performance Degradation)

Congestion Handling = (Packets Delivered During Congestion) / (Total Packets Generated During Congestion)
```

## Visualization Recommendations

### 1. Time Series Plots
- Queue length over time with mode indicators
- Delay measurements by traffic type
- Throughput variations during simulation

### 2. Distribution Plots
- Delay histograms by priority level
- Queue occupancy distributions
- Inter-arrival time distributions

### 3. Comparative Charts
- Box plots of delays across scenarios
- Bar charts of packet drops by priority
- Scatter plots of throughput vs. load

## Common Analysis Patterns

### Successful Adaptation Indicators
1. **Rapid Mode Switching**: Quick response to congestion
2. **Delay Differentiation**: Clear priority-based delay separation
3. **Selective Dropping**: Low-priority packets dropped first
4. **Stable Recovery**: Quick return to normal operation

### Problem Indicators
1. **Oscillating Modes**: Frequent unnecessary switching
2. **High-Priority Drops**: Critical traffic being lost
3. **Poor Recovery**: Slow return to normal performance
4. **Unfair Starvation**: Excessive low-priority blocking

## Automated Analysis Scripts

### Result Processing Pipeline
```bash
#!/bin/bash
# Process all simulation results

# Export scalar data (CSV-S format, scalars)
opp_scavetool export -F CSV-S -T s -o results/scalars.csv results/*.sca

# Export vector data (CSV-S format, vectors)
opp_scavetool export -F CSV-S -T v -o results/vectors.csv results/*.vec

# Generate summary statistics
python analyze_results.py results/scalars.csv results/vectors.csv
```

### Key Metrics Extraction
```python
# Example Python analysis script structure
import pandas as pd
import matplotlib.pyplot as plt

def analyze_delays(data):
    # Extract delay metrics by traffic type
    # Calculate statistics and generate plots
    pass

def analyze_adaptation(data):
    # Examine mode switching behavior
    # Correlate with queue occupancy
    pass

def generate_report(results):
    # Create comprehensive analysis report
    # Include visualizations and recommendations
    pass
```

## Interpretation Guidelines

### Performance Thresholds
- **Excellent**: Voice delay < 50ms, minimal drops
- **Good**: Voice delay < 100ms, <1% high-priority drops  
- **Acceptable**: Voice delay < 200ms, <5% high-priority drops
- **Poor**: Voice delay > 200ms, >5% high-priority drops

### Adaptation Quality
- **Responsive**: Mode switch within 2x checkInterval
- **Stable**: <10% unnecessary mode switches
- **Effective**: >50% improvement in high-priority metrics during congestion

## Troubleshooting Analysis Issues

### Common Problems
1. **Missing Data**: Check simulation completion and file permissions
2. **Inconsistent Results**: Verify random seed settings and simulation duration
3. **Unexpected Patterns**: Review configuration parameters and network topology

### Validation Checks
1. **Conservation Laws**: Packets sent = packets received + packets dropped
2. **Timing Consistency**: Delays should be positive and reasonable
3. **Priority Ordering**: High-priority metrics should outperform low-priority

This analysis framework provides a systematic approach to understanding the adaptive router's behavior and validating its effectiveness across different traffic scenarios.