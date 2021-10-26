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
double a_3x3 = 9*(29.24e-4*26.88e-4);

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
  //std::vector<int>intRate ={1000,50000,100000,150000,200000,300000,400000,500000,700000,1000000,3000000,5000000,10000000,15000000,45000000};
  //std::vector<int>intRate ={1000,50000,100000,150000,200000,300000,400000,500000,700000, };
  std::vector<int>intRate ={500000,1000000,5000000,10000000,15000000,20000000};
  //Loop through all interaction rates
  //std::vector<int> protonsEvent = {100,200,300,400,500};
  std::vector<int> protonsEvent = {100,200,300};
  float beamArea = 0.5; // in cm2
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
	
	auto data_name = Form("%d.dat",protonsEvent[j]);
	std::ofstream data(data_name, std::ios_base::app);
	data << 1.0*nClustersFromPrimaries_val/nPrimaries_val << " " << 1.0*intRate[i] << '\n';
	data.close();
	
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
  
 
 // TF1 *efficiency_th = new TF1("2x2 pixel clustering","0.997*(1 - (TMath::PoissonI(0,(4*(29.24e-4*26.88e-4))*x*9.8805107e-06)))/(x*9.8805107e-06*(4*(29.24e-4*26.88e-4)))",0,12e9);
 // efficiency_th->SetTitle("Efficiency of 2x2 pixel clustering from Poisson distribution");
 // efficiency_th->SetLineColor(1);
  
  
  TCanvas* c1 = new TCanvas();
  c1->cd();
  mg1->Draw("AP");
  c1->SaveAs("interactionRatePlot.pdf");
  
  auto fitRes = mg2->Fit("expo","S");
  auto errors = fitRes->Errors();
  auto parameters = fitRes->Parameters();
  
  std::ostringstream equation;
 equation << "fit function: exp(Ax + B)";


 std::ostringstream a;
 a << "A = " << parameters[1] << " #pm " << errors[1];

  
 std::ostringstream b;
 b << "B = " << parameters[0] << " #pm " << errors[0];
  
  auto legend = new TLegend(0.9,0.9,0.9,0.9);
  //legend->AddEntry(efficiency_th,"Eff. of 2x2 pixel clustering from Poisson distribution");
  legend->AddEntry((TObject*)0,equation.str().c_str(), "");
  legend->AddEntry((TObject*)0,a.str().c_str(), "");
  legend->AddEntry((TObject*)0,b.str().c_str(), "");
 
  //gStyle->SetOptFit(0001);
  
  TCanvas* c2 = new TCanvas();
  c2->cd();
  mg2->Draw("AP");
  //efficiency_th->Draw("SAME");
  //mg2->Fit("expo");
  c2->BuildLegend(0.2,0.15,0.2,0.15, "Simulation data");
  legend->Draw();
  
  
 // TF1 * fitFunc = mg2->GetFunction("f1");
  
  c2->SaveAs("fluencePlot.pdf");

  TCanvas* c3 = new TCanvas();
  c3->cd();
  mg3->Draw("AP");
  c3->BuildLegend();
  c3->SaveAs("amperePlot.pdf");

  

}
