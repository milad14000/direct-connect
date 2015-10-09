#include "./dfs.h"
#include "./log.h"
#include "./util.h"

#include <cassert>
#include <cstdlib>

namespace NSimulator {

void 
DFS::sample() {	  
  vector<Flow*> *flows = topo_->getFlowList();
  for (int i=0; i<flows->size(); i++) {
    Flow *f = flows->at(i);
    updateRoute(f);  
  }
}

void 
FLLB::updateRoute(Flow *f) {
  Link *oldHop;
  // Find the old route
  int hops = f->links.size();
  if (hops == 2) 
    return;
  else
    oldHop = f->links[1];

  // Check current route congestion
  int congested = 0;
  for(int i=0; i<f->links.size(); i++) {
    Link *l = f->links[i];
    if(l->flows.size() > threshold_){
      congested = 1;	
      break;
    }    
  }
  if(congested == 0)
    return;

  // Remove the old route  
  for (int i=0; i<f->links.size(); i++) {
    Link *l = f->links[i];
    for (int j=0; j<l->flows.size(); j++) {
      if (f == l->flows[j]) {
        l->flows.erase(l->flows.begin() + j);  
      }
    }
  }
  f->links.clear();

  // Select a new route
  int server_per_rack, tor1, tor2;
  int src=f->src, dst=f->dst;
  server_per_rack = topo_->getNumSPR();
  tor1 = src / server_per_rack + topo_->getNumServers();
  tor2 = dst / server_per_rack + topo_->getNumServers();
  assert(oldHop->src==tor1);

  
  
  int numRoutes = topo_->getNumTORs();                          // # of TORs in direct interconnection
  int offset = topo_->getNumServers();
  if( topo_->getNumTORs() < topo_->getNumSwitches()) {
    numRoutes = topo_->getNumSwitches() - topo_->getNumTORs();  // # of Spines in bipartite interconnection     
    offset += topo_->getNumTORs();
  }

  int HopId;
  do {   /* Select a new route randomly */
    HopId = rand() % numRoutes + offset;
  } while(HopId==tor1 || HopId==oldHop->dst);			// Make sure it selected a new route
  
  topo_->insertFlow(f, HopId);

}

void 
CA_FLLB::updateRoute(Flow *f) {
  Link *secondHop, *thirdHop;
  int LCR, minNumFlows;
  // Find the old route
  int hops = f->links.size();
  if (hops == 2)
    return;
  else if(hops == 3) {
    secondHop = f->links[1];
    minNumFlows = secondHop->flows.size();
  } else {
    secondHop = f->links[1];
    thirdHop = f->links[2];
    minNumFlows =  util::max(secondHop->flows.size(), thirdHop->flows.size()); 
  }  
  LCR = secondHop->dst - 1;

  // Remove the old route
  for(int i=0; i<f->links.size(); i++) {
    Link *l = f->links[i];
    for(int j=0; j<l->flows.size(); j++) {
      if(f == l->flows[j]) {
        l->flows.erase(l->flows.begin() + j);
      } 
    }
  }
  f->links.clear();

  // Select a new route
  int server_per_rack, tor1, tor2, TORId;
  int src=f->src, dst=f->dst;
  server_per_rack = topo_->getNumServers() / topo_->getNumTORs();                                 
  tor1 = src / server_per_rack + topo_->getNumServers();                                          
  tor2 = dst / server_per_rack + topo_->getNumServers();                                          
  
  // Find least congested route
  int numRoutes = topo_->getNumTORs();				// # of TORs in direct interconnection
  int offset = topo_->getNumServers();
  if( topo_->getNumTORs() < topo_->getNumSwitches()) { 
    numRoutes = topo_->getNumSwitches() - topo_->getNumTORs();	// # of Spines in bipartite interconnection	
    offset += topo_->getNumTORs();
  }

  for(int i=0; i<numRoutes; i++) {
    int HopId = offset + i;					// SpineId or TORId based on the interconnection	
    if(HopId == tor1)
      continue;
    secondHop = topo_->findLink(tor1, HopId);
    thirdHop  = topo_->findLink(HopId, tor2);
    int n = secondHop->flows.size();
    if(thirdHop!=NULL)
      n = util::max(secondHop->flows.size(), thirdHop->flows.size());	    
    if(n < minNumFlows) {
      minNumFlows = n;
      LCR = HopId;
    } 
  }

  // Add the new route
  topo_->insertFlow(f, LCR);

}

}
