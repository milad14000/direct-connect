#include "./generator.h"
#include <iostream>
#include "math.h"
#include "log.h"
#include <stdlib.h>
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <cmath>

namespace NSimulator {

double Generator::lastArrival = 0;

//Generate random (src,dst) based on the traffic demand matrix
void Generator::pairGen(int *src, int *dst) {  /*FIXME do not work for general traffic demand */
  int server_per_rack = tdm_->getNumServers()/tdm_->getNumTORs();
  int i, count = tdm_->demands.size();
  double r, tot_d=0;
  for(int j=0; j<count; j++) 
    tot_d+=tdm_->demands[j].d;

  r =  tot_d * (double)rand() / (double)RAND_MAX;
  for(i=0; i<count; i++) {
    r -= tdm_->demands[i].d;
    if(r<0)
      break;	    
  }
  // Uniform demand
  struct demand *temp = &(tdm_->demands[i]); 
  do {
    *src = temp->src * server_per_rack + rand() % server_per_rack;
    *dst = temp->dst * server_per_rack + rand() % server_per_rack;
  } while(*src == *dst);
}

// Return a flow size based on a given empirical distribution
double Generator::flowSize() {
  double r = (double)rand() / (double)RAND_MAX;
  return inverseCDF(r);
}

double Generator::inverseCDF(double y) {
  string interp = "linear";

  if(interp == "linear") {
    for(int i=0; i<cdf_.size()-1; i++) {
      if( cdf_[i].value_ <= y && cdf_[i+1].value_ > y) {
        double s = (cdf_[i+1].nbytes_ - cdf_[i].nbytes_)/(cdf_[i+1].value_ - cdf_[i].value_); 
        return 8*pow(10,s*( y - cdf_[i].value_) + cdf_[i].nbytes_);				// Flow size in (bit)
      }
    }
  } else if (interp == "zero-order") {
    for(int i=0; i<cdf_.size()-1; i++) {
      if( cdf_[i].value_ <= y && cdf_[i+1].value_ > y) {
        return 8*pow(10, cdf_[i+1].nbytes_);	      
      }
    }
  } else {
    logger.error( "Unrecognized interpolation");
  }
  logger.error(" Error in inverseCDF");
}

// Load the empirical CDF from a file
void
Generator::loadCDF(string empCDF) {
  string line;
  ifstream infile(empCDF.c_str());

  if (infile == NULL)  {
    logger.error("Could not find the CDF");
    return;
  }

  while (getline(infile, line))  {
    try {
      if (line.at(0) == '#')
        continue;
      istringstream iss(line);

      double nbytes, value;
      iss >> nbytes >> value;
      cdf temp(nbytes, value);
      cdf_.push_back(temp);
    } catch(const out_of_range& oor) {}
  }
}




FFUgenerator::FFUgenerator(double avgInt, double flowSize, TDM *tdm, int seed)
    {
      flowSize_ = flowSize;
      avgInt_ = avgInt;
      tdm_ = tdm;
      serverRange_ = tdm->getNumServers();

}

//deterministic arrivals flow generator (testing purposes only)
Flow*
FFUgenerator::getNextFlow() {
  //static unsigned int id = 0;
  Flow* f = new Flow();

  int src,dst;
  pairGen(&src, &dst);

  //f->flowId = id++;
  f->src = src;
  f->dst = dst;
  
  f->reserved = 0 ;
  f->fsize = flowSize_;
  f->links.clear();
  f->arrival_time = Generator::lastArrival + avgInt_;
  Generator::lastArrival += avgInt_;
  return f;
}

PFUgenerator::PFUgenerator(double avgInt, double flowSize, TDM *tdm, int seed)
    {	    
      flowSize_ = flowSize;
      avgInt_ = avgInt;
      tdm_ = tdm;
      serverRange_ = tdm_->getNumServers();
}

//poisson arrivals flow generator
Flow*
PFUgenerator::getNextFlow() {
  static unsigned int id=0;
  Flow *f = new Flow();
  
  int src,dst;
  pairGen(&src, &dst);

  f->flowId = id++;
  f->src = src;
  f->dst = dst;

  f->reserved = 0;
  //f->fsize = flowSize_;
  //f->fsize = flowSize();
  f->fsize = 30*8; 
  f->links.clear();
  double intval = -log(0.001 + (float)rand()/((float)RAND_MAX/0.999)) * avgInt_;
  f->arrival_time = Generator::lastArrival + intval;
  Generator::lastArrival += intval;
  return f;
}



}
