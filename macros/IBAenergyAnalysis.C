#if !defined(__CLING__) || defined(__ROOTCLING__)

#include "TROOT.h"
#include "TFile.h"
#include "TTree.h"
#include "TCanvas.h"
#include "TEfficiency.h"
#include "TParameter.h"
#include <TMath.h>
#include <TPad.h>
#include <TH1F.h>
#include <TGraphErrors.h>
#include <TProfile.h>
#include <TH1.h>
#include "SimulationDataFormat/MCTrack.h"
#include "SimulationDataFormat/MCCompLabel.h"
#include "SimulationDataFormat/MCTruthContainer.h"
#include "SimulationDataFormat/BaseHits.h"
#include "DataFormatsITSMFT/CompCluster.h"
#include "DataFormatsITSMFT/ROFRecord.h"
#include "DataFormatsITSMFT/Hit.h"
#include <iostream>

#endif

using o2::MCTrackT;
using o2::itsmft::Hit;

bool DEBUG_VERBOSE = false;

const double protonRestEnergy = 0.93827208816;
const double epitaxialLayerThickness = 25; // in um
//In code (30)
//
// $ root.exe -q IBAclusterStats.C+
//
//_________________________________________________________________________________________________

double GetnSim(std::string filename){
   int nSim;
   FILE *file = fopen(filename.c_str(),"r");
   char line[80];
   int value;
   while (fgets(line,80,file)) {
      sscanf(&line[0],"%d",&value);
      std::cout << value << std::endl;
      nSim = value;
   }

  return nSim;

}

void IBAenergyAnalysis(int preciousSensorID = 21)
{

  int nSim = GetnSim("nSim.txt");
  int clusterFromPrimaries = 0;
  int numberEvents = 0;
  int nPrimaries = 0;
  int TrackWithMultipleClusters = 0;

  // Histogram
  TH1* h_nlabel = new TH1F("nLabel","nLabel",100,0,10);
  //TH1* h_EnergyDeposited = new TH1F("dE/dx (keV/um)","dE/dx (keV/um)",100,0,10);
  TH1* h_sourceid = new TH1F("sourceID","sourceID",100,0,300);
  TH1* h_energyObserved = new TH1F("MC Kinetic energy of observed ions","MC Kinetic energy of observed ions",100,0.0,1.0);
  h_energyObserved->GetXaxis()->SetTitle("Kinetic Energy (GeV)");
  TH1* h_energyMC = new TH1F("MC Kinetic energy","MC Kinetic energy",100,0.0,1.0);
  h_energyMC->GetXaxis()->SetTitle("Kinetic Energy (GeV)");
  TH1* h_nROF_size = new TH1F("size_nROF", "size_nROF",100,0,10);
  TEfficiency* hEff = nullptr;

  TProfile* p_energyDeposited = new TProfile("Geant4 Simulation", "Geant4 Simulation",1000000,0,1,0,100);//100000
  p_energyDeposited->SetTitle(";Kinetic Energy(GeV); dE/dx (keV/um)");
  //p_energyDeposited->SetMarkerColor(9);
  //  p_energyDeposited->SetLineColor(9);
  //p_energyDeposited->SetMarkerStyle(3);

  std::map<double,std::vector<double>> energy_edep;

  for(int i=1; i<=nSim; i++){



    std::string clsFile = std::to_string(i) + "/mftclusters.root";
    std::string o2sim_KineFile = std::to_string(i) + "/o2sim_Kine.root";
    std::string o2sim_MTFHitsFile = std::to_string(i) + "/o2sim_HitsMFT.root";

    // Files & Trees
    // MC
    TFile *o2sim_KineFileIn = new TFile(o2sim_KineFile.c_str());
    TTree *o2SimKineTree = (TTree*) o2sim_KineFileIn -> Get("o2sim");

    std::vector<MCTrackT<float>>* mcTr = nullptr;
    o2SimKineTree -> SetBranchAddress("MCTrack",&mcTr);

    // MFT Tracks
    TFile *clsFileIn = new TFile(clsFile.c_str());
    TTree *mftClusterTree = (TTree*) clsFileIn -> Get("o2sim");
    std::vector<o2::itsmft::CompClusterExt> clusterMFTVec, *clusterMFTVecP = &clusterMFTVec;
    mftClusterTree->SetBranchAddress("MFTClusterComp", &clusterMFTVecP);

    o2::dataformats::MCTruthContainer<o2::MCCompLabel>* mcLabels = nullptr;
    mftClusterTree -> SetBranchAddress("MFTClusterMCTruth",&mcLabels);

    //Info from RO
    std::vector<o2::itsmft::ROFRecord> rofRecVec,*rofRecVecP = &rofRecVec;
    mftClusterTree -> SetBranchAddress("MFTClustersROF", &rofRecVecP);

    //Hits
    TFile *mftHitsFileIn = new TFile(o2sim_MTFHitsFile.c_str());
    TTree *mftHitsTree = (TTree*) mftHitsFileIn -> Get("o2sim");
    std::vector<o2::itsmft::Hit>* mftHitsVec = nullptr;
    mftHitsTree-> SetBranchAddress("MFTHit", &mftHitsVec);

    mftHitsTree->GetEntry(0);

    // Kinematics tree has one entry per event
    std::cout << " Number of entries on o2SimKineTree: " << o2SimKineTree->GetEntries() << std::endl;
    auto nEvents = o2SimKineTree->GetEntries();
    numberEvents = numberEvents + nEvents;
    o2SimKineTree -> GetEntry(0);


    // Hits analysis
     std::cout << "Number of entries on o2sim_HitsMFT: " << mftHitsTree->GetEntries() << std::endl;
     for  (int nEvent = 0; nEvent < mftHitsTree->GetEntries(); nEvent++){
       mftHitsTree->GetEntry(nEvent);
       int nHits = mftHitsVec->size();
       for (int iHit = 0; iHit < nHits; iHit++) {
           Hit* hit = &(*mftHitsVec).at(iHit);
           auto trackID = hit->GetTrackID();
           auto sensorID = hit->GetDetectorID(); // Get track ID of particle
           o2SimKineTree->GetEntry(nEvent);
           MCTrackT<float>* mcTrack =  &(*mcTr).at(trackID);
           if ((mcTrack->isPrimary()) && (sensorID == preciousSensorID)) { // if hit is caused by a primary:
             //double kineticEnergy = mcTrack->GetEnergy() - protonRestEnergy;
             double kineticEnergy = hit->GetTotalEnergy() - protonRestEnergy;
             double theta = mcTrack->GetTheta();
             double distance = TMath::Abs(epitaxialLayerThickness/(TMath::Cos(theta)));
             double energyLoss = (hit->GetEnergyLoss())*1e6; // tranform to KeV
             //h_EnergyDeposited->Fill(energyLoss);
             //std::cout << (energyLoss/distance) << std::endl;
             p_energyDeposited->Fill(kineticEnergy,(energyLoss/distance));
             

           }
       }
     } 

  }

   //Get Theoretical plot
 std::vector<double> TheoEnergy;
 std::vector<double> TheoLossEnergy;

 FILE *file = fopen("energyLossFormat.txt","r");
 char line[80];
 double En;
 double lossEnergy;
 while (fgets(line,80,file)) {
    sscanf(&line[0],"%lf %lf",&En, &lossEnergy);
    //std::cout << En << std::endl;
    TheoEnergy.push_back(En);
    TheoLossEnergy.push_back(lossEnergy);

 }
  
  TGraph *gr1 = new TGraph(TheoEnergy.size(), &TheoEnergy[0], &TheoLossEnergy[0]);
  gr1->SetTitle("SRIM-2013");
  gr1->SetMarkerColor(2);
  gr1->SetLineColor(2);
  
  TCanvas *c1 = new TCanvas();
  c1->cd(1); // divides
  TPad* p1 = (TPad*)(c1->cd(1));
  p1->SetLogx();
  p1->SetLogy();
  p_energyDeposited->Draw("E1");
  gr1->Draw("SAME l");
  p1->BuildLegend();
 
  

  c1->SaveAs("ALPIDEStoppingPower.pdf");

  TFile* pFile = new TFile("ALPIDEStoppingPower.root","recreate");

  p_energyDeposited->Write();
  gr1->Write();
  c1->Write();

}