// A command line driver for the simulator

#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <cstddef>
#include <getopt.h>
#include <fstream>
#include <stdexcept>
#include "./topology.h"
#include "./direct.h"
#include "./bipartite.h"
#include "./log.h"
#include "./simulator.h"
#include "./tdm.h"
#include "./b4.h"
#include "./vl2.h"
#include "./dfs.h"
#include "./ps.h"
#include "./bllb.h"
#include "./hedera.h"

#define MAX_LINE_LEN 32

using namespace std;

int num_rack = 32;			/* Number of racks */
int server_per_rack = num_rack -1; 	/* Number of servers per rack */
int oversub= 1; 			/* Oversubscription */
string topology= "bipartite";	    	/* Connection type
					   - "direct"    : Direct connect topology
					   - "bipartite" : connection trough spine switches */ 
double load= 0.1;			/* Load observed at the server (not core) */
string demand_matrix="gravity";		/*  Traffic Matirx deman
					    - "file"
					    - "gravity"
					    - "uniform"
					    - "permutation" */

string traffic_eng="vl2";		/* Traffic engineering
					   - "vl2"  : Valiant load balancing
					   - "B4"  
					   - "IEWF" : Iterative water-filling
					   - "ECMP" : Equal-cost multipath
					   - "PS"   : packet spraying */

string option="optimal";		/* Option for traffic engineering
					   IEWF: options for initial splits
					   	- "exponential" : 1/10^(# of hops)
						- "equal"
						- "dahu" start with direct connection */

string scheduler="FLLB";		/* Dynamic flow scheduler
					   - "NONE"	  
					   - "FLLB"	: Flow-level w/ random selection
					   - "CA-FLLB"	: Scheduler w/ least congested selection */

double interval = 100e-6; 
int threshold   = 1;
int num_flow = 20000;			/* number of flows to run */
string flow_distribution= "PFU";	/* Flow distribution
					  - "PFU":  poisson
					  - "FFU": uniform */
//double flow_size= 1e6 * 8;		/* Average flow size : 1 MByte */
double flow_size = 1.1e5 * 8;		/* Average flow size : ~100 KBytes based on estimated CDF */
string output_file= "flow.log";		/* Output file name */

int logging_level = LOG_DEBUG;		/* Logging level */

const struct option long_options[] = {
	{ "help",	0, 0, 'h' },
	{ "load",	1, 0, 'l' },
	{ "te"  ,	1, 0, 't' },
	{ "scheduler",	1, 0, 's' },
	{ "demand",	1, 0, 'd' },	
	{ 0,		0, 0,  0  }
};


int main(int argc, char** argv) {

  int c;
  while ((c = getopt_long(argc, argv, "hl:o:t:p:s:T:d:i:n:f:", long_options, NULL)) != EOF) {
    fprintf(stderr,"%c: %s\n",c, optarg);
    switch(c) {
      case 'l':
	load = atof(optarg)/100.0;
	break;
      case 'o':
        output_file.assign(optarg);
        break;
      case 't':
        traffic_eng.assign(optarg);
        break;	
      case 'i':
        topology.assign(optarg);
	break;
      case 'p':
	option.assign(optarg);
	break;
      case 's':
	scheduler.assign(optarg);
	break;
      case 'd':
	demand_matrix.assign(optarg);
	break;
      case 'T':	
	threshold = atoi(optarg);
	break;
      case 'n':
	num_rack = atoi(optarg);
	server_per_rack = num_rack-1;
	break;
      case 'f':
        num_flow = atoi(optarg);
        break;	
      case 'h':
	fprintf(stderr, "...");
	break;	
    }
  }

  using namespace NSimulator;

  logger.set_level(logging_level);

  NSimulator::Simulator sim;
  
  
 
  // Topology
  logger.info("*** Creating %s topology with %d TOR(s)", topology.c_str(), num_rack);
  if (topology == "direct") {
    sim.topo = new DirectConnect;
    sim.topo->buildTopology(num_rack, server_per_rack, oversub);
  } else if (topology == "bipartite") {
    sim.topo = new Bipartite;
    sim.topo->buildTopology(num_rack, server_per_rack, oversub);
  } else {
    logger.error("*** Error: Unknown topology");
  }

  // Demand
  sim.tdm = new TDM(load, num_rack*server_per_rack, num_rack,demand_matrix);  

  // Traffic Engineering	  
  if (topology == "direct") {
     logger.info("Traffic engineer: %s", traffic_eng.c_str());
    if(traffic_eng == "vl2")
      sim.te = new VL2; 
    else if(traffic_eng == "PS")
      sim.te = new PS(sim.tdm, option);	    
    else if(traffic_eng == "B4")
      sim.te = new B4(sim.tdm);
    else if(traffic_eng == "ECMP") 
      sim.te = NULL;
    else
      logger.error("Undefined traffic engineering");
  
    if (sim.te!=NULL) {    
      sim.te->topo = new DirectConnect;
      sim.te->topo->buildTopology(num_rack, server_per_rack, oversub);
      sim.te->calW();
    }
  } 
 
  // Flow Scheduling
  logger.info("Flow scheduler: %s with threshold:%d", scheduler.c_str(), threshold);
  if (scheduler == "FLLB") {
    sim.dfs = new FLLB(sim.topo, interval, threshold);
  } else if (scheduler == "CA_FLLB") {
    sim.dfs = new CA_FLLB(sim.topo, interval, threshold);
  } else if (scheduler == "BLLB") {
    sim.dfs = new BLLB(sim.topo, interval, threshold);
  } else if (scheduler == "NONE") {
    sim.dfs = new FLLB(sim.topo, 1e20, threshold); /* Scheduler with very high sampling interval */
  } else if (scheduler == "Hedera") {
    sim.dfs = new Hedera(sim.topo, 100*interval, threshold); /*FIXME*/
  } else {
    logger.error("Undefined flow scheduler");
  }
  
  // Load
  logger.info("*** Generating %.2f load with %s dist. based on \"%s\"", load, flow_distribution.c_str(), demand_matrix.c_str());
  double lambda = (load * 1e10) / (flow_size) * (sim.topo->getNumServers());
  if (flow_distribution == "PFU") {
    double avgInt = 1.0 / lambda;
    sim.gen = new PFUgenerator(avgInt,
                                   flow_size,
                                   sim.tdm,
                                   301);
  } else
  if (flow_distribution == "FFU") {
    double avgInt = 1.0 / lambda;
    sim.gen = new FFUgenerator(avgInt,
                                   flow_size,
                                   sim.tdm,
                                   301);
  } else {
    logger.error("*** Error: Unknown distribution");
  }
  sim.gen->loadCDF("CDF");
  for (int i = 0; i < num_flow; i++) {
    NSimulator::Flow *f = sim.gen->getNextFlow();
    sim.flowlist_torun.push_back(f);
  }
    
  logger.info("*** Running the simulation");
  sim.run();
  
  logger.info("Done.");
  FILE *of = fopen(output_file.c_str(), "w");

  for (unsigned int i = 0; i < sim.flowlist_fin.size(); i++) {
    Flow *f = sim.flowlist_fin[i];
    int hop = f->links.size();
    fprintf(of, "id %u start %f end %f size %f rate %f fct %f hop %d\n",
	    f->flowId,
	    f->arrival_time,
            f->exp_fintime,
            f->fsize,
            f->fsize / (f->exp_fintime - f->arrival_time),
            (f->exp_fintime - f->arrival_time)*1000,			// FCT in (ms)
	    hop);
  }
  fclose(of);
  return 0;
}

