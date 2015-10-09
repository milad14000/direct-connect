// Milad Sharif
// Nov 2013
// Traffice engineer based on "Hedera:Dynamic FLow Scheduling for Data Center Networks"

#ifndef HEDERA_H
#define HEDERA_H

#include "./te.h"
#include "./tdm.h"
#include "./dfs.h"

namespace NSimulator {

class demandMatrix {
  public:
    demandMatrix(int size);
    //void addFlow(Flow *f);

    int converged(int i,int j) { return M_[i][j].converged_; }
    void updateFlag(int i, int j, int flag) { M_[i][j].converged_ = flag; }
    double demand(int i, int j) { return M_[i][j].demand_; }
    void updateDemand(int i, int j, double d) { M_[i][j].demand_ = d; }
    int rl(int i, int j) {  return M_[i][j].recvLimited_; };
    void updateRl(int i, int j, int r) { M_[i][j].recvLimited_ = r; }
    void clear();
    
  private:   
    class estDemand {
      public:	    
        estDemand():demand_(1), converged_(0), recvLimited_(0) {}; 
        double demand_;
        //int numFlows_;
        int converged_;
        int recvLimited_; 
    };
    int size_;
    estDemand **M_;

};

class Hedera : public DFS {
  public:
    Hedera(Topology *topo, double interval, int threshold) :
  	DFS(topo, interval, threshold) {
	  M_ = new demandMatrix(topo->getNumServers());
	};   
    ~Hedera(); 
    void sample();
    void updateRoute(Flow *f);
    string type() { return "Hedera"; } 	  

  protected:	   
    vector<Flow *> flows_;			// Large flows
    demandMatrix *M_;
    void estimateDemand();			// Demand estimation based on Fig. 7
    void  estSrc(int src);			// Check Fig. 7
    int estDst(int dst);    			
    double unreservedBW(int tor1, int tor2, int hop);
    int search(Flow *f); 			// Search for a new route using Global First Fit Fig. 5
};

}

#endif
