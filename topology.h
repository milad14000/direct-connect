#ifndef TOPOLOGY_H
#define TOPOLOGY_H
#include <vector>
#include <map>

using namespace std;
namespace NSimulator {

class TE;

struct Link;
struct Flow;

//Node, includes server and switches
struct Node {
  public:
   enum NodeType {
      Switch = 0,
      Server = 1
   };

   int id;
   NodeType type;
   vector<Link*> links;
};

//Link, directional
struct Link {
  public:
    double cap;
    int src, dst;
    vector<Flow*> flows;

    double acap; //available capacity for min-max fairness
    int aflow; //unsatisfied flow
    //vector<double> Lambda; // Hack for iterative waterfilling 
    map<Flow *, double> Lambda;
};

//Flows
struct Flow {
 public:
  unsigned int flowId;
  int src, dst;
  double fsize; //flow size
  double size;//remaining flow size
  double arrival_time;
  double exp_fintime;//expected finish time
  double rate;//rate since lastEventTime
  vector<Link*> links;

  double lastEventTime;
  bool satisfied;//used to calculate MMF
  int reserved;	// Used for bandwidth reservation for Hedera
};

//Base class for the network topology
class Topology {
  public:
   Topology();
   virtual ~Topology();

   virtual void buildTopology(int num_rack, int server_per_rack, int oversub)=0;

   // Return the length of the route
   // if unable to find the route
   // return -1
   virtual int route(Flow *f, TE *te)=0;

   // Remove flow from topology
   virtual void removeflow(Flow *f);

   // Insert a flow in topology
   //virtual int insertFlow(Flow *f, int hop); /* Only support 2-hop routes and direct-connections */

   // Helper function to print the topology for debug
   virtual void printTopology();

   //calculate the max min fair for running flows
   //and update their expected finish time
   //return the next flow to finish
   virtual Flow* calMMF(double curTime);

   virtual void insertFlow(Flow *f, int hop)=0;

   int getNumServers() {return num_server;}
   int getNumSwitches() {return nodelist.size() - num_server;}
   int getNumNodes() {return nodelist.size();}
   int getNumTORs() { return num_rack_; }
   int getNumSPR() { return server_per_rack_; }
   int getNumLinks() {return linklist.size();}
   int getNumFlows() {return flowlist.size();}
   int getOverSub() { return oversub_; }
   vector<Node*> *getNodeList() {return &nodelist;}
   vector<Link*> *getLinkList() {return &linklist;}
   vector<Flow*> *getFlowList() {return &flowlist;}
   Link* findLink(int src, int dst);

  protected:
   vector<Node*> nodelist;
   vector<Link*> linklist;
   vector<Flow*> flowlist;
   int num_server;
   int num_rack_;
   int server_per_rack_;
   int oversub_;
   bool init;
};

}

#endif
