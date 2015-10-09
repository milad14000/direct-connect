#ifndef TDM_H
#define TDM_H


#include <vector>
#include <string>


using namespace std;

namespace NSimulator {

struct demand {
  int src;
  int dst;
  double d;
};

// Traffic Demand Matrix
class TDM {
  public:
    TDM(double load, int num_server, int num_rack, string in_file);
    ~TDM();
    vector<demand> demands;
    int getNumServers() { return num_server_;}
    int getNumTORs() { return num_rack_;}

  protected:
    void parseDemand(string infile);
    int num_server_;
    int num_rack_;
};


}

#endif
