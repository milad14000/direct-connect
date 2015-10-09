#include "./topology.h"

namespace NSimulator {

class TE;

// All ToR racks connect in a full-mesh
class DirectConnect : public Topology {
 public:
  DirectConnect();
  ~DirectConnect();
  virtual void buildTopology(int num_rack, int server_per_rack, int oversub);
  virtual int route(Flow *f, TE *te);
  void insertFlow(Flow *f, int hop);
};

}
