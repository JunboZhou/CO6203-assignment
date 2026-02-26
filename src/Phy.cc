//DO NOT ALTER THE CODE IN THIS FILE

#include "Phy.h"
#include <sstream>

double clamp(double d, double min, double max) {
  double t = d < min ? min : d;
  return t > max ? max : t;
}

Phy::Phy() {
    // TODO Auto-generated constructor stub
}

Phy::~Phy() {
    // TODO Auto-generated destructor stub
}
void Phy::initialize(){
    temperature = getParentModule()->getParentModule()->par("temperature").doubleValue();
    limit = getParentModule()->getParentModule()->par("limit").doubleValue();
    getParentModule()->getDisplayString().setTagArg("r",0,std::to_string(limit).c_str());
    if(temperature > 0){
        cMessage * timer = new cMessage();
        scheduleAt(simTime() + SimTime(100,SIMTIME_MS),timer);
    }
}
void Phy::handleMessage(cMessage *msg){
    if(msg->isSelfMessage()){
        cancelEvent(msg);
        scheduleAt(simTime() + SimTime(100,SIMTIME_MS),msg);
        //do position update
        getParentModule()->par("x").setDoubleValue(clamp(getParentModule()->par("x").doubleValue() + normal(0,1)*temperature,getParentModule()->par("world_offset").doubleValue(),getParentModule()->par("world_width").doubleValue()));
        getParentModule()->par("y").setDoubleValue(clamp(getParentModule()->par("y").doubleValue() + normal(0,1)*temperature,getParentModule()->par("world_offset").doubleValue(),getParentModule()->par("world_width").doubleValue()));
        getParentModule()->getDisplayString().setTagArg("tt",0,std::to_string(getParentModule()->par("x").doubleValue()).c_str());
        getParentModule()->getDisplayString().setTagArg("tt",0,std::to_string(getParentModule()->getIndex()).c_str());
    }
    else if(strcmp(msg->getArrivalGate()->getName(), "layer$i") == 0 ){
        //find all devices in range and sendDirect
        for (cModule::SubmoduleIterator it(this->getParentModule()->getParentModule()); !it.end(); ++it){
            for (cModule::SubmoduleIterator jt(*it); !jt.end(); ++jt){
                Phy* ptr = dynamic_cast<Phy*>(*jt);
                if(ptr && ptr != this){
                    //is it in range
                    double dx = ptr->getParentModule()->par("x").doubleValue() - getParentModule()->par("x").doubleValue();
                    double dy = ptr->getParentModule()->par("y").doubleValue() - getParentModule()->par("y").doubleValue();
                    if( (dx*dx + dy*dy) < limit*limit){
                        sendDirect(msg->dup(),SimTime(5,SIMTIME_MS),0,ptr,"medium");
                    }
                }
            }
        }
        delete msg;
    }
    else if(strcmp(msg->getArrivalGate()->getName(), "medium") == 0 ){
        //send all incoming packets to logical layer
        send(msg, "layer$o");
    }
}
