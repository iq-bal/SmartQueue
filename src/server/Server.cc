//
// Server.cc - Implementation of server module for metrics collection
//

#include "Server.h"

Define_Module(Server);

Server::Server()
{
    totalPacketsReceived = 0;
    totalDelay = 0;
    throughputTimer = nullptr;
    throughputInterval = 1.0;  // Calculate throughput every second
    packetsInInterval = 0;
}

Server::~Server()
{
    cancelAndDelete(throughputTimer);
}

void Server::initialize()
{
    // Initialize statistics
    totalPacketsReceived = 0;
    totalDelay = 0;
    packetsInInterval = 0;
    
    // Register signals
    packetReceivedSignal = registerSignal("packetReceived");
    delaySignal = registerSignal("delay");
    voiceDelaySignal = registerSignal("voiceDelay");
    videoDelaySignal = registerSignal("videoDelay");
    dataDelaySignal = registerSignal("dataDelay");
    
    // Initialize vectors
    delayVector.setName("delay");
    voiceDelayVector.setName("voiceDelay");
    videoDelayVector.setName("videoDelay");
    dataDelayVector.setName("dataDelay");
    throughputVector.setName("throughput");
    
    // Schedule throughput calculation
    throughputTimer = new cMessage("throughputTimer");
    scheduleAt(simTime() + throughputInterval, throughputTimer);
    
    EV_INFO << "Server initialized - Ready to receive packets" << endl;
}

void Server::handleMessage(cMessage *msg)
{
    if (msg == throughputTimer) {
        // Calculate and record throughput
        calculateThroughput();
        scheduleAt(simTime() + throughputInterval, throughputTimer);
    }
    else if (msg->arrivedOn("in")) {
        // Incoming packet from router
        Packet *packet = check_and_cast<Packet*>(msg);
        processPacket(packet);
        delete packet;
    }
    else {
        EV_ERROR << "Unknown message received" << endl;
        delete msg;
    }
}

void Server::finish()
{
    EV_INFO << "Server finished - Processing final statistics" << endl;
    
    // Calculate final throughput
    calculateThroughput();
    
    // Print detailed statistics
    printStatistics();
    
    // Record scalar statistics
    recordScalar("totalPacketsReceived", totalPacketsReceived);
    recordScalar("averageDelay", totalPacketsReceived > 0 ? totalDelay / totalPacketsReceived : 0);
    
    // Record statistics per traffic type
    for (const auto &pair : trafficStats) {
        const std::string &type = pair.first;
        const TrafficStats &stats = pair.second;
        
        recordScalar((type + "PacketsReceived").c_str(), stats.packetsReceived);
        recordScalar((type + "AverageDelay").c_str(), 
                     stats.packetsReceived > 0 ? stats.totalDelay / stats.packetsReceived : 0);
        recordScalar((type + "MinDelay").c_str(), stats.minDelay == DBL_MAX ? 0 : stats.minDelay);
        recordScalar((type + "MaxDelay").c_str(), stats.maxDelay);
        
        // Calculate throughput for this traffic type
        if (stats.lastPacketTime > stats.firstPacketTime) {
            double duration = SIMTIME_DBL(stats.lastPacketTime - stats.firstPacketTime);
            double throughput = stats.packetsReceived / duration;
            recordScalar((type + "Throughput").c_str(), throughput);
        }
    }
}

void Server::processPacket(Packet *packet)
{
    totalPacketsReceived++;
    packetsInInterval++;
    
    // Calculate end-to-end delay
    simtime_t arrivalTime = simTime();
    double delay = SIMTIME_DBL(arrivalTime - packet->getTimestamp());
    totalDelay += delay;
    
    // Get packet information
    std::string packetType = packet->getType();
    int priority = packet->getPriorityLevel();
    
    EV_DEBUG << "Received packet: " << packet->getName() 
             << " (type: " << packetType 
             << ", priority: " << priority 
             << ", delay: " << delay << "s)" << endl;
    
    // Update traffic-specific statistics
    updateTrafficStats(packetType, delay, arrivalTime);
    
    // Emit signals for statistics collection
    emit(packetReceivedSignal, totalPacketsReceived);
    emit(delaySignal, delay);
    
    // Emit type-specific delay signals
    if (packetType == "voice") {
        emit(voiceDelaySignal, delay);
        voiceDelayVector.record(delay);
    } else if (packetType == "video") {
        emit(videoDelaySignal, delay);
        videoDelayVector.record(delay);
    } else if (packetType == "data") {
        emit(dataDelaySignal, delay);
        dataDelayVector.record(delay);
    }
    
    // Record overall delay
    delayVector.record(delay);
}

void Server::updateTrafficStats(const std::string &type, double delay, simtime_t arrivalTime)
{
    TrafficStats &stats = trafficStats[type];
    
    stats.packetsReceived++;
    stats.totalDelay += delay;
    
    if (delay < stats.minDelay) {
        stats.minDelay = delay;
    }
    if (delay > stats.maxDelay) {
        stats.maxDelay = delay;
    }
    
    if (stats.packetsReceived == 1) {
        stats.firstPacketTime = arrivalTime;
    }
    stats.lastPacketTime = arrivalTime;
}

void Server::calculateThroughput()
{
    double throughput = packetsInInterval / throughputInterval;
    throughputVector.record(throughput);
    
    EV_DEBUG << "Throughput in last " << throughputInterval 
             << "s: " << throughput << " packets/s" << endl;
    
    packetsInInterval = 0;
}

void Server::printStatistics()
{
    EV_INFO << "=== SERVER STATISTICS ===" << endl;
    EV_INFO << "Total packets received: " << totalPacketsReceived << endl;
    EV_INFO << "Average delay: " << (totalPacketsReceived > 0 ? totalDelay / totalPacketsReceived : 0) << "s" << endl;
    
    for (const auto &pair : trafficStats) {
        const std::string &type = pair.first;
        const TrafficStats &stats = pair.second;
        
        EV_INFO << "--- " << type << " traffic ---" << endl;
        EV_INFO << "  Packets received: " << stats.packetsReceived << endl;
        EV_INFO << "  Average delay: " << (stats.packetsReceived > 0 ? stats.totalDelay / stats.packetsReceived : 0) << "s" << endl;
        EV_INFO << "  Min delay: " << (stats.minDelay == DBL_MAX ? 0 : stats.minDelay) << "s" << endl;
        EV_INFO << "  Max delay: " << stats.maxDelay << "s" << endl;
        
        if (stats.lastPacketTime > stats.firstPacketTime) {
            double duration = SIMTIME_DBL(stats.lastPacketTime - stats.firstPacketTime);
            double throughput = stats.packetsReceived / duration;
            EV_INFO << "  Throughput: " << throughput << " packets/s" << endl;
        }
    }
    EV_INFO << "=========================" << endl;
}