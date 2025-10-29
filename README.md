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

- OMNeT++ 6.2 or later
- Access to `opp_env` shell environment
- Basic knowledge of OMNeT++ simulation framework

## Building and Running

### 1. Setup Environment

Make sure you are in the OMNeT++ environment shell:
```bash
# Start opp_env shell if not already in it
opp_env shell
```

### 2. Build the Project

Navigate to the project directory and build:
```bash
cd /path/to/SmartQueue
make
```

If you have significantly changed the project structure (e.g., added new source directories), regenerate the Makefile:
```bash
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
# using the built binary
./SmartQueue -u Cmdenv -f omnetpp.ini -c Light
./SmartQueue -u Cmdenv -f omnetpp.ini -c Heavy
./SmartQueue -u Cmdenv -f omnetpp.ini -c Mixed
./SmartQueue -u Cmdenv -f omnetpp.ini -c Congestion
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
- `queueCapacity`: Maximum total packets across all priorities (default: 200)
- `congestionThreshold`: Utilization fraction to enter strict mode (default: 0.7)
- `recoveryThreshold`: Utilization fraction to return to normal mode (default: 0.5)
- `checkInterval`: Congestion monitoring frequency (default: 0.1s)

### Traffic Generation
- `sendInterval`: Packet generation rate per traffic type
- `priorityLevel`: Packet priority (1=Low, 2=Medium, 3=High)
- `packetType`: Traffic type identifier (`"voice"`, `"video"`, `"data"`)

## Results Analysis

After running simulations, results are stored in the `results/` directory:

- `*.sca`: Scalar statistics (averages, counts, etc.)
- `*.vec`: Vector data (time series)
- `*_log.txt`: Execution logs

### Export Results to CSV
```bash
# Export scalar results (CSV-S format, scalars)
opp_scavetool export -F CSV-S -T s -o results/scalars.csv results/*.sca

# Export vector results (CSV-S format, vectors)
opp_scavetool export -F CSV-S -T v -o results/vectors.csv results/*.vec

# Optional: filter examples
# Only router queue length vector
opp_scavetool export -F CSV-S -T v -o results/queueLength.csv -f 'name(queueLength)' results/*.vec
# Only per-type average delays from server (scalars)
opp_scavetool export -F CSV-S -T s -o results/delays_avg.csv -f 'name(~*AverageDelay) && module=~**.server' results/*.sca

# If 'opp_scavetool' is unavailable, use the full path or add to PATH:
# $OMNETPP_ROOT/bin/scavetool export -F CSV-S -T s -o results/scalars.csv results/*.sca
# export PATH="$OMNETPP_ROOT/bin:$PATH"
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