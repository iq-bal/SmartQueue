//
// PriorityQueue.cc - Implementation of priority-based packet queue
//

#include "PriorityQueue.h"

PriorityQueue::PriorityQueue(int capacity) : maxCapacity(capacity), roundRobinCounter(0)
{
}

PriorityQueue::~PriorityQueue()
{
    clear();
}

bool PriorityQueue::enqueue(Packet *packet)
{
    if (getLength() >= maxCapacity) {
        return false;  // Queue is full
    }
    
    switch (packet->getPriorityLevel()) {
        case 3:  // High priority (voice)
            highPriorityQueue.push_back(packet);
            break;
        case 2:  // Medium priority (video)
            mediumPriorityQueue.push_back(packet);
            break;
        case 1:  // Low priority (data)
            lowPriorityQueue.push_back(packet);
            break;
        default:
            EV_ERROR << "Invalid priority level: " << packet->getPriorityLevel() << endl;
            return false;
    }
    
    return true;
}

Packet* PriorityQueue::dequeueNormal()
{
    // Fair scheduling using round-robin approach
    // Try each queue in rotation to ensure fairness
    
    for (int attempts = 0; attempts < 3; attempts++) {
        switch (roundRobinCounter % 3) {
            case 0:  // Try high priority
                if (!highPriorityQueue.empty()) {
                    Packet* packet = highPriorityQueue.front();
                    highPriorityQueue.pop_front();
                    roundRobinCounter++;
                    return packet;
                }
                break;
            case 1:  // Try medium priority
                if (!mediumPriorityQueue.empty()) {
                    Packet* packet = mediumPriorityQueue.front();
                    mediumPriorityQueue.pop_front();
                    roundRobinCounter++;
                    return packet;
                }
                break;
            case 2:  // Try low priority
                if (!lowPriorityQueue.empty()) {
                    Packet* packet = lowPriorityQueue.front();
                    lowPriorityQueue.pop_front();
                    roundRobinCounter++;
                    return packet;
                }
                break;
        }
        roundRobinCounter++;
    }
    
    return nullptr;  // All queues are empty
}

Packet* PriorityQueue::dequeueStrict()
{
    // Strict priority: High > Medium > Low
    
    if (!highPriorityQueue.empty()) {
        Packet* packet = highPriorityQueue.front();
        highPriorityQueue.pop_front();
        return packet;
    }
    
    if (!mediumPriorityQueue.empty()) {
        Packet* packet = mediumPriorityQueue.front();
        mediumPriorityQueue.pop_front();
        return packet;
    }
    
    if (!lowPriorityQueue.empty()) {
        Packet* packet = lowPriorityQueue.front();
        lowPriorityQueue.pop_front();
        return packet;
    }
    
    return nullptr;  // All queues are empty
}

int PriorityQueue::getLength() const
{
    return highPriorityQueue.size() + mediumPriorityQueue.size() + lowPriorityQueue.size();
}

int PriorityQueue::getHighPriorityLength() const
{
    return highPriorityQueue.size();
}

int PriorityQueue::getMediumPriorityLength() const
{
    return mediumPriorityQueue.size();
}

int PriorityQueue::getLowPriorityLength() const
{
    return lowPriorityQueue.size();
}

bool PriorityQueue::isEmpty() const
{
    return getLength() == 0;
}

bool PriorityQueue::isFull() const
{
    return getLength() >= maxCapacity;
}

bool PriorityQueue::dropLowPriorityPacket()
{
    if (!lowPriorityQueue.empty()) {
        Packet* packet = lowPriorityQueue.back();
        lowPriorityQueue.pop_back();
        delete packet;
        return true;
    }
    return false;
}

void PriorityQueue::clear()
{
    // Clean up all packets in all queues
    while (!highPriorityQueue.empty()) {
        delete highPriorityQueue.front();
        highPriorityQueue.pop_front();
    }
    
    while (!mediumPriorityQueue.empty()) {
        delete mediumPriorityQueue.front();
        mediumPriorityQueue.pop_front();
    }
    
    while (!lowPriorityQueue.empty()) {
        delete lowPriorityQueue.front();
        lowPriorityQueue.pop_front();
    }
}

void PriorityQueue::printQueueStatus() const
{
    EV_INFO << "Queue Status - High: " << getHighPriorityLength() 
            << ", Medium: " << getMediumPriorityLength() 
            << ", Low: " << getLowPriorityLength() 
            << ", Total: " << getLength() << "/" << maxCapacity << endl;
}