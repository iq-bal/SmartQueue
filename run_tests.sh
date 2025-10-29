#!/bin/bash

# Adaptive Priority Router Test Script
# Runs different traffic scenarios and collects results

# Check if we're in opp_env shell
if [ -z "$OMNETPP_ROOT" ]; then
    echo "Error: This script must be run within the opp_env shell."
    echo "Please run: opp_env"
    echo "Then navigate to the project directory and run this script again."
    exit 1
fi

echo "=== Adaptive Priority Router Test Suite ==="
echo "Starting simulation tests..."

# Create results directory if it doesn't exist
mkdir -p results

# Array of test scenarios
scenarios=("General" "Light" "Heavy" "Mixed" "Congestion")

# Run each scenario
for scenario in "${scenarios[@]}"; do
    echo ""
    echo "Running $scenario traffic scenario..."
    echo "======================================="
    
    # Convert scenario name to lowercase for filename
    scenario_lower=$(echo "$scenario" | tr '[:upper:]' '[:lower:]')
    
    # Run simulation with time limit
    if ./SmartQueue -u Cmdenv -f omnetpp.ini -c $scenario --sim-time-limit=30s \
       > "results/${scenario_lower}_log.txt" 2>&1; then
        echo "✓ $scenario scenario completed successfully"
    else
        echo "✗ $scenario scenario failed - check results/${scenario_lower}_log.txt for details"
    fi
done

echo ""
echo "=== Test Summary ==="
echo "All scenarios completed. Results saved in the 'results' directory:"
echo "- Scalar results: *.sca files"
echo "- Vector results: *.vec files" 
echo "- Execution logs: *_log.txt files"
echo ""
echo "To analyze results, you can use OMNeT++ IDE or command-line tools:"
echo "  opp_scavetool export -F CSV-S -T s -o results/scalars.csv results/*.sca"
echo "  opp_scavetool export -F CSV-S -T v -o results/vectors.csv results/*.vec"
echo "  opp_scavetool export -F CSV-R -T v -o results/queue_length_vectors.csv -f "name=~queueLength" results/*.vec > results/scavetool_log.txt 2>&1"
echo ""
echo "Test suite completed!"