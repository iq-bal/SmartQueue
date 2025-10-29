//
// AdaptiveRouter.h - Adaptive priority router with congestion control
// Dynamically adjusts packet forwarding priority based on queue congestion
//

#ifndef ADAPTIVEROUTER_H
#define ADAPTIVEROUTER_H

#include <omnetpp.h>
#include "PriorityQueue.h"
#include "../messages/Packet_m.h"

using namespace omnetpp;

enum RouterMode {
    NORMAL_MODE,     // Fair scheduling
    CONGESTED_MODE   // Strict priority scheduling
};

class AdaptiveRouter : public cSimpleModule
{
private:
    // Parameters
    int queueCapacity;
    double congestionThreshold;
    double recoveryThreshold;
    double checkInterval;
    
    // Router state
    PriorityQueue *priorityQueue;
    RouterMode currentMode;
    cMessage *checkTimer;
    cMessage *processTimer;
    
    // Statistics
    int packetsReceived;
    int packetsForwarded;
    int packetsDropped;
    int lowPriorityDropped;
    
    // Signals for statistics
    simsignal_t queueLengthSignal;
    simsignal_t packetDroppedSignal;
    simsignal_t modeChangedSignal;
    simsignal_t delaySignal;
    
    // Statistics vectors
    cOutVector queueLengthVector;
    cOutVector modeVector;
    
protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;
    
    // Router operations
    void checkCongestion();
    void processNextPacket();
    void handleIncomingPacket(Packet *packet);
    void switchMode(RouterMode newMode);
    
    // Queue management
    bool enqueuePacket(Packet *packet);
    void dropLowPriorityPackets();
    
public:
    AdaptiveRouter();
    virtual ~AdaptiveRouter();
};

#endif // ADAPTIVEROUTER_H