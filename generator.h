#include "./topology.h"
#include "./tdm.h"
#include <string>

namespace NSimulator {
// Base class for flow generator
class Generator {
  public:
    Generator(){}
    virtual ~Generator(){}
    virtual Flow* getNextFlow()=0;
    void loadCDF(string empCDF);
    static double lastArrival;
  protected:
    TDM *tdm_;
    int serverRange_;
    void pairGen(int *src, int *dst);
    
    class cdf {
      public: 	    
        cdf(double nbytes, double value) : nbytes_(nbytes), value_(value) {}
        double nbytes_;		/* Log10 of # of the bytes */
        double value_;
    };

    vector<cdf> cdf_;
    double flowSize();
    double inverseCDF(double y);
};

// fixed arrival interval, fixed flow size, uniform src/dst flow generator
class FFUgenerator : public Generator {
  public:
   explicit FFUgenerator(double avgInt, double flowSize, TDM *tdm, int seed);
   ~FFUgenerator() {}
   Flow* getNextFlow();
  private:
   double flowSize_;
   double avgInt_;
};

// poisson arrival interval, fixed flow size, uniform src/dst flow generator
class PFUgenerator : public Generator {
 public:
  explicit PFUgenerator(double avgInt, double flowSize, TDM *tdm, int seed);
  ~PFUgenerator() {}
  Flow* getNextFlow();
 private:
  double flowSize_;
  double avgInt_;
};


}
