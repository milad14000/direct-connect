// Block Level Load balancing
// Milad Sharif
// Nov 2013

#ifndef BLLB_H
#define BLLB_H

#include "./dfs.h"

namespace NSimulator {

class BLLB : public DFS {
  public:
    BLLB(Topology *topo, double interval, int threshold);
    ~BLLB();
    
    void updateRoute(Flow *f);
    void sample();
    void updateW();
    string type() { return "BLLB"; };
    void printW(int i, int j); 
     
  private:
    int threshold1_;
    int threshold2_; 
    double ***W; 			// 3D array for weights  	
    int **C;				// 2D array to save the state of the direct links    

    int selectHop(int tor1, int tor2); 	// Select a new route based on the weights    
    int checkW(int i, int j);
    void explore(int i, int j);
    void adjust(int i, int j);

};

}

#endif
