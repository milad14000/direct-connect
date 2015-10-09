#include "./simulator.h"
#include <cstddef>
#include <cstdio>

#include "./log.h"

namespace NSimulator {

Simulator::~Simulator() {
  while (!flowlist_fin.empty()) {
    delete flowlist_fin.back();
    flowlist_fin.pop_back();
  }
  while (!flowlist_torun.empty()) {
    delete flowlist_torun.back();
    flowlist_torun.pop_back();
  }
  delete gen, topo, te, tdm;
}

void
Simulator::set_FFUgenerator(double avgInt, double flowSize, TDM *tdm, int seed) {
  gen = new FFUgenerator(avgInt, flowSize, tdm, seed);
}

void
Simulator::run() {
  initRun();
  double interval = dfs->getInt();
  double sampleTime = dfs->getInt(), curTime = 0; /* FIXME */
  static int count=0;
  while (topo->getNumFlows() > 0 || flowlist_torun.size() > 0) {
    Flow *f = topo->calMMF(curTime);
    /*
    char *file_name = "topoLog";
    static struct Logger topoLog(LOG_DEBUG, file_name);
    vector<Link*>* list = topo->getLinkList();
    for(int i=0; i<list->size(); i++) {
      Link* l=(*list)[i];
      if(l->src < topo->getNumServers() || l->dst < topo->getNumServers())
        continue;
      //Link *l = topo->findLink();
      int n=l->flows.size();
      topoLog.debug("ts %f src %d dst %d num %d",l->src, l->dst, n);   
    } */
    if (flowlist_torun.size() == 0
        || (f != NULL && f->exp_fintime < flowlist_torun.front()->arrival_time)){
      //departure
      double nextTime = f->exp_fintime;
      if(nextTime < sampleTime) {
	curTime = nextTime;
        topo->removeflow(f);
	flowlist_fin.push_back(f);
      } else {
        dfs->sample();
	curTime = sampleTime;
	sampleTime += interval;
      }
    } else {
      //arrival
      double nextTime = flowlist_torun.front()->arrival_time;
      if(nextTime < sampleTime) {
	count ++;
	if(count % 5000 ==0)
          logger.info("%d flow(s) arrived ...", count);	
	curTime = nextTime;
	topo->route(flowlist_torun.front(), te);
        flowlist_torun.pop_front();
      } else {
	dfs->sample();
	curTime = sampleTime;
	sampleTime += interval;
      }
    }
  }
}

void
Simulator::initRun() {
  flowlist_fin.clear();
  for (unsigned int i = 0; i < flowlist_torun.size(); i++) {
    flowlist_torun[i]->lastEventTime = flowlist_torun[i]->arrival_time;
    flowlist_torun[i]->rate = 0;
    flowlist_torun[i]->size = flowlist_torun[i]->fsize;
  }
}

}


