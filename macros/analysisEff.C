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


void analysisEff(){


TFile* ALPIDEsinglepixelIn = new TFile("ALPIDESinglePixel.root","READ");

TEfficiency* eff;

ALPIDEsinglepixelIn->GetObject("MC Kinetic energy_clone", eff);


TF1* f1 = new TF1("f1", "[2]*ROOT::Math::normal_cdf(x,[0],[1])", -10, 10);
f1->SetParameters(0.5,1,0.9);
auto fitFunc = eff->GetListOfFunctions()->First();
auto infoFit = eff->Fit(f1,"S");
//double sigma = fitFunc->Parameter(0);
//double xo = fitFunc->Parameter(1);
auto errors = infoFit->Errors();
auto parameters = infoFit->Parameters();
//double error_xo = fitFunc->Error(1);
//TGraph* g1 = new TGraph(1, xPoint, yPoint);
//g1->SetMarkerColor(4);
//g1->SetMarkerStyle(20);
std::ostringstream name_mu;
name_mu << "#mu = " << parameters[1] << " #pm " << errors[1] << " MeV";


std::ostringstream name_sigma;
name_sigma << "#sigma = " << parameters[0] << " #pm " << errors[0] << " MeV";


auto legend = new TLegend(0.1,0.7,0.48,0.9);
TCanvas* c1 = new TCanvas();   
c1->cd();  
eff->Draw();
//g1->Draw("P");  
legend->AddEntry(eff,"Simulation data");
legend->AddEntry(f1,"Error function");
legend->AddEntry((TObject*)0,name_mu.str().c_str(), "");
legend->AddEntry((TObject*)0,name_sigma.str().c_str(), "");
legend->Draw();
c1->SaveAs("eff.pdf");

infoFit->Print();







}
