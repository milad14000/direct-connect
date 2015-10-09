#include "b4.h"
#include "log.h"
#include "util.h"

#include <cstdlib>
#include <time.h>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>

#define EPS 1e-6
namespace NSimulator {

B4::B4(TDM *tdm) {
  int n=tdm->demands.size();
  logger.info("*** Creating flow groups ... ");
  // Creating flow groups based on the traffic demand matrix
  for(int i=0; i<n; i++){
    struct demand *temp = &(tdm->demands[i]);
    FG f(temp->src+tdm->getNumServers(), temp->dst+tdm->getNumServers(), temp->d);
    fg.push_back(f); 
    logger.info("\t%d->%d: %.3f",f.src(), f.dst(), f.demand());
  }
}

B4::~B4() {
}
	
void 
B4::calW() { 
  if (fg.size()==0)
    return;
  // Initialize links with normalized cap
  vector<Link*>* list = topo->getLinkList(); 
  for(int i=0; i<list->size(); i++) {
    Link* l=(*list)[i];
    double cap = l->cap*topo->getNumTORs()/topo->getNumServers()/topo->getOverSub()/1e10; 
    l->cap = 0.7 * cap;
    l->acap = l->cap;
  }
  // Fill up the direct connections
  for(int i=0; i<fg.size(); i++) {  
    int src = fg[i].src(), dst=fg[i].dst();
    double demand = fg[i].demand();
    if (src == dst) {
      fg[i].setDemand(0);
      continue;
    }
    // Find the direct connecton
    Link* L = topo->findLink(src, dst); 
    L->flows.push_back(fg[i].flow()); 		/* update topo */
    (fg[i].flow())->links.push_back(L);

    fg[i].paths.push_back(L);			/* add direct connection to the list */	
    fg[i].W.push_back(0);

  }

int exit=0;
do {
  // Find minDemand
  double minDemand = 1e20;
  for(int i=0; i<fg.size(); i++) {
    double fairShare, demand=fg[i].demand();
    if (demand==0)
      continue;
    Link *secondHop = fg[i].paths.back();
    Link *thirdHop = topo->findLink(secondHop->dst, fg[i].dst());

    if (thirdHop==NULL) {
      fairShare = secondHop->acap/secondHop->flows.size();	    
    } else {
      fairShare = util::min(secondHop->acap/secondHop->flows.size() ,
		     thirdHop->acap/thirdHop->flows.size());
    }
    minDemand = util::min( minDemand, min(demand, fairShare));
  }
  logger.debug("Min demand: %.3f", minDemand);

  for(int i=0; i<fg.size(); i++) {
    int src = fg[i].src(), dst=fg[i].dst();
    if (fg[i].demand() <= EPS)
      continue;

    // Update residual demands
    fg[i].setDemand(fg[i].demand() - minDemand);
    
    // Update weights
    fg[i].W.back() += (minDemand / fg[i].demand_);
    
    // Update available capacity
    Link *secondHop = fg[i].paths.back();
    Link *thirdHop = topo->findLink(secondHop->dst, dst);
    secondHop->acap -= minDemand;
    if(thirdHop!=NULL)
      thirdHop->acap -= minDemand;

    // Check if the demand is satisfied 
    if(fg[i].demand() <= EPS) {
      topo->removeflow(fg[i].flow());
      continue;
    }

    // Check if the links are saturated
    if (secondHop->acap==0 || (thirdHop!=NULL && thirdHop->acap==0)) {
      // Clear the old route
      topo->removeflow(fg[i].flow());

      // Find a new route
      int TORId = selectHop(fg[i]);
      if (TORId == -1) {
        logger.error("TE couldnt satisfy the demands");
        return;
      }	      
      secondHop = topo->findLink(src, TORId);
      
      fg[i].paths.push_back(secondHop);
      fg[i].W.push_back(0);
      secondHop->flows.push_back(fg[i].flow());
      (fg[i].flow())->links.push_back(secondHop);

      thirdHop = topo->findLink(TORId, dst);
      thirdHop->flows.push_back(fg[i].flow());
      (fg[i].flow())->links.push_back(thirdHop);

      logger.debug("found a 2-hop route: %d->%d->%d, link cap: %.3f, %.3f",
		      fg[i].src(), TORId, fg[i].dst(), secondHop->acap, thirdHop->acap); 
    }
  }
  // Check the exit condition
  exit=1;
  for (int i=0; i<fg.size(); i++) {
    if (fg[i].demand()==0)
      continue;	    
    
    Link *secondHop = fg[i].paths.back();
    Link *thirdHop = topo->findLink(secondHop->dst, fg[i].dst());

    if ( fg[i].paths.size()<(topo->getNumTORs()-1)) {   			/* There are still unused 2-routes */
      exit=0;
      logger.debug("More routes");
      break;
    } else if( secondHop->acap>0 && (thirdHop!=NULL && thirdHop->acap>0) )  { 	/* The las 2-hop route is not saturated */
      exit=0;
      logger.debug("More cap.");
      break;
    }
  }
  logger.debug("Exit %d", exit);

} while(!exit);
 
  logger.info("*** Generating Tunnel groups ...");
  for(int i=0; i<fg.size(); i++) {
    logger.info("  %d->%d: %.3f",fg[i].src(), fg[i].dst(), fg[i].demand_);
    for(int j=0; j<fg[i].paths.size(); j++) {
      logger.info("\t%d->%d, weight:%.3f", fg[i].src(), fg[i].paths[j]->dst, fg[i].W[j]);
    }
  }
}


int 
B4::selectHop(FG fg) {
  logger.debug("Size: %d", fg.paths.size());
  if (fg.paths.size() == (topo->getNumTORs()-1))
    return -1;

  int src=fg.src(), dst=fg.dst();
  int TORId, new_route;

  int random=0;
  srand (time(NULL));   /* initialize random seed: */

  // Randomly select a 2-hop route as the next shortest path
  if(random==1) { 
    do {
      TORId = ( rand() % topo->getNumTORs() ) + topo->getNumServers();
      new_route=1;
      for(int i=0; i<fg.paths.size(); i++) {
        if ( fg.paths[i]->dst == TORId )
          new_route=0;	
      }
    } while (TORId==src || !new_route);
   
  } else { 
    // Select least congested 2-hop route
    double maxCap = 0;
    for(int i=0; i<topo->getNumTORs(); i++) {
      int temp = i+topo->getNumServers();
      Link *l = topo->findLink(fg.src(), temp);
      if(l==NULL)
        continue;

      // Check if it's a new hop
      new_route = 1;
      for(int i=0; i<fg.paths.size(); i++) {                                                            
        if ( fg.paths[i]->dst == temp )                                                                
          new_route=0;                                                                                  
      }

      if(new_route && l->acap>=maxCap) {
        maxCap = l->acap;
        TORId = temp;
      }
    }
  }

  return TORId;  
}


Link *
B4::getLink(int src, int dst) {
  int i;
  for (i=0; i<fg.size(); i++) {
    if(fg[i].src()==src && fg[i].dst()==dst)
      break;
  }
  if(i==fg.size()) {
    logger.error("(%d, %d) not found in the flow groups", src, dst);
    return NULL;
  }

  /*FIXME for the case that sum(W_i) < 1 (the demand is not satisfied) */
  double r = (double)rand() / (double)RAND_MAX;
  //logger.debug("Searching for %d->%d in %d, %.2f", src,dst, fg[i].W.size(), r);

  for(int j=0; j<fg[i].W.size(); j++) {
    r -= fg[i].W[j];
    //logger.debug("weight: %.2f, r: %.2f", fg[i].W[j], r);
    if (r<=0)
      return fg[i].paths[j];
  } 
  return NULL;
}

}
