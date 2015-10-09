#include "./topology.h"

namespace NSimulator {

class TE;

class Bipartite: public Topology {
  public:
   Bipartite();
   ~Bipartite();
   virtual void buildTopology(int num_rack, int server_per_rack, int oversub);
   virtual int route(Flow *f, TE *te);
   void insertFlow(Flow *f, int hop); 
  private:
   int num_spine;
};

}
