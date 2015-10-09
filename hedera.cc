#include "./hedera.h"
#include "./log.h"
#include "./util.h"

#include <cstdlib>

namespace NSimulator {




Flow *gen(int i, int j){
  Flow *f = new Flow;
  f->src = i;
  f->dst = j;
  return f;
}

Hedera::~Hedera() {
  delete M_;
}

void 
Hedera::sample() {
  vector<Flow*> *flowlist = topo_->getFlowList();
  for(int i=0; i<flowlist->size(); i++) {
    Flow *f = flowlist->at(i);
    // Detect the large flows /* FIXME */ 
    flows_.push_back(f);
  }
  
  // Estimate the demand
  estimateDemand();

  // Update routes
  for(int i=0; i<flows_.size(); i++)
    updateRoute(flows_[i]);

  // Clear the list of big flows
  flows_.clear();
}

void
Hedera::estimateDemand() {
  int changed, num_server = topo_->getNumServers();
  do {
    changed = 0;
    for(int i=0; i<num_server; i++)	  
      estSrc(i);

    for(int i=0; i<num_server; i++)
      changed += estDst(i);
  } while(changed);     
}

void
Hedera::estSrc(int host) {
  double d_F = 0;
  int n_U = 0;

  for(int i=0; i<flows_.size(); i++) {
    Flow *f = flows_[i];
    if (f->src == host) {
      if( M_->converged(f->src, f->dst) ) 
        d_F += M_->demand(f->src, f->dst);	      
      else 
        n_U += 1;
    }
  }
  double e_s = (1.0 - d_F) / n_U; 
  for(int i=0; i<flows_.size(); i++) {
    Flow *f = flows_[i];
    if( f->src == host && !M_->converged(f->src, f->dst)) {
      M_->updateDemand(f->src, f->dst, e_s);	    
    }
  }
}


int
Hedera::estDst(int host) {
  double d_T = 0, d_s = 0;
  int changed = 0, n_R = 0;
  for(int i=0; i<flows_.size(); i++) {
    Flow *f = flows_[i];
    if (f->dst == host) {
      M_->updateRl(f->src, f->dst, 1);
      d_T += M_->demand(f->src, f->dst);
      n_R += 1;
    }
  }
  
  if ( d_T <= 1.0) 
    return 0;
  double e_s = 1.0/n_R;
  int flag;
  do {
    flag=0;
    n_R = 0;
    for(int i=0; i<flows_.size(); i++) {
      Flow *f = flows_[i];
      if ( f->dst == host && M_->rl(f->src, f->dst)) {
        if ( M_->demand(f->src, f->dst) <= e_s ) {
	  d_s += M_->demand(f->src, f->dst);
          M_->updateRl(f->src, f->dst, 0);	  
	  flag = 1;
	} else {
          n_R += 1;
	}
      }
    }
    e_s =  (1.0-d_s) / n_R;
  } while(flag); 

  for(int i=0; i<flows_.size(); i++) {
    Flow *f = flows_[i];
    if( f->dst == host && M_->rl(f->src, f->dst) ) {
      M_->updateDemand(f->src, f->dst, e_s);  
      M_->updateFlag(f->src, f->dst, 1);
      changed = 1;
    }
  }
  return changed;
}

void 
Hedera::updateRoute(Flow *f) {
  // Check if the flow already reserved a BW
  if(f->reserved)
    return;

  // Check the available BW of the old route
  int hops = f->links.size();
  if (hops <= 2)
    return;

  int tor1 = f->links[0]->dst;
  int hop  = f->links[1]->dst;
  int tor2 = f->links[hops-1]->src;

  double available_bw = unreservedBW(tor1, tor2, hop);
  if(available_bw >= M_->demand(f->src, f->dst)) {
    f->reserved = 1;
    return;
  }  
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

  // Search for a new route
  int HopId = search(f);

  // Insert the new route
  topo_->insertFlow(f, HopId);
}

int
Hedera::search(Flow *f) {
  int hopId, num_server = topo_->getNumServers();
  int tor1 = f->src / topo_->getNumSPR() + num_server;
  int tor2 = f->dst / topo_->getNumSPR() + num_server;
  for(int i=0; i<topo_->getNumTORs(); i++) {
    hopId = i + topo_->getNumServers();
    if(hopId == tor1)
      continue;	    
    double bw = unreservedBW(tor1, tor2, hopId);
    if(bw >= M_->demand(f->src, f->dst)) {
      f->reserved = 1;
      return hopId;
    }
  }
  do {
    hopId = rand() % topo_->getNumTORs() + num_server;
  } while (hopId == tor1);
  //logger.info("Random:%d, %d", hopId, tor1);
  return hopId;
}

double 
Hedera::unreservedBW(int tor1, int tor2, int hop) {                                            
  // Find the unreserved bandwidth of the given route
  Link *secondHop, *thirdHop; 
  double bw1 = 1.0, bw2 = 1.0;

  secondHop = topo_->findLink(tor1, hop);                                                                    
  thirdHop  = topo_->findLink(hop, tor2);
  if(secondHop!=NULL) {
    for(int i=0; i<secondHop->flows.size(); i++) {                                                           
      Flow *f = secondHop->flows[i];                                                                  
      if(f->reserved) {
        bw1 -= M_->demand(f->src, f->dst);
      }
    }
  } 
  if(thirdHop!=NULL) {
    for(int i=0; i<thirdHop->flows.size(); i++) {
      Flow *f = thirdHop->flows[i];
      if(f->reserved) {
        bw2 -= M_->demand(f->src, f->dst);
      }
    }
  }
  return util::min(bw1, bw2);
}

// demandMatrix
demandMatrix::demandMatrix(int size) {
  size_ = size;	
  M_ = new estDemand*[size];
  for(int i=0; i<size; i++) 
    M_[i] = new estDemand[size];
}

void
demandMatrix::clear() {
  for(int i=0; i<size_; i++) {
    for(int j=0; j<size_; j++) {
      updateDemand(i, j, 0);
      updateFlag(i, j, 0);
      updateRl(i, j, 0);  
    }
  }   
}


}
