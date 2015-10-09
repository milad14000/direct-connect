// Milad Sharif
// Oct 2013

#ifndef PS_H
#define PS_H

#include "./te.h"

namespace NSimulator {

class PS : public TE {
  public:
    PS(TDM* tdm, string option);
    void calW() {};
    string type() { return "packetSpray"; }
    Link *getLink(int src, int dst) { return NULL; }
    double getL(int i,int j, int k); 
  protected:
    int num_tor;	/* # of TORs */
    double *L;  /* Initial splits for direct-connection and 2-hop routes */
    void setL(int i, int j, int k, double l);
    void readOptW(int n);
};

}

#endif
