#include "./topology.h"
#include "./generator.h"
#include "./te.h"
#include "./tdm.h"
#include "./dfs.h"

#include <vector>
#include <deque>

class Generator;
class Topology;

namespace NSimulator {

//Base of the simulator
//Input: topo & flowlist_torun
//Output: flowlist_fin
class Simulator {
 public:
   Simulator() {};
   virtual ~Simulator();

   //fixed arrival interval
   //fixed flow size
   //uniform src/dst
   //this function is obsolete
   void set_FFUgenerator(double avgInt, double flowSize, TDM *tdm, int seed);

   //run this after set the topology and flowlist_torun;
   virtual void run();

   // Helper
   // you don't have to setup gen, but it might be helpful to generate the flow
   Generator *gen;
   // Have to be set before run
   Topology *topo;
   // Generate after run
   vector<Flow*> flowlist_fin;
   // Have to be set before run
   deque<Flow*> flowlist_torun;

   //Traffic demand matrix
   TDM *tdm;
   //Traffic engineer
   TE *te;
   // Dynamic flow scheduler 
   DFS *dfs;

   double currentTime;

 private:
   void initRun();
};

}

