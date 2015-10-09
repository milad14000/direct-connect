#include "./ps.h"
#include "log.h"
#include "util.h"
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <cmath>

namespace NSimulator {


PS::PS(TDM *tdm, string option) {
  int n=tdm->demands.size();
  logger.info("*** Creating flow groups ... ");

  // Creating flow groups based on the traffic demand matrix
  for(int i=0; i<n; i++) {	     
    struct demand *temp = &(tdm->demands[i]);
    FG f(temp->src+tdm->getNumServers(), temp->dst+tdm->getNumServers(), temp->d);
    fg.push_back(f);
  }


  // Set initial splits
  num_tor = tdm->getNumTORs();
  int size = pow(num_tor,3);
  L = new double[size];
  if(option!="optimal") {
    double L1, L2, L_tot=1;
    if(option == "exponential") {                       /* Exponential len decay, proportional to 1/10^(# of hops) */
      L_tot = (num_tor + 8.0)/100;
      L1 = 0.1/L_tot;
      L2 = 0.01/L_tot;
    } else if(option == "equal") {                      /* Equal splits */
      L_tot = num_tor-1;
      L1 = 1.0/L_tot;
      L2 = 1.0/L_tot;
    } else if(option == "perm_optimal") {
      L_tot = num_tor;
      L1 = 2.0/L_tot;
      L2 = 1.0/L_tot;
    } else {
      logger.error("Unrecognize option for initial splits of water-filling");
    }

    for (int i=0; i<n; i++) {
      struct demand *temp = &(tdm->demands[i]);
      for(int tor=0; tor<num_tor; tor++) {
      	if( tor == temp->dst ) {
          setL(temp->src, tor, temp->dst, L1);   
	} else {
	  setL(temp->src, tor, temp->dst, L2);	
	}
	//L[temp->src*num_tor^2 + tor*num_tor + temp->dst] = L2;
      }
    }
  } else {                                              /* Read optimal weights from a file */
    readOptW(num_tor);
  }
}


void
PS::setL(int i, int j, int k, double l) {
  int index = i*pow(num_tor,2) + j*num_tor + k;
  L[index]=l;
}

double
PS::getL(int i, int j, int k) {
  int index = i*pow(num_tor,2) + j*num_tor + k;
  return L[index];
}


void
PS::readOptW(int num_tor) {
  string line;
  ifstream infile("optW");

  if (infile == NULL)  {
    logger.error("Could not find the \"optW\" file");
    return;
  }

  int l = 0;
  while (getline(infile, line))  {
    try {
      if (line.at(0) == '#')
        continue;

      istringstream iss(line);

      struct demand temp;
      double l;
      iss >> temp.src >> temp.dst;

      for(int k=0; k<num_tor; k++) {
        iss >> l;
        setL(temp.src, k, temp.dst, l);
      }

    } catch(const out_of_range& oor) {}
    l++;
  }
}



}
