// Milad Sharif
// Sep 2013
// Valiant load balancing similar to "VL2: A Scalable and Flexible Data Center Network"

#ifndef VL2_H
#define VL2_H

#include "./te.h"

namespace NSimulator {

class VL2 : public TE {
  public:
    VL2();
    ~VL2();
    void calW() {};
    string type() { return "vl2"; }
    Link *getLink(int src, int dst);
};

}
#endif
