#include "./bipartite.h"
#include "./te.h"

#include <stdlib.h>
#include <stdio.h>

namespace NSimulator {

Bipartite::Bipartite()
  {
  init = false;
}

Bipartite::~Bipartite() {

}

void
Bipartite::buildTopology(int num_rack, int server_per_rack, int oversub) {
  num_rack_ = num_rack;
  server_per_rack_ = server_per_rack;
  oversub_ = oversub;

  //Init Server node;
  num_server = num_rack * server_per_rack;
  for (int i = 0; i < num_server; i++) {
    Node *n = new Node();
    n->id = i;
    n->type = Node::Server;
    nodelist.push_back(n);
  }

  //Init ToR;
  for (int i = 0; i < num_rack; i++) {
    Node *n = new Node();
    n->id = i + num_server;
    n->type = Node::Switch;
    nodelist.push_back(n);
  }

  //Init Spine;
  num_spine = server_per_rack / oversub;
  for (int i = 0; i < num_spine; i++) {
    Node *n = new Node();
    n->id = i + num_server + num_rack;
    n->type = Node::Switch;
    nodelist.push_back(n);
  }

  //Connect servers to ToR
  for (int i = 0; i < num_server; i++) {
    int tor = i / server_per_rack + num_server;
    Link *l1 = new Link();
    l1->src = i;
    l1->dst = tor;
    l1->cap = 1e10;
    nodelist[i]->links.push_back(l1);
    linklist.push_back(l1);

    Link *l2 = new Link();
    l2->src = tor;
    l2->dst = i;
    l2->cap = 1e10;
    nodelist[tor]->links.push_back(l2);
    linklist.push_back(l2);
  }

  //Connect ToRs to Spines
  for (int i = 0; i < num_rack; i++)
    for (int j = 0; j < num_spine; j++) {
      int torid = i + num_server;
      int spineid = j + num_server + num_rack;
      Link *l1 = new Link();
      l1->src = torid;
      l1->dst = spineid;
      l1->cap = 1e10;
      nodelist[torid]->links.push_back(l1);
      linklist.push_back(l1);

      Link *l2 = new Link();
      l2->src = spineid;
      l2->dst = torid;
      l2->cap = 1e10;
      nodelist[spineid]->links.push_back(l2);
      linklist.push_back(l2);

    }
  init = true;
}

int
Bipartite::route(Flow *f, TE *te) {
//random shortest path
  int PacketSpraying = 0;
  if (f->src >= num_server || f->dst >= num_server) {
    printf("error: flow shouldn't be allowed\n");
    return -1;
  }
  int tor1 = f->src/server_per_rack_ + num_server;
  int tor2 = f->dst/server_per_rack_ + num_server;
  int res = 2;
  
  if (tor1 != tor2) {
    if(PacketSpraying == 0 ) {	  
      int spineid = ( rand() % num_spine) + num_server + num_rack_;
      insertFlow(f, spineid);
      flowlist.push_back(f);
      res = 4;
    } else {
      for(int i=0; i<num_spine; i++) {
        int SpineId = i + num_server + num_rack_;
	Flow *subFlow = new Flow();                                                                     
	subFlow->src          = f->src;                                                                 
	subFlow->dst          = f->dst;                                                                 
	subFlow->fsize        = f->fsize / num_spine;       
	subFlow->flowId       = f->flowId;                                                              
	subFlow->arrival_time = f->arrival_time;                                                        
	subFlow->size         = subFlow->fsize;                                                         
	subFlow->rate         = 0;
	subFlow->lastEventTime= f->arrival_time; 
	insertFlow(subFlow, SpineId);                                                                     
	flowlist.push_back(subFlow);  
      }
    }
  } else {
    insertFlow(f, -1);
    flowlist.push_back(f);
  }
  return res;
}

void
Bipartite::insertFlow(Flow *f, int spineId) {
  int tor1 = f->src/server_per_rack_ + num_server;
  int tor2 = f->dst/server_per_rack_ + num_server;

  f->links.clear();
  Link *firstHop = findLink(f->src, tor1);
  f->links.push_back(firstHop);
  firstHop->flows.push_back(f);
  if(spineId != -1) {
    Link *secondHop = findLink(tor1, spineId);
    f->links.push_back(secondHop);
    secondHop->flows.push_back(f);

    Link *thirdHop = findLink(spineId, tor2);
    f->links.push_back(thirdHop);
    thirdHop->flows.push_back(f);
  }
  Link* lastHop = findLink(tor2, f->dst);
  f->links.push_back(lastHop);
  lastHop->flows.push_back(f);
}




}
