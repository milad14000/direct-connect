#include "vl2.h"
#include "log.h"

#include <cstdlib>

namespace NSimulator {

VL2::VL2() {
}

VL2::~VL2() {
}

Link *
VL2::getLink(int src, int dst) {
  // Randomly selec a hop
  int TORId;
  do {
    TORId= rand() % topo->getNumTORs() + topo->getNumServers();
  } while (TORId==src);
  return topo->findLink(src, TORId);
}

}
