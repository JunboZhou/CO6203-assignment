package assignment;

@license(LGPL);

//--------------You may add parameters to the logical layer--------------//
//------(such as wait times for keepalives or supporting protocols)------//
//-------Please do NOT change the gates or add any other components------//


simple LogicalLayer
{
    parameters:
		//add wait times as necessary here
    gates:
        //Do not alter this
        inout port;
}

//---------template code------do not alter---------//
//-----parameters may be set using omnetpp.ini-----//

simple Phy
{
    gates:
        inout layer;
        input medium @directIn;
}

module CO6203Node
{
    parameters:
        double world_width = default(800);
        double world_offset = default(-400);
        double x @mutable = default(uniform(0,world_width) + world_offset);
        double y @mutable = default(uniform(0,world_width) + world_offset);
        @display("p=$x,$y;;b=20,20,oval,red,black,2");
    submodules:
        physical: Phy;
        logical: LogicalLayer;
    connections:
        logical.port <--> physical.layer;
}

network Manet
{
    parameters:
	    int n = default(20);
	    double temperature = default(5);
	    double limit = default(200);
	    @display("bgb=0,0,t= ");
    submodules:
        nodes[n]: CO6203Node;   
}
