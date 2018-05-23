#ifndef __AT_PIZEROFLOW_HH__
#define __AT_PIZEROFLOW_HH__

#include "AT_ReadTree.h"

class AT_PiZeroFlow : public AT_ReadTree {
 public:
  AT_PiZeroFlow();
  virtual ~AT_PiZeroFlow() {}
  virtual void MyInit();
  virtual void MyExec();
  virtual void MyFinish();

};

#endif