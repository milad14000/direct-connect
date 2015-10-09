
// Milad Sharif
// Aug 2013

#ifndef TE_H
#define TE_H

#include "./topology.h"
#include "./tdm.h"

#include <string>
#include <vector>

using namespace std;

namespace NSimulator {

// Flow Group
class FG {
  protected:	
    int src_, dst_;
    double rdemand_;            /* Residual demand */
    Flow flow_; 		/* Dummy flow from src TOR to dst TOR */
  public:
    double demand_;             /* Normalized demand */
    vector<Link*> paths;	/* List of the first hops of the shortest-paths from src to dst */
    vector<double> W;           /* Weights corresponding to each path */
    vector<double> Lambda;	/* Splits corresponding to each path 
				   mainly added for waterfilling */
    				

    FG(int src, int dst, double d) : src_(src), 
	dst_(dst), demand_(d), rdemand_(d) 
    {
      flow_.src  = src;
      flow_.dst  = dst;  
      flow_.size = d;    
    }
    
    double demand() { return rdemand_; }
    void setDemand(double d) { 
      rdemand_ = d;
      flow_.size = d;
    }
    int src() { return src_; }
    int dst() { return dst_; } 
    Flow* flow() { return &flow_; }  
    void addLink(Link *l) { flow_.links.push_back(l); }    
    vector<Link *>* links() { return &flow_.links; }
}; 

// Centralized: traffic engineering
class TE {	
  protected:
    vector<FG> fg;
  
  public:
    Topology* topo;
    TE() {};
    ~TE() {};
    virtual void calW() = 0 ;
    virtual string type() = 0;
    virtual Link *getLink(int src, int dst) = 0;
};

}

#endif
