#include "./topology.h"
#include <cstddef>
#include <stdio.h>
#include "log.h"

namespace NSimulator {

Topology::Topology() {
}

Topology::~Topology() {
  if (init) {
    while (!linklist.empty()) {
      delete linklist.back();
      linklist.pop_back();
    }
    while (!nodelist.empty()) {
      delete nodelist.back();
      nodelist.pop_back();
    }
    while (!flowlist.empty()) {
      delete flowlist.back();
      flowlist.pop_back();
    }
  }
}

void
Topology::printTopology() {
  if (!init) {
    printf("Topology not init yet");
  }
  for (unsigned int i = 0; i < nodelist.size(); i++) {
    printf("node %d: ", nodelist[i]->id);
    switch (nodelist[i]->type) {
      case Node::Server:
        printf("server\n");
        break;
      case Node::Switch:
        printf("switch\n");
        break;
    }
    for (unsigned int j = 0; j < nodelist[i]->links.size(); j++) {
      printf("%d(%.3g) ", nodelist[i]->links[j]->dst,
                          nodelist[i]->links[j]->cap);
    }
    printf("\n");
  }
}

Link*
Topology::findLink(int src, int dst) {
  if (src < 0 || src >= nodelist.size()
      || dst < 0 || dst >= nodelist.size()) {
    return NULL;
  }
  Node *cur = nodelist[src];
  for (unsigned int i = 0; i < cur->links.size(); i++) {
    if (cur->links[i]->dst == dst)
      return cur->links[i];
  }
  return NULL;
}

void
Topology::removeflow(Flow *f) {
  for (unsigned int i = 0; i < f->links.size(); i++) {
    Link *l = f->links[i];
    for (unsigned int j = 0; j < l->flows.size(); j++) {
      if (f == l->flows[j]) {
        l->flows.erase(l->flows.begin() + j);
        break;
      }
    }
  }
  for (unsigned int i = 0; i < flowlist.size(); i++) {
    if (flowlist[i] == f) {
      flowlist.erase(flowlist.begin() + i);
      return;
    }
  }
}

Flow*
Topology::calMMF(double curTime) {
  if (flowlist.size() == 0)
    return NULL;

  double nextExp = 1e20;
  Flow* nextFin = NULL;
  //init capacity
  int linknum = 0;
  for (unsigned int i = 0; i < linklist.size(); i++) {
    Link *l = linklist[i];
    l->acap = l->cap;
    l->aflow = l->flows.size();
    if (l->aflow > 0)
      linknum++;
  }
  //init flow
  for (unsigned int i = 0; i < flowlist.size(); i++) {
    Flow *f = flowlist[i];
    f->satisfied = false;
  }
  int flownum = flowlist.size();

  while (flownum > 0 && linknum > 0) {
    //find the minimum fair share
    double minrate = 1e20;
    Link *minL;
    for (unsigned int i = 0; i < linklist.size(); i++) {
      if (linklist[i]->aflow > 0 && linklist[i]->acap > 0 &&
          linklist[i]->acap / linklist[i]->aflow < minrate) {
        minL = linklist[i];
        minrate = linklist[i]->acap / linklist[i]->aflow;
      }
    }
    //logger.debug("@%f minrate: %f", curTime, minrate/10e9);

    for (unsigned int i = 0; i < minL->flows.size(); i++) {
      if (!minL->flows[i]->satisfied) {
	Flow *f = minL->flows[i];
        f->satisfied = true;
        f->size -= (curTime - f->lastEventTime) * f->rate;
        flownum--;
        f->lastEventTime = curTime;
        f->rate = minrate;
        f->exp_fintime = curTime + f->size / f->rate;
        if (f->exp_fintime < nextExp) {
          nextExp = f->exp_fintime;
          nextFin = f;
        }

        for (unsigned int j = 0; j < f->links.size(); j++) {
          f->links[j]->acap -= f->rate;
          f->links[j]->aflow --;
          if (f->links[j]->aflow == 0)
            linknum--;
        }
      }
    }
  }
  return nextFin;
}

}
