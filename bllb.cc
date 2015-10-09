#include <cstdlib>
#include <cstdio>
#include <cassert>

#include "./bllb.h"
#include "./util.h"
#include "./log.h"

namespace NSimulator {

BLLB::BLLB(Topology *topo, double interval, int threshold) :
       DFS(topo, interval, threshold) {
  // Initialize the weigths
  int N = topo_->getNumTORs();		
  W = new double**[N];
  for(int x=0; x<N; x++) {
    W[x] = new double*[N];
    for (int y=0; y<N; y++) {
      W[x][y] = new double[N];
      for (int z=0; z<N; z++) 
        W[x][y][z] = 0;
      
      if(x!=y)
      	W[x][y][y] = 1;  // Only use direct link at the begining 
    }
  }

  // Initialize C
  C = new int*[N];
  for(int x=0; x<N; x++) {
    C[x] = new int[N];
    for(int y=0; y<N; y++)
      C[x][y]=0;	    
  }

  // Initialize the thresholds
  threshold1_ = 2;
  threshold2_ = 4;
}

BLLB::~BLLB() {
  int N=topo_->getNumTORs();
  for(int x=0; x<N; x++) {
    for(int y=0; y<N; y++)
      delete [] W[x][y];
    
    delete [] W[x];
  }
  delete [] W;

}

void
BLLB::sample() {
  updateW();
  vector<Flow*> *flows = topo_->getFlowList();
  for(int i=0; i<flows->size(); i++) {
    Flow *f = flows->at(i);
    updateRoute(f);
  }
}


void
BLLB::updateW() {
  logger.debug("Updating ...");
  int num_tor = topo_->getNumTORs();
  int num_server = topo_->getNumServers();
  for (int i=0; i<num_tor; i++) {	// Src TOR
    for(int j=0; j<num_tor; j++) {	// Dst TOR
      if(j==i)
        continue;	      
      // Check the direct-link's congestion 
      Link *secondHop=topo_->findLink(i+num_server, j+num_server);
      if(secondHop->flows.size() > threshold2_) { 	// Congested
	logger.debug("%d->%d Congested: %d flow(s)", i, j, secondHop->flows.size());      
        C[i][j] = 1;
        // Check if i is transiting any flows
        logger.debug("Transiting %d", checkW(i,j));
	if ( checkW(i,j) ) {
          // Do nothing and wait for other TORs to reduce the traffic  
        } else {  // Explore new routes
	  explore(i,j);
	}
      	
      } else if(secondHop->flows.size() < threshold1_) { // Not congested
        C[i][j] = 0;
      }

      // Check the 2-hop routes and adjust the weights
      adjust(i, j);
    }
  }
  printW(3,0);
}

void
BLLB::updateRoute(Flow *f) {
  Link *oldHop;
  // Find the old route
  int hops = f->links.size();                                                                         
  if (hops == 2) 
    return;                                                                                           
  else
    oldHop = f->links[1];
      
  // Select a new route                                                                               
  int server_per_rack, tor1, tor2, TORId;                                                                    
  int src=f->src, dst=f->dst;                                                                         
  server_per_rack = topo_->getNumServers() / topo_->getNumTORs();                                     
  tor1 = src / server_per_rack + topo_->getNumServers();
  tor2 = dst / server_per_rack + topo_->getNumServers();                                              

  TORId = selectHop(tor1, tor2);
  if (TORId == oldHop->dst)
    return;

  logger.debug("%d,%d,%d", tor1,TORId, tor2);
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

  // Add the new route
  topo_->insertFlow(f, TORId);
}

int
BLLB::selectHop(int tor1, int tor2){
  double r;
  int i, N = topo_->getNumTORs();
  int num_server = topo_->getNumServers();
  r = (double)rand() / (double)RAND_MAX;
  for (i=0; i<N; i++) {
    r -= W[tor1-num_server][i][tor2-num_server];
    if (r<0)
      break;
  }
  logger.debug("%d->%d: %f",tor1, tor2, r);
  return i+num_server;
}

int 
BLLB::checkW(int i, int j) {
  int num_tor=topo_->getNumTORs();
  double sum=0;
  for (int x=0; x<num_tor; x++) 
    sum += W[x][i][j];
  if ( sum > 0 )
    return 1;
  sum=0;
  for(int z=0; z<num_tor; z++) {
    if(z==j)
      continue;	    
    sum += W[i][j][z];
  }
  if ( sum >0 )
    return 1;
  return 0;  
}

void
BLLB::explore(int i, int j) {
  double rate=0.1; /* FIXME */	
  int num_tor = topo_->getNumTORs();
  int available_routes = 0;

  assert(C[i][j]==1);
  for (int k=0; k<num_tor; k++) {
    if(C[i][k]==0 && C[k][j]==0 && k!=i)
      available_routes++;
  }
  if(available_routes == 0)
    return;
  
  double w = W[i][j][j]*rate;
  W[i][j][j] -= w;
  for (int k=0; k<num_tor; k++) {
    if(C[i][k]==0 && C[k][j]==0 && k!=i)
      W[i][k][j] += w/available_routes;
  }
}

void 
BLLB::adjust(int i, int j) {
  double rate=0.1;	/* FIXME */
  double sum=0;
  int num_tor = topo_->getNumTORs();
  int available_routes = 0;
  for(int k=0; k<num_tor; k++) {
    if(k==i || k==j)
      continue;
    if(C[i][k]==1 || C[k][j]==1) {
      sum += W[i][k][j]*rate; 
      W[i][k][j] = W[i][k][j]*(1-rate);
    } else {
      available_routes++;
    }

  }
  for(int k=0; k<num_tor; k++) {
    if(k==i || k==j || C[i][k]==1 || C[k][j]==1)
      continue;
    W[i][k][j] += sum/available_routes;   
	    
  }
}

void
BLLB::printW(int i, int j) {
  int num_tor = topo_->getNumTORs();
  logger.debug("Weights fot %d->%d", i, j);
  for(int k=0; k<num_tor; k++) {
    if(k==i)
      continue;
    logger.debug("W(%d):%.3f", k, W[i][k][j]);
  }
}

}
