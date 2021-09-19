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
double e = 1.602E-19;

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


void IBAInteractionFluence(int preciousSensorID = 24){

  std::string IR_file = "ir_list.txt";
  std::vector<int>intRate = {1000,50000,100000,200000};

  //Loop through all interaction rates
  std::vector<int> protonsEvent = {100,200,300,400,500};
  float beamArea = 1.327; // in cm2
  auto mg1 = new TMultiGraph();
  mg1->SetTitle(";Interaction Rate (Ev/s);Efficiency");
  auto mg2 = new TMultiGraph();
  mg2->SetTitle(";Fluence (protons/s.cm2);Efficiency");
  auto mg3 = new TMultiGraph();
  mg3->SetTitle(";A/cm2; Efficiency");
  
  for (int j = 0; j < protonsEvent.size(); j++){
    std::vector<double> efficiency;
    std::vector<double> d_intRate;
    std::vector<double> fluence;
    std::vector<double> ampere_cm;
        
  	for(int i = 0; i < intRate.size(); i++){
      	// Files & Trees
      	// MC

      	auto ALPIDEsinglePixel = Form("%d/%d/ALPIDESinglePixel.root",protonsEvent[j],intRate[i]);
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

        fluence.push_back(1.0*protonsEvent[j]*intRate[i]/beamArea);
	ampere_cm.push_back(1.0*protonsEvent[j]*intRate[i]*e/beamArea);
    }
  
    // First graph is in function of interation Rate
    TGraph* gr1 = new TGraph(d_intRate.size(), &d_intRate[0], &efficiency[0]);
    std::ostringstream name1;

    name1 << protonsEvent[j] << " protons/ev";

    gr1->SetTitle(name1.str().c_str());  
    gr1->SetMarkerStyle(3);
    gr1->SetMarkerColor(j+5);
    gr1->SetLineColor(j+5);

    mg1->Add(gr1);

    TGraph* gr2 = new TGraph(fluence.size(), &fluence[0], &efficiency[0]);

    gr2->SetTitle(name1.str().c_str());
    gr2->SetMarkerStyle(3);
    gr2->SetMarkerColor(j+5);
  
    gr2->SetLineColor(j+5);

    mg2->Add(gr2);

    TGraph *gr3 = new TGraph(ampere_cm.size(), &ampere_cm[0], &efficiency[0]);

    gr3->SetTitle(name1.str().c_str());
    gr3->SetMarkerStyle(3);
    gr3->SetMarkerColor(j+5);
    gr3->SetLineColor(j+5);

    mg3->Add(gr3);
  }

  TCanvas* c1 = new TCanvas();
  c1->cd();
  mg1->Draw("AP");
  c1->BuildLegend();
  c1->SaveAs("interactionRatePlot.pdf");

  TCanvas* c2 = new TCanvas();
  c2->cd();
  mg2->Draw("AP");
  c2->BuildLegend();
  c2->SaveAs("fluencePlot.pdf");

  TCanvas* c3 = new TCanvas();
  c3->cd();
  mg3->Draw("AP");
  c3->BuildLegend();
  c3->SaveAs("amperePlot.pdf");

  

}
