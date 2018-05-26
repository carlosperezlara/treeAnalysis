#ifndef __AT_PIZERO_HH__
#define __AT_PIZERO_HH__

#include <vector>
#include "AT_ReadTree.h"
#include "TLorentzVector.h"

class TH1F;
class TH2F;

class AT_PiZero : public AT_ReadTree {
 public:
  AT_PiZero();
  virtual ~AT_PiZero();
  virtual void MyInit();
  virtual void MyExec();
  virtual void MyFinish();

 private:
  bool IsBad(int sc, int y, int z);
  int  EMCMAP[8][48][96];

  bool fQA;
  TH1F *hVertex;
  TH1F *hCentrality;
  TH2F *hPizeroMass[4][8]; // Step Section

 protected:
  std::vector<TLorentzVector> fCandidates;
};

#endif