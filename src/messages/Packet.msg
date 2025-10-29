//
// Packet message definition for Adaptive Priority Router
// This message represents network packets with priority information
//

message Packet {
    int priorityLevel;     // 3=high, 2=medium, 1=low
    string type;           // "voice", "video", "data"
    simtime_t timestamp;   // creation time
}