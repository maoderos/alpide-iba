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
#include <TMultiGraph.h>
#include <TString>
#include "SimulationDataFormat/MCTrack.h"
#include "SimulationDataFormat/MCCompLabel.h"
#include "SimulationDataFormat/MCTruthContainer.h"
#include "DataFormatsITSMFT/CompCluster.h"
#include <iostream>

#endif

using o2::MCTrackT;

bool DEBUG_VERBOSE = false;

// Constants
const double e = 1.602E-19;
const float beamArea = 0.5; // in cm2

//
// $ root.exe -q IBAclusterStats.C
//
//________________________________________

void generate_graph(std::vector<int> intRate, std::vector<int> protons_evt, std::string output_name, bool fit){
  
  std::vector<double> efficiency;
  std::vector<double> flux;
  for (int j = 0; j < protons_evt.size(); j++){
    
    for(int i = 0; i<intRate.size();i++){
      auto ALPIDEsinglePixel = Form("%d/%d/ALPIDESinglePixel.root",protons_evt[j],intRate[i]);
      std::cout<<ALPIDEsinglePixel<<std::endl;
      TFile *ALPIDEsinglePixelIn = new TFile(ALPIDEsinglePixel, "READ");

      TParameter<int> *nPrimaries;
      TParameter<int> *nClustersFromPrimaries;

      ALPIDEsinglePixelIn->GetObject("nPrimaries;1", nPrimaries);
      ALPIDEsinglePixelIn->GetObject("nClustersFromPrimaries;1",nClustersFromPrimaries);


      int nPrimaries_val = nPrimaries->GetVal();
      int nClustersFromPrimaries_val = nClustersFromPrimaries->GetVal();

      if (DEBUG_VERBOSE) {
        std::cout << "Primaries from IR = " << intRate[i] << " is: "<< nPrimaries->GetVal() << std::endl;
        std::cout << "Clusters from primaries from IR = " << intRate[i] << " is: "<< nClustersFromPrimaries->GetVal() << std::endl;
      }
      std::cout << 1.0*nClustersFromPrimaries_val/nPrimaries_val << std::endl;
      efficiency.push_back(1.0*nClustersFromPrimaries_val/nPrimaries_val);
      flux.push_back(1.0*protons_evt[j]*intRate[i]/beamArea);
    }
   
  }
  
  
  auto legend = new TLegend(0.9,0.9,0.9,0.9);
   
  
  TGraph* gr = new TGraph(flux.size(), &flux[0], &efficiency[0]);
  std::ostringstream name;
  name << "Simulation";
  gr->SetTitle(name.str().c_str());  
  gr->SetTitle(name.str().c_str());
  gr->SetMarkerStyle(3);
  gr->SetMarkerColor(9);
  gr->SetLineColor(9);
  gr->SetTitle(";Flux (protons/s.cm2);Efficiency");
  
  TCanvas* c1 = new TCanvas();
  c1->cd();
  gr->Draw("AP");
  if (fit){
    TF1 *efficiency_thMASK = new TF1("Mascaramento temporal","0.997*(1 - (TMath::PoissonI(0,(1*(29.24e-4*26.88e-4))*x*2*9.8805107e-06)))/(x*2*9.8805107e-06*((29.24e-4*26.88e-4)))",0,12e9);
    efficiency_thMASK->SetTitle("Temporal masking");
    efficiency_thMASK->SetLineColor(1);  
  
  
    TF1* efficiency_withoutMASK = new TF1("Aglomerados de 2x2 pixeis","0.997*(1 - (TMath::PoissonI(0,(4*(29.24e-4*26.88e-4))*x*9.8805107e-06)))/(x*9.8805107e-06*(4*(29.24e-4*26.88e-4)))",0,12e9);
    efficiency_withoutMASK->SetTitle("2x2 pixel clustering");
    efficiency_withoutMASK->SetLineColor(3); 
    
    efficiency_thMASK->Draw("SAME");
    efficiency_withoutMASK->Draw("SAME");
    gr->Fit("expo");
    legend->AddEntry(gr,"Simulation");
    legend->AddEntry(efficiency_withoutMASK, "2x2 pixel clustering");
    legend->AddEntry(efficiency_thMASK, "Temporal masking");
  } else {
    TF1* efficiency_withoutMASK = new TF1("Aglomerados de 2x2 pixeis","0.997*(1 - (TMath::PoissonI(0,(4*(29.24e-4*26.88e-4))*x*9.8805107e-06)))/(x*9.8805107e-06*(4*(29.24e-4*26.88e-4)))",0,12e9);
    efficiency_withoutMASK->SetTitle("2x2 pixel clustering");
    efficiency_withoutMASK->SetLineColor(3); 
    efficiency_withoutMASK->Draw("SAME");
    legend->AddEntry(efficiency_withoutMASK, "2x2 pixel clustering");
    legend->AddEntry(gr,"Simulation");
  }
  //c1->BuildLegend(0.2,0.15,0.2,0.15, "");
    
 // TF1 * fitFunc = mg2->GetFunction("f1");
  legend->Draw();
  c1->SaveAs(output_name.c_str());
}

void IBAInteractionFluence(){
  std::vector<int>intRate_reduced ={1000,50000,100000,150000,200000,300000,400000,500000,700000};
  std::vector<int>intRate_full ={500000,1000000,5000000,10000000,15000000,20000000};
  //Loop through all interaction rates
  std::vector<int> protonsEvent = {100,200,300};
  generate_graph(intRate_full,protonsEvent,"fluencePlot.pdf",true);
  generate_graph(intRate_reduced,protonsEvent,"fluenceRePlot.pdf",false);
}
