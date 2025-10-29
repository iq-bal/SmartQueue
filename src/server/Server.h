//
// Server.h - Server module for packet reception and metrics collection
// Logs arrival times, delays, throughput, and packet loss statistics
//

#ifndef SERVER_H
#define SERVER_H

#include <omnetpp.h>
#include <map>
#include "../messages/Packet_m.h"

using namespace omnetpp;

struct TrafficStats {
    int packetsReceived;
    double totalDelay;
    double minDelay;
    double maxDelay;
    simtime_t firstPacketTime;
    simtime_t lastPacketTime;
    
    TrafficStats() : packetsReceived(0), totalDelay(0), minDelay(DBL_MAX), 
                     maxDelay(0), firstPacketTime(0), lastPacketTime(0) {}
};

class Server : public cSimpleModule
{
private:
    // Statistics per traffic type
    std::map<std::string, TrafficStats> trafficStats;
    
    // Overall statistics
    int totalPacketsReceived;
    double totalDelay;
    
    // Signals for statistics
    simsignal_t packetReceivedSignal;
    simsignal_t delaySignal;
    simsignal_t voiceDelaySignal;
    simsignal_t videoDelaySignal;
    simsignal_t dataDelaySignal;
    
    // Statistics vectors
    cOutVector delayVector;
    cOutVector voiceDelayVector;
    cOutVector videoDelayVector;
    cOutVector dataDelayVector;
    cOutVector throughputVector;
    
    // Throughput calculation
    cMessage *throughputTimer;
    double throughputInterval;
    int packetsInInterval;
    
protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;
    
    // Helper methods
    void processPacket(Packet *packet);
    void calculateThroughput();
    void updateTrafficStats(const std::string &type, double delay, simtime_t arrivalTime);
    void printStatistics();
    
public:
    Server();
    virtual ~Server();
};

#endif // SERVER_H