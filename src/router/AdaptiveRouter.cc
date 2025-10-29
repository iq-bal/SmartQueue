//
// AdaptiveRouter.cc - Implementation of adaptive priority router
//

#include "AdaptiveRouter.h"

Define_Module(AdaptiveRouter);

AdaptiveRouter::AdaptiveRouter()
{
    priorityQueue = nullptr;
    checkTimer = nullptr;
    processTimer = nullptr;
    currentMode = NORMAL_MODE;
    packetsReceived = 0;
    packetsForwarded = 0;
    packetsDropped = 0;
    lowPriorityDropped = 0;
}

AdaptiveRouter::~AdaptiveRouter()
{
    delete priorityQueue;
    cancelAndDelete(checkTimer);
    cancelAndDelete(processTimer);
}

void AdaptiveRouter::initialize()
{
    // Read parameters
    queueCapacity = par("queueCapacity");
    congestionThreshold = par("congestionThreshold");
    recoveryThreshold = par("recoveryThreshold");
    checkInterval = par("checkInterval");
    
    // Initialize priority queue
    priorityQueue = new PriorityQueue(queueCapacity);
    
    // Initialize timers
    checkTimer = new cMessage("checkTimer");
    processTimer = new cMessage("processTimer");
    
    // Initialize statistics
    queueLengthSignal = registerSignal("queueLength");
    packetDroppedSignal = registerSignal("packetDropped");
    modeChangedSignal = registerSignal("modeChanged");
    delaySignal = registerSignal("delay");
    
    queueLengthVector.setName("queueLength");
    modeVector.setName("routerMode");
    
    // Schedule first congestion check
    scheduleAt(simTime() + checkInterval, checkTimer);
    
    EV_INFO << "AdaptiveRouter initialized - Capacity: " << queueCapacity 
            << ", Congestion threshold: " << congestionThreshold 
            << ", Recovery threshold: " << recoveryThreshold << endl;
}

void AdaptiveRouter::handleMessage(cMessage *msg)
{
    if (msg == checkTimer) {
        // Periodic congestion check
        checkCongestion();
        scheduleAt(simTime() + checkInterval, checkTimer);
    }
    else if (msg == processTimer) {
        // Process next packet in queue
        processNextPacket();
    }
    else if (msg->arrivedOn("in")) {
        // Incoming packet from clients
        Packet *packet = check_and_cast<Packet*>(msg);
        handleIncomingPacket(packet);
    }
    else {
        EV_ERROR << "Unknown message received" << endl;
        delete msg;
    }
}

void AdaptiveRouter::finish()
{
    EV_INFO << "AdaptiveRouter finished:" << endl;
    EV_INFO << "  Packets received: " << packetsReceived << endl;
    EV_INFO << "  Packets forwarded: " << packetsForwarded << endl;
    EV_INFO << "  Packets dropped: " << packetsDropped << endl;
    EV_INFO << "  Low priority dropped: " << lowPriorityDropped << endl;
    
    recordScalar("packetsReceived", packetsReceived);
    recordScalar("packetsForwarded", packetsForwarded);
    recordScalar("packetsDropped", packetsDropped);
    recordScalar("lowPriorityDropped", lowPriorityDropped);
    recordScalar("finalQueueLength", priorityQueue->getLength());
}

void AdaptiveRouter::handleIncomingPacket(Packet *packet)
{
    packetsReceived++;
    
    EV_DEBUG << "Received packet: " << packet->getName() 
             << " (type: " << packet->getType() 
             << ", priority: " << packet->getPriorityLevel() << ")" << endl;
    
    // Try to enqueue the packet
    if (!enqueuePacket(packet)) {
        // Queue is full, drop the packet
        EV_INFO << "Dropping packet due to full queue: " << packet->getName() << endl;
        packetsDropped++;
        emit(packetDroppedSignal, packetsDropped);
        delete packet;
        return;
    }
    
    // Update statistics
    emit(queueLengthSignal, priorityQueue->getLength());
    queueLengthVector.record(priorityQueue->getLength());
    
    // If no packet is currently being processed, start processing
    if (!processTimer->isScheduled()) {
        scheduleAt(simTime() + 0.001, processTimer);  // Small processing delay
    }
}

bool AdaptiveRouter::enqueuePacket(Packet *packet)
{
    // Check if queue is full
    if (priorityQueue->isFull()) {
        return false;
    }
    
    // In congested mode, drop low priority packets if queue is getting full
    if (currentMode == CONGESTED_MODE && 
        priorityQueue->getLength() > (congestionThreshold * queueCapacity)) {
        
        if (packet->getPriorityLevel() == 1) {  // Low priority packet
            EV_INFO << "Dropping low priority packet in congested mode" << endl;
            lowPriorityDropped++;
            packetsDropped++;
            emit(packetDroppedSignal, packetsDropped);
            return false;
        }
    }
    
    return priorityQueue->enqueue(packet);
}

void AdaptiveRouter::processNextPacket()
{
    Packet *packet = nullptr;
    
    // Dequeue based on current mode
    if (currentMode == NORMAL_MODE) {
        packet = priorityQueue->dequeueNormal();
    } else {
        packet = priorityQueue->dequeueStrict();
    }
    
    if (packet != nullptr) {
        // Calculate end-to-end delay
        simtime_t delay = simTime() - packet->getTimestamp();
        emit(delaySignal, delay);
        
        EV_DEBUG << "Forwarding packet: " << packet->getName() 
                 << " (delay: " << delay << "s)" << endl;
        
        // Forward packet to server
        send(packet, "out");
        packetsForwarded++;
        
        // Update statistics
        emit(queueLengthSignal, priorityQueue->getLength());
        queueLengthVector.record(priorityQueue->getLength());
        
        // Schedule next packet processing if queue is not empty
        if (!priorityQueue->isEmpty()) {
            scheduleAt(simTime() + 0.001, processTimer);  // Small processing delay
        }
    }
}

void AdaptiveRouter::checkCongestion()
{
    int currentQueueLength = priorityQueue->getLength();
    double utilization = (double)currentQueueLength / queueCapacity;
    
    RouterMode newMode = currentMode;
    
    if (currentMode == NORMAL_MODE && utilization > congestionThreshold) {
        newMode = CONGESTED_MODE;
        EV_INFO << "Switching to CONGESTED mode (utilization: " << utilization << ")" << endl;
    }
    else if (currentMode == CONGESTED_MODE && utilization < recoveryThreshold) {
        newMode = NORMAL_MODE;
        EV_INFO << "Switching to NORMAL mode (utilization: " << utilization << ")" << endl;
    }
    
    if (newMode != currentMode) {
        switchMode(newMode);
    }
    
    // Print queue status periodically
    priorityQueue->printQueueStatus();
    
    EV_DEBUG << "Congestion check - Mode: " << (currentMode == NORMAL_MODE ? "NORMAL" : "CONGESTED")
             << ", Utilization: " << utilization << endl;
}

void AdaptiveRouter::switchMode(RouterMode newMode)
{
    currentMode = newMode;
    emit(modeChangedSignal, (int)currentMode);
    modeVector.record((int)currentMode);
    
    // In congested mode, proactively drop some low priority packets
    if (currentMode == CONGESTED_MODE) {
        dropLowPriorityPackets();
    }
}

void AdaptiveRouter::dropLowPriorityPackets()
{
    // Drop some low priority packets to make room for higher priority traffic
    int droppedCount = 0;
    while (priorityQueue->getLength() > (recoveryThreshold * queueCapacity) && 
           priorityQueue->dropLowPriorityPacket()) {
        droppedCount++;
        lowPriorityDropped++;
        packetsDropped++;
    }
    
    if (droppedCount > 0) {
        EV_INFO << "Dropped " << droppedCount << " low priority packets due to congestion" << endl;
        emit(packetDroppedSignal, packetsDropped);
    }
}