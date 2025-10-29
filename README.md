# Adaptive Packet Priority Router

An OMNeT++ simulation project that implements a router with dynamic packet forwarding priority based on real-time queue congestion monitoring.

## Project Overview

This project simulates an adaptive router that dynamically adjusts packet forwarding priority based on queue congestion levels. The router operates in two modes:
- **Normal Mode**: Round-robin scheduling across all priority queues
- **Strict Mode**: High-priority packets are processed first when congestion is detected

### Key Features

- **Three-Priority Queue System**: High, Medium, and Low priority queues
- **Congestion Detection**: Real-time monitoring of queue lengths
- **Adaptive Scheduling**: Dynamic switching between normal and strict priority modes
- **Traffic Generation**: Multiple traffic types (Voice, Video, Data) with different characteristics
- **Comprehensive Metrics**: Detailed statistics collection for performance analysis

## Project Structure

```
SmartQueue/
├── src/                    # Source code
│   ├── apps/              # Traffic generator applications
│   ├── messages/          # Message definitions
│   ├── router/            # Router implementation
│   └── server/            # Server implementation
├── ned/                   # Network description files
├── results/               # Simulation results
├── docs/                  # Documentation
├── omnetpp.ini           # Simulation configuration
├── run_tests.sh          # Test automation script
└── Makefile              # Build configuration
```

## Prerequisites

- OMNeT++ 6.0 or later
- Access to `opp_env` shell environment
- Basic knowledge of OMNeT++ simulation framework

## Building and Running

### 1. Setup Environment

Make sure you are in the OMNeT++ environment shell:
```bash
# Start opp_env shell if not already in it
opp_env
```

### 2. Build the Project

Navigate to the project directory and build:
```bash
cd /path/to/SmartQueue
opp_makemake -f --deep -I./src -I./ned
make
```

### 3. Run Simulations

#### Run All Test Scenarios
```bash
./run_tests.sh
```

#### Run Individual Scenarios
```bash
# Run specific configuration
opp_run -u Cmdenv -f omnetpp.ini -c Light
opp_run -u Cmdenv -f omnetpp.ini -c Heavy
opp_run -u Cmdenv -f omnetpp.ini -c Mixed
opp_run -u Cmdenv -f omnetpp.ini -c Congestion
```

#### Run with GUI (if available)
```bash
opp_run -u Qtenv -f omnetpp.ini
```

## Traffic Scenarios

The project includes several predefined traffic scenarios:

1. **General**: Default balanced traffic
2. **Light**: Low traffic load across all types
3. **Heavy**: High traffic load testing congestion handling
4. **Mixed**: Varied traffic patterns
5. **Congestion**: Extreme load to test adaptive behavior

## Configuration Parameters

Key parameters in `omnetpp.ini`:

### Router Configuration
- `queueCapacity`: Maximum packets per priority queue (default: 50)
- `congestionThreshold`: Queue length triggering strict mode (default: 30)
- `recoveryThreshold`: Queue length for returning to normal mode (default: 15)
- `checkInterval`: Congestion monitoring frequency (default: 0.1s)

### Traffic Generation
- `sendInterval`: Packet generation rate per traffic type
- `priorityLevel`: Packet priority (0=High, 1=Medium, 2=Low)
- `packetType`: Traffic type identifier

## Results Analysis

After running simulations, results are stored in the `results/` directory:

- `*.sca`: Scalar statistics (averages, counts, etc.)
- `*.vec`: Vector data (time series)
- `*_log.txt`: Execution logs

### Export Results to CSV
```bash
# Export scalar results
scavetool export -f CSV -o results.csv results/*.sca

# Export vector results  
scavetool export -f CSV -o vectors.csv results/*.vec
```

## Key Metrics

The simulation collects various performance metrics:

### Router Metrics
- Queue lengths over time
- Packets dropped by priority
- Mode switching frequency
- Processing delays

### Traffic Metrics
- End-to-end delay by traffic type
- Throughput measurements
- Packet loss rates
- Jitter analysis

### System Metrics
- Overall network utilization
- Congestion frequency
- Adaptive behavior effectiveness

## Troubleshooting

### Common Issues

1. **Build Errors**: Ensure you're in `opp_env` shell and OMNeT++ is properly installed
2. **Missing Tools**: Verify `opp_makemake`, `opp_run`, and `scavetool` are available
3. **Permission Errors**: Make sure `run_tests.sh` is executable (`chmod +x run_tests.sh`)

### Debug Mode

Run with detailed logging:
```bash
opp_run -u Cmdenv -f omnetpp.ini --debug-on-errors=true
```

## Contributing

When modifying the project:

1. Follow OMNeT++ coding conventions
2. Update documentation for new features
3. Test all scenarios after changes
4. Maintain backward compatibility with configuration files

## License

This project is for educational and research purposes. Please refer to OMNeT++ licensing terms for commercial use.

## References

- [OMNeT++ Documentation](https://omnetpp.org/documentation/)
- [OMNeT++ User Guide](https://doc.omnetpp.org/omnetpp/UserGuide.pdf)
- [Network Simulation Best Practices](https://omnetpp.org/documentation/guides/)