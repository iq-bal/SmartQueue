//
// TrafficGenerator.cc - Implementation of traffic generation module
//

#include "TrafficGenerator.h"

Define_Module(TrafficGenerator);

TrafficGenerator::TrafficGenerator()
{
    sendTimer = nullptr;
    packetsSent = 0;
}

TrafficGenerator::~TrafficGenerator()
{
    cancelAndDelete(sendTimer);
}

void TrafficGenerator::initialize()
{
    // Read parameters from NED file
    packetType = par("packetType").stringValue();
    priorityLevel = par("priorityLevel");
    sendInterval = par("sendInterval");
    
    // Initialize statistics
    packetsSent = 0;
    packetSentSignal = registerSignal("packetSent");
    
    // Create and schedule first packet
    sendTimer = new cMessage("sendTimer");
    scheduleNextPacket();
    
    EV_INFO << "TrafficGenerator initialized - Type: " << packetType 
            << ", Priority: " << priorityLevel 
            << ", Interval: " << sendInterval << "s" << endl;
}

void TrafficGenerator::handleMessage(cMessage *msg)
{
    if (msg == sendTimer) {
        // Time to send a packet
        Packet *packet = createPacket();
        
        if (packet != nullptr) {
            send(packet, "out");
            packetsSent++;
            emit(packetSentSignal, packetsSent);
            
            EV_DEBUG << "Sent packet #" << packetsSent 
                     << " of type " << packetType 
                     << " with priority " << priorityLevel << endl;
        }
        
        // Schedule next packet
        scheduleNextPacket();
    } else {
        EV_ERROR << "Unknown message received" << endl;
        delete msg;
    }
}

void TrafficGenerator::finish()
{
    EV_INFO << "TrafficGenerator finished - Total packets sent: " << packetsSent << endl;
    recordScalar("packetsSent", packetsSent);
}

void TrafficGenerator::scheduleNextPacket()
{
    // Schedule next packet based on the configured interval
    // Using exponential distribution for realistic traffic patterns
    double nextTime = exponential(sendInterval);
    scheduleAt(simTime() + nextTime, sendTimer);
}

Packet* TrafficGenerator::createPacket()
{
    Packet *packet = new Packet("trafficPacket");
    
    // Set packet properties
    packet->setPriorityLevel(priorityLevel);
    packet->setType(packetType.c_str());
    packet->setTimestamp(simTime());
    
    // Set packet name for easier debugging
    std::string packetName = packetType + "_packet_" + std::to_string(packetsSent + 1);
    packet->setName(packetName.c_str());
    
    return packet;
}