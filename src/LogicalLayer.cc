//  Implementation code for the assignment needs to go in here
//      (using class defined in Logical_Layer.h, obviously)

#include "LogicalLayer.h"

LogicalLayer::LogicalLayer() {
    // TODO Auto-generated constructor stub

}

LogicalLayer::~LogicalLayer() {
    // Cancel and delete timer if it exists
    if (broadcastTimer != nullptr) {
        cancelAndDelete(broadcastTimer);
    }
}


void LogicalLayer::handleMessage(cMessage * msg){
    // Check for null pointer
    if (msg == nullptr) {
        EV_ERROR << "Received null message pointer\n";
        return;
    }
    
    if (msg->isSelfMessage()) {
        // This is a timer message (only root node receives this)
        handleBroadcastTimer();
        // Reschedule next broadcast
        scheduleAt(simTime() + broadcastInterval, msg);
    } else {
        // This is a broadcast message from another node
        handleReceivedMessage(msg);
        delete msg;  // Clean up after processing
    }
}

void LogicalLayer::initialize(){
    // 1. Get node ID
    cModule* parent = getParentModule();
    if (parent == nullptr) {
        throw cRuntimeError("Cannot get parent module");
    }
    nodeId = parent->getIndex();
    
    // 2. Initialize statistics variables
    messagesReceived = 0;
    messagesForwarded = 0;
    duplicatesDropped = 0;
    nextSequenceNumber = 1;
    
    // 3. Get configuration parameters
    broadcastInterval = par("broadcastInterval").doubleValue();
    if (broadcastInterval <= 0) {
        broadcastInterval = 2.0;  // Default 2 seconds
    }
    
    // 4. Determine if this is the root node (node with smallest ID)
    isRootNode = (nodeId == 0);
    
    // 5. If root node, start broadcast timer
    if (isRootNode) {
        broadcastTimer = new cMessage("broadcastTimer");
        scheduleAt(simTime() + 1.0, broadcastTimer);  // Start after 1 second
        EV << "Node " << nodeId << ": I am the root node\n";
    } else {
        broadcastTimer = nullptr;
        EV << "Node " << nodeId << ": I am a regular node\n";
    }
}

void LogicalLayer::finish(){
    // Output statistics to log
    EV << "=== Node " << nodeId << " Statistics ===\n";
    EV << "  Messages received: " << messagesReceived << "\n";
    EV << "  Messages forwarded: " << messagesForwarded << "\n";
    EV << "  Duplicates dropped: " << duplicatesDropped << "\n";
    
    if (isRootNode) {
        EV << "  Messages generated: " << (nextSequenceNumber - 1) << "\n";
        recordScalar("messagesGenerated", nextSequenceNumber - 1);
    }
    
    // Record to result file
    recordScalar("messagesReceived", messagesReceived);
    recordScalar("messagesForwarded", messagesForwarded);
    recordScalar("duplicatesDropped", duplicatesDropped);
    
    // Calculate derived metrics
    if (messagesReceived > 0) {
        double duplicateRatio = (double)duplicatesDropped / messagesReceived;
        recordScalar("duplicateRatio", duplicateRatio);
        EV << "  Duplicate ratio: " << duplicateRatio << "\n";
    }
    
    cSimpleModule::finish(); //always remembering to tidy up the base class
}


void LogicalLayer::handleBroadcastTimer() {
    // Create new broadcast message
    BroadcastMessage* bcast = new BroadcastMessage("broadcast");
    bcast->setSourceNodeId(nodeId);
    bcast->setSequenceNumber(nextSequenceNumber++);
    bcast->setTimestamp(simTime());
    bcast->setHopCount(0);
    
    EV << "Node " << nodeId << ": Broadcasting message seq=" 
       << bcast->getSequenceNumber() << " at time " << simTime() << "\n";
    
    // Send to physical layer (will be broadcast to neighbors)
    send(bcast, "port$o");
}

bool LogicalLayer::isMessageDuplicate(int sourceId, int seqNum) {
    std::pair<int, int> msgId = {sourceId, seqNum};
    return receivedMessages.find(msgId) != receivedMessages.end();
}

void LogicalLayer::recordMessage(int sourceId, int seqNum) {
    std::pair<int, int> msgId = {sourceId, seqNum};
    receivedMessages.insert(msgId);
}

void LogicalLayer::handleReceivedMessage(cMessage* msg) {
    // Try to cast to BroadcastMessage
    BroadcastMessage* bcast = dynamic_cast<BroadcastMessage*>(msg);
    if (bcast == nullptr) {
        EV_WARN << "Node " << nodeId << ": Received non-broadcast message, ignoring\n";
        return;
    }
    
    // Validate message fields
    if (bcast->getSourceNodeId() < 0 || bcast->getSequenceNumber() <= 0) {
        EV_ERROR << "Node " << nodeId << ": Invalid message fields: src=" 
                 << bcast->getSourceNodeId() << ", seq=" << bcast->getSequenceNumber() << "\n";
        return;
    }
    
    messagesReceived++;
    
    int srcId = bcast->getSourceNodeId();
    int seqNum = bcast->getSequenceNumber();
    
    EV << "Node " << nodeId << ": Received message from " << srcId 
       << " seq=" << seqNum << " at time " << simTime() << "\n";
    
    // Check if this is a duplicate message
    if (isMessageDuplicate(srcId, seqNum)) {
        duplicatesDropped++;
        EV << "Node " << nodeId << ": Duplicate message, dropping\n";
        return;
    }
    
    // Record the message
    recordMessage(srcId, seqNum);
    EV << "Node " << nodeId << ": New message, will forward\n";
    
    // Forward the message
    forwardMessage(msg);
    messagesForwarded++;
}

void LogicalLayer::forwardMessage(cMessage* msg) {
    // Create a copy of the message and forward it
    cMessage* copy = msg->dup();
    send(copy, "port$o");
    EV << "Node " << nodeId << ": Forwarding message\n";
}
