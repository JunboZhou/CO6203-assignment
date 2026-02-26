//DO NOT ALTER THE CODE IN THIS FILE

#ifndef PHY_H_
#define PHY_H_

#include <omnetpp.h>
#include <random>
using namespace omnetpp;

class Phy : public cSimpleModule {
    double temperature, limit;
public:
    Phy();
    virtual ~Phy();
protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
};

Define_Module(Phy);

#endif /* PHY_H_ */
