#include <iostream>
#include <fstream>
#include <vector>
#include <TString.h>
#include <TTree.h>
#include <TH1F.h>
#include <TH2F.h>
#include "Analysis.h"
#include "AT_PiZero.h"
#include "EmcIndexer.h"
#include "EmcIndexer.C"
#include "PbGlIndexer.C"
#include "PbScIndexer.C"

AT_PiZero::AT_PiZero() : AT_ReadTree() {
  for (int i = 0; i < 8; ++i){
    for (int j = 0; j < 48; ++j){
      for (int k = 0; k < 96; ++k){
	EMCMAP[i][j][k] = 0;
      }
    }
  }
  TString fname = "Run16dAu200WarnMap.list";
  std::cout << "AT_PiZero::Ctor === is reading EMCal dead map: ";
  std::cout << fname.Data() << std::endl;
  int armsect = 0, ypos = 0, zpos = 0, status = 0;
  ifstream readmap( fname.Data() );
  while(readmap >> armsect >> ypos >> zpos >> status) {
    EMCMAP[armsect][ypos][zpos] = status;
    //if(status==-1)EMCMAP[armsect][ypos][zpos] = 0; // this is for ERT trigger
  }
  readmap.close();
  fQA = false;
  hVertex = NULL;
  hCentrality = NULL;
  for(int i=0; i!=4; ++i) {
    for(int j=0; j!=8; ++j) {
      hPizeroMass[i][j] = NULL;
    }
  }
}
AT_PiZero::~AT_PiZero() {
}

void AT_PiZero::MyInit() {
  if(fQA) {
    hVertex = new TH1F("Vertex","",100,-30,+30);
    hCentrality = new TH1F("Centrality","",100,0,100);
    for(int i=0; i!=4; ++i) {
      for(int j=0; j!=8; ++j) {
	hPizeroMass[i][j] = new TH2F(Form("PizeroMass_Cut%d_Sector%d",i,j),
				     "Mass;pT", 100,0.0,0.7, 150,0,15);
      }
    }
  }
}

void AT_PiZero::MyFinish() {
  if(fQA) {
    hVertex->Write();
    hCentrality->Write();
    for(int i=0; i!=4; ++i) {
      for(int j=0; j!=8; ++j) {
	hPizeroMass[i][j]->Write();
      }
    }
  }
}

void AT_PiZero::MyExec() {
  fCandidates.clear();
  
  //====== EVENT SELECTION ======
  float cent = fGLB.cent;
  float frac = fGLB.frac;
  float vtxZ = fGLB.vtxZ;
  unsigned int trigger = fGLB.trig;
  unsigned int kBBCnc = 0x00000008;
  unsigned int kBBCn  = 0x00000010;
  unsigned int  mask = kBBCnc | kBBCn;
  bool trig = false;
  if(trigger & mask) trig = true;
  if(cent<0||cent>5) return;
  if(frac<0.95) return;
  if(!trig) return;
  if(fabs(vtxZ)>20) return;
  
  if(fQA) hCentrality->Fill(cent);
  if(fQA) hVertex->Fill(vtxZ);
  //============
  
  
  //====== MAIN LOOP ON CLUSTERS ======
  int isc, jsc;
  int y, z;
  uint nclu = pEMCecore->size();
  for(uint icl=0; icl!=nclu; ++icl) {
    int idx = pEMCtwrid->at(icl);
    float it = pEMCtimef->at(icl);
    EmcIndexer::decodeTowerId(idx,isc,z,y);
    if( IsBad(isc,y,z) ) continue;
    //=== loading cluster i
    float iecore = pEMCecore->at(icl);
    float ix = pEMCx->at(icl);
    float iy = pEMCy->at(icl);
    float iz = pEMCz->at(icl) - vtxZ;
    double idl = TMath::Sqrt(ix*ix + iy*iy + iz*iz);
    TLorentzVector ii;
    ii.SetPx(iecore*ix/idl);
    ii.SetPy(iecore*iy/idl);
    ii.SetPz(iecore*iz/idl);
    ii.SetE(iecore);
    for(uint jcl=icl+1; jcl<nclu; ++jcl) {
      int jdx = pEMCtwrid->at(jcl);
      float jt = pEMCtimef->at(jcl);
      EmcIndexer::decodeTowerId(jdx,jsc,z,y);
      if(isc!=jsc) continue;
      if( IsBad(jsc,y,z) ) continue;
      //=== loading cluster j
      float jecore = pEMCecore->at(jcl);
      float jx = pEMCx->at(jcl);
      float jy = pEMCy->at(jcl);
      float jz = pEMCz->at(jcl) - vtxZ;
      double jdl = TMath::Sqrt(jx*jx + jy*jy + jz*jz);
      TLorentzVector jj;
      jj.SetPx(jecore*jx/jdl);
      jj.SetPy(jecore*jy/jdl);
      jj.SetPz(jecore*jz/jdl);
      jj.SetE(jecore);
      
      //=== building pair
      TLorentzVector pp = ii + jj;
      double dist = TMath::Sqrt( +(ix-jx)*(ix-jx)
				 +(iy-jy)*(iy-jy)
				 +(iz-jz)*(iz-jz) );
      float alpha = TMath::Abs(iecore-jecore)/(iecore+jecore);
      if(pp.Pt()<0.8) continue;
      if(pp.Pt()>16.0) continue;
      if(fQA) hPizeroMass[0][isc]->Fill( pp.M(),pp.Pt()); // step0
      if(dist<8) continue;
      if(fQA) hPizeroMass[1][isc]->Fill( pp.M(),pp.Pt()); // step1
      if(alpha>0.8) continue;
      if(fQA) hPizeroMass[2][isc]->Fill( pp.M(),pp.Pt()); // step2
      if( fabs(it)>5 )continue;
      if( fabs(jt)>5 )continue;
      if(fQA) hPizeroMass[3][isc]->Fill( pp.M(),pp.Pt()); // step3
      fCandidates.push_back( pp );
    }
  }
}

bool AT_PiZero::IsBad(int isc, int y, int z) {
  bool ret = false;

  // convert to veronica convention for sectors
  int vSc = isc;
  if(isc==6) vSc=7;
  if(isc==7) vSc=6;
  if(isc==4) vSc=5;
  if(isc==5) vSc=4;
  isc = vSc;
  //

  if( y==0 || z==0 ) ret = true;
  if( isc < 6 && ( y == 35 || z == 71) ) ret = true;
  if( isc > 5 && ( y == 47 || z == 95) ) ret = true;
  if( EMCMAP[isc][y-1][z-1] || EMCMAP[isc][y][z-1] || EMCMAP[isc][y+1][z-1] ||
      EMCMAP[isc][y-1][z]   || EMCMAP[isc][y][z]   || EMCMAP[isc][y+1][z] ||
      EMCMAP[isc][y-1][z+1] || EMCMAP[isc][y][z+1] || EMCMAP[isc][y+1][z+1] )
    ret = true;
  return ret;
}