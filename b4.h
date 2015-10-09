
// Milad Sharif
// Aug 2013
// Centralized TE based on "B4: Experience with Globally-Deployed Software-Defined WAN"

#ifndef B4_H
#define B4_H

#include "./tdm.h"
#include "./te.h"

#include <string>
#include <vector>

using namespace std;

namespace NSimulator {

// Centralized: traffic engineering
class B4 : public TE {	
  protected:
    //vector<FG> fg;
    int selectHop(FG flowGroup);
  
  public:
    //Topology* topo;
    B4(TDM *tdm);
    ~B4();
    void calW();
    string type() { return "B4"; }
    Link *getLink(int src, int dst);

};


}

#endif
