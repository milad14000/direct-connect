
#include <iostream>
#include <fstream>
#include <sstream> 
#include <stdexcept>
#include <algorithm>
#include <cassert>
#include "./tdm.h"

#include "./util.h"
#include "./log.h"

namespace NSimulator {


TDM::TDM(double load, int num_server, int num_rack, string in_demand) {
  
  num_server_ = num_server;
  num_rack_ = num_rack;
  
  srand (time(NULL));
  // Uniform demand
  if (in_demand == "uniform") {                                                                 
    double normDemand = load/num_rack;                                                                             
      for(int i=0; i<num_rack; i++) {                                                         
        for(int j=0; j<num_rack; j++) {                                                       
	  struct demand temp;
	  temp.src = i;
	  temp.dst = j;
	  temp.d   = normDemand; 
	  demands.push_back(temp); 					            
	}                                                                                               
      }                                                                                                 
  // Gravity model
  } else if (in_demand == "gravity") {
    int check=1;
    double min_rate = util::max(0, 2*load-1);
    double max_rate = util::min(2*load, 1); 
    vector<double> r,c;
    for(int i=0; i<num_rack; i++) {
      r.push_back( min_rate + (max_rate-min_rate)*(float)rand()/(float)RAND_MAX );
      c.push_back( min_rate + (max_rate-min_rate)*(float)rand()/(float)RAND_MAX );
    }

    double R = util::sum(&r);
    double C = util::sum(&c);

    for(int i=0; i<num_rack-1; i++) {
      r[i] *= (num_rack*load)/R;
      c[i] *= (num_rack*load)/C;
      logger.info("r(%d)=%.2f, c(%d)=%.2f", i, r[i], i, c[i]);
      assert(c[i]<=1 && r[i]<=1);
    }

    double max_ij = 0;
    for(int i=0; i<num_rack; i++) {
    	for(int j=0; j<num_rack; j++) {
	  if(i!=j && max_ij < r[i]+c[j])
	       max_ij = r[i]+c[j];
	 }
    }
    logger.info("Max ri + cj: %.2f", max_ij);

    double V_c=0, V_r= 0;
    for (int i=0; i<num_rack; i++) {
        V_r += (r[i]-load)*(r[i]-load)/num_rack;
        V_c += (c[i]-load)*(c[i]-load)/num_rack;
    }
    logger.info("Variance C: %f , Variance R: %f", V_c, V_r);

    for(int i=0; i<num_rack; i++) {
      for(int j=0; j<num_rack; j++) {
        struct demand temp;
	temp.src = i;
	temp.dst = j;
	if (i!=j) {
		temp.d   = r[i]*c[j]/load/num_rack;
        } else { 
		temp.d = 0;
	}
	demands.push_back(temp);	
      }
    }
        
  // Read from file
  } else if (in_demand == "pattern") {                                                     
    parseDemand("pattern");
    for(int i=0; i<demands.size(); i++) {
      demands[i].d *= load;
      assert(demands[i].d <= 1);
    }
  } else if(in_demand =="file") {
    parseDemand("file"); 
  // Permutation
  } else if (in_demand == "permutation") {
    double normDemand = 1*load;                                                                                
    vector<int> randPerm;        
    for (int i=0; i<num_rack; i++)
      randPerm.push_back(i);

    random_shuffle( randPerm.begin(), randPerm.end());
    
    for(int i=0; i<num_rack; i++) {                                                         
      struct demand temp;
      temp.src = i;
      temp.dst = randPerm[i];
      temp.d   = normDemand;      
      demands.push_back(temp);                                                                                
    }
  } else {
    logger.error("Undefined traffice model");
  }

  // Calculate the variance
  double V = 0;
  for (int i=0; i<demands.size(); i++)
    V += (demands[i].d-load/num_rack)*(demands[i].d-load/num_rack)/num_rack/num_rack;
  logger.info("Variance : %f", V);

  char *file_name = "pattern";
  static struct Logger tdmLog(LOG_INFO, file_name);
  tdmLog.info("# %s", in_demand.c_str());
  tdmLog.info("# src dst demand");
  for(int i=0; i<demands.size(); i++) 	  
    tdmLog.info("%d %d %.3f", demands[i].src, demands[i].dst, demands[i].d/load); 
}

TDM::~TDM() {
}

void
TDM::parseDemand(string in_demand) {

  string line;
  ifstream infile(in_demand.c_str());

  if (infile == NULL)  {
    logger.error("Could not find the tdm file");
    return;
  }

  while (getline(infile, line))  {
    try {
      if (line.at(0) == '#')
        continue;
      
      istringstream iss(line);
      
      struct demand temp;
      iss >> temp.src >> temp.dst;
      iss >> temp.d;
   
      //assert(temp.d <= 1);

      if(temp.src<0 || temp.src>num_rack_ || 
		      temp.dst<0 || temp.dst>num_rack_ ) {
        logger.error("server Id in traffic demand file exceeds the specified # of servers");
      }

      demands.push_back(temp);
    } catch(const out_of_range& oor) {}
  }
}



}
