//  Define logical layer class here
//  (member variables, additional methods etc)

#ifndef LOGICALLAYER_H_
#define LOGICALLAYER_H_
#include <omnetpp.h>
#include <set>
#include <utility>
using namespace omnetpp;

// BroadcastMessage class: Custom message for broadcast protocol
class BroadcastMessage : public cMessage {
private:
    int sourceNodeId;      // Source node ID
    int sequenceNumber;    // Sequence number for message identification
    simtime_t timestamp;   // Timestamp for delay calculation
    int hopCount;          // Hop count (optional, for TTL mechanism)
    
public:
    BroadcastMessage(const char* name = nullptr) : cMessage(name) {
        sourceNodeId = -1;
        sequenceNumber = 0;
        timestamp = 0;
        hopCount = 0;
    }
    
    BroadcastMessage(const BroadcastMessage& other) : cMessage(other) {
        sourceNodeId = other.sourceNodeId;
        sequenceNumber = other.sequenceNumber;
        timestamp = other.timestamp;
        hopCount = other.hopCount;
    }
    
    // Getter and Setter methods
    void setSourceNodeId(int id) { sourceNodeId = id; }
    int getSourceNodeId() const { return sourceNodeId; }
    
    void setSequenceNumber(int seq) { sequenceNumber = seq; }
    int getSequenceNumber() const { return sequenceNumber; }
    
    void setTimestamp(simtime_t t) { timestamp = t; }
    simtime_t getTimestamp() const { return timestamp; }
    
    void setHopCount(int hops) { hopCount = hops; }
    int getHopCount() const { return hopCount; }
    void incrementHopCount() { hopCount++; }
    
    virtual BroadcastMessage* dup() const override {
        return new BroadcastMessage(*this);
    }
};

class LogicalLayer : public cSimpleModule{
private:
    // Message deduplication set: stores (sourceId, sequenceNumber) pairs
    std::set<std::pair<int, int>> receivedMessages;
    
    // Sequence number counter for root node
    int nextSequenceNumber;
    
    // Broadcast message sending interval (seconds)
    double broadcastInterval;
    
    // Timer message pointer
    cMessage* broadcastTimer;
    
    // Statistics
    int messagesReceived;      // Total messages received
    int messagesForwarded;     // Messages forwarded
    int duplicatesDropped;     // Duplicate messages dropped
    
    // Node identification
    int nodeId;                // Current node ID
    bool isRootNode;           // Whether this is the root node
    
public:
    LogicalLayer();
    virtual ~LogicalLayer();
    
protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage* msg) override;
    virtual void finish() override;
    
private:
    // Helper methods
    void handleBroadcastTimer();
    void handleReceivedMessage(cMessage* msg);
    bool isMessageDuplicate(int sourceId, int seqNum);
    void recordMessage(int sourceId, int seqNum);
    void forwardMessage(cMessage* msg);
};

Define_Module(LogicalLayer);

#endif /* LOGICALLAYER_H_ */
