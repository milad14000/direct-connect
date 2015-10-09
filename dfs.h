// Dynamic Flow Scheduler
// Milad Sharif
// Oct 2013

#ifndef DFS_H
#define DFS_H

#include <string>
#include "./topology.h"

using namespace std;

namespace NSimulator {

// Dynamic flow scheduler
class DFS {
  protected:
    double interval_;
    Topology *topo_;
    int threshold_;
  public:
    DFS(Topology *topo, double interval, int threshold) : 
	    topo_(topo), interval_(interval), threshold_(threshold) {};
    ~DFS() {};
   
    virtual void updateRoute(Flow *f) = 0; 
    virtual void sample();
    double getInt() { return interval_;}
    virtual string type() = 0;
};



class FLLB : public DFS {
  public:
    FLLB(Topology *topo, double interval, int threshold) :
	DFS(topo, interval, threshold) {}; 
    ~FLLB() {};	    
    void updateRoute(Flow *f);
    string type() { return "FLLB"; };
};

class CA_FLLB : public DFS {
  public:
    CA_FLLB(Topology *topo, double interval, int threshold) :
	DFS(topo, interval, threshold) {};
    ~CA_FLLB() {};
    void updateRoute(Flow *f);
    string type() { return "CA_FLLB"; };
};

}
#endif




