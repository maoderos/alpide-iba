#if !defined(__CLING__) || defined(__ROOTCLING__)

#include "TROOT.h"
#include "TFile.h"
#include "TTree.h"
#include "TParameter.h"
#include <TMath.h>
#include <TH1F.h>
#include <TH1.h>
#include <TLegend.h>
#include <TGraph.h>
#include <TString>
#include "SimulationDataFormat/MCTrack.h"
#include "SimulationDataFormat/MCCompLabel.h"
#include "SimulationDataFormat/MCTruthContainer.h"
#include "DataFormatsITSMFT/CompCluster.h"
#include <iostream>

#endif

using o2::MCTrackT;

bool DEBUG_VERBOSE = true;

//
// $ root.exe -q IBAclusterStats.C+
//
//________________________________________

std::vector<int> GetIRlist(std::string filename){
   std::vector<int> IRVetor;
   FILE *file = fopen(filename.c_str(),"r");
   char line[80];
   int int_rate;
   while (fgets(line,80,file)) {
      sscanf(&line[0],"%d",&int_rate);
      std::cout << int_rate << std::endl;
      IRVetor.push_back(int_rate);
   }

  return IRVetor;

}


void IBAInteractionRate(int preciousSensorID = 24){

  std::string IR_file = "ir_list.txt";
  std::vector<int>intRate =  GetIRlist(IR_file);

  std::vector<double> efficiency;
  std::vector<double> d_intRate;
  //Loop through all interaction rates

  for(int i = 0; i < intRate.size(); i++){
    // Files & Trees
    // MC

    auto ALPIDEsinglePixel = Form("%d/ALPIDESinglePixel.root",intRate[i]);
    std::cout<<ALPIDEsinglePixel<< std::endl;
    TFile *ALPIDEsinglePixelIn = new TFile(ALPIDEsinglePixel,"READ");

    //ALPIDEsinglePixelIn->ls();

    TParameter<int> *nPrimaries;
    TParameter<int> *nClustersFromPrimaries;

    ALPIDEsinglePixelIn->GetObject("nPrimaries;1",nPrimaries);
    ALPIDEsinglePixelIn->GetObject("nClustersFromPrimaries;1",nClustersFromPrimaries);

    int nPrimaries_val = nPrimaries->GetVal();
    int nClustersFromPrimaries_val = nClustersFromPrimaries->GetVal();

    if (DEBUG_VERBOSE) {
      std::cout << "Primaries from IR = " << intRate[i] << " is: "<< nPrimaries->GetVal() << std::endl;
      std::cout << "Clusters from primaries from IR = " << intRate[i] << " is: "<< nClustersFromPrimaries->GetVal() << std::endl;
    }
    std::cout << 1.0*nClustersFromPrimaries_val/nPrimaries_val << std::endl;
    efficiency.push_back(1.0*nClustersFromPrimaries_val/nPrimaries_val);
    d_intRate.push_back(1.0*intRate[i]);
  }

  TGraph *gr1 = new TGraph(d_intRate.size(),&d_intRate[0],&efficiency[0]);
  gr1->SetTitle("; Protons/s.cm^2; Efficiency");

  int max_x =  d_intRate.size() - 1;
  double max_value = d_intRate[max_x];
  //TF1 *efficiency_th = new TF1("Theoretical Efficiency","(1 - (TMath::PoissonI(0,2*x*9.8805107e-06)))/x/9.8805107e-06/2",0,max_value);

  TLegend *legend = new TLegend(0.8,0.8,0.8,0.8);
  legend->AddEntry(gr1,"Simulation data","p");
  //legend->AddEntry(efficiency_th,"Theoretical eff.","l");
  legend->Draw();

  TCanvas *c1 = new TCanvas();
  c1->cd();
  gr1->Draw("A*");
  //efficiency_th->Draw("SAME");
  legend->Draw();
  c1->SaveAs("Efficiency.pdf");

}
