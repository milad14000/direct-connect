#include "./direct.h"
#include <stdio.h>
#include "./te.h"
#include "./ps.h"
#include "./log.h"
#include <typeinfo>

namespace NSimulator {

DirectConnect::DirectConnect() {
  init=false;
}

DirectConnect::~DirectConnect() {
}

void
DirectConnect::buildTopology(int num_rack, int server_per_rack, int oversub) {
  num_rack_ = num_rack;
  server_per_rack_ = server_per_rack;
  oversub_ = oversub;
  //Init Server nodes;
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

  //Connect ToRs
  double cap = double(server_per_rack) * 1e10 / oversub / (num_rack - 1);
  for (int i = 0; i < num_rack; i++) {
    for (int j = 0; j < i; j++) {
      Link *l1 = new Link();
      l1->src = i + num_server;
      l1->dst = j + num_server;
      l1->cap = cap;
      nodelist[i + num_server]->links.push_back(l1);
      linklist.push_back(l1);

      Link *l2 = new Link();
      l2->src = j + num_server;
      l2->dst = i + num_server;
      l2->cap = cap;
      nodelist[j + num_server]->links.push_back(l2);
      linklist.push_back(l2);
    }
  }
  init = true;
}

int 
DirectConnect::route(Flow *f, TE *te) {
  if(f->src >= num_server || f->dst >= num_server)
    logger.error("src or dst exceeds the number of servers");
  
  int tor1,tor2, TORId;
  tor1 = f->src/server_per_rack_ + num_server;
  tor2 = f->dst/server_per_rack_ + num_server;
  
  if(tor1 == tor2) {
    insertFlow(f,-1);	 // -1 for intra-rack traffics 
    flowlist.push_back(f); 
  } else if(te == NULL) {
    insertFlow(f, tor2);
    flowlist.push_back(f);
  } else if(te->type() != "packetSpray") {   
    Link *hop = te->getLink(tor1, tor2);
    insertFlow(f, hop->dst);
    flowlist.push_back(f);
  } else {
    	  
    for(int i=0; i<num_rack_; i++) {  
      TORId = i+num_server;
      if (TORId == tor1) {
        continue;
      }
      Flow *subFlow = new Flow();
      subFlow->src          = f->src;
      subFlow->dst          = f->dst;
      subFlow->fsize        = f->fsize * ((PS *)te)->getL(tor1-num_server, i, tor2-num_server);
      subFlow->flowId 	    = f->flowId;
      subFlow->arrival_time = f->arrival_time;
      subFlow->size         = subFlow->fsize;
      subFlow->rate         = 0;
      subFlow->lastEventTime= f->arrival_time; 
      insertFlow(subFlow, TORId);
      flowlist.push_back(subFlow);
    }
  }

  return 0; 
}


void 
DirectConnect::insertFlow(Flow *f, int hop) {
  int tor1 = f->src/server_per_rack_ + num_server;
  int tor2 = f->dst/server_per_rack_ + num_server;

  f->links.clear();
  Link *firstHop  = findLink(f->src, tor1);
  f->links.push_back(firstHop);
  firstHop->flows.push_back(f);

  if(hop != -1) {
    Link *secondHop = findLink(tor1, hop);
    f->links.push_back(secondHop);
    secondHop->flows.push_back(f);

    Link *thirdHop  = findLink(hop, tor2); 
    if(thirdHop!=NULL) {
      f->links.push_back(thirdHop);
      thirdHop->flows.push_back(f);
    }
  }
  Link *lastHop   = findLink(tor2, f->dst); 
  f->links.push_back(lastHop);
  lastHop->flows.push_back(f);
}


}
