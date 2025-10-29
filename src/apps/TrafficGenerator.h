//
// TrafficGenerator.h - Traffic generation module for different packet types
// Generates voice, video, or data packets with appropriate priorities
//

#ifndef TRAFFICGENERATOR_H
#define TRAFFICGENERATOR_H

#include <omnetpp.h>
#include "../messages/Packet_m.h"

using namespace omnetpp;

class TrafficGenerator : public cSimpleModule
{
private:
    // Parameters
    std::string packetType;
    int priorityLevel;
    double sendInterval;
    
    // Statistics
    int packetsSent;
    cMessage *sendTimer;
    
    // Signals for statistics
    simsignal_t packetSentSignal;
    
protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;
    
    // Helper methods
    void scheduleNextPacket();
    Packet* createPacket();
    
public:
    TrafficGenerator();
    virtual ~TrafficGenerator();
};

#endif // TRAFFICGENERATOR_H