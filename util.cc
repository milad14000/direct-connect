
#include "./util.h"
#include <cstdio>
namespace util
{
  double min(double a, double b) {
    if(a>b)
      return b;
    else 
      return a;
  }

  double max(double a, double b) {
    if(a>b)
      return a;
    else
      return b;
  }

  double sum(vector<double> *v) {
    double s=0;
    for (int i=0; i<v->size(); i++) {
      s+=v->at(i);
    }
    return s;
  }
}

