//
// PriorityQueue.h - Priority-based packet queue implementation
// Supports three priority levels with different scheduling modes
//

#ifndef PRIORITYQUEUE_H
#define PRIORITYQUEUE_H

#include <omnetpp.h>
#include <deque>
#include "../messages/Packet_m.h"

using namespace omnetpp;

enum SchedulingMode {
    NORMAL,     // Fair scheduling between all priorities
    CONGESTED   // Strict priority: High > Medium > Low
};

class PriorityQueue : public cObject
{
private:
    std::deque<Packet*> highPriorityQueue;    // Priority 3 (voice)
    std::deque<Packet*> mediumPriorityQueue;  // Priority 2 (video)
    std::deque<Packet*> lowPriorityQueue;     // Priority 1 (data)
    
    int maxCapacity;
    int roundRobinCounter;  // For fair scheduling in NORMAL mode
    
public:
    PriorityQueue(int capacity = 200);
    virtual ~PriorityQueue();
    
    // Queue operations
    bool enqueue(Packet *packet);
    Packet* dequeueNormal();    // Fair scheduling
    Packet* dequeueStrict();    // Strict priority scheduling
    
    // Queue status
    int getLength() const;
    int getHighPriorityLength() const;
    int getMediumPriorityLength() const;
    int getLowPriorityLength() const;
    
    bool isEmpty() const;
    bool isFull() const;
    
    // Drop operations
    bool dropLowPriorityPacket();
    void clear();
    
    // Statistics
    void printQueueStatus() const;
};

#endif // PRIORITYQUEUE_H