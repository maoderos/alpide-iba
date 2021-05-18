#if !defined(__CLING__) || defined(__ROOTCLING__)

#include "TROOT.h"
#include "TFile.h"
#include "TTree.h"
#include <TMath.h>
#include <TH1F.h>
#include <TH1.h>
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

void IBAInteractionRate(int preciousSensorID = 492){

  std::vector<double> intRate{1e4,  1e5,  5e2, 5e3,  5e4,  7e2,  7e3};
  const char * intRate_name [] = {"1e4",  "1e5",  "5e2", "5e3",  "5e4",  "7e2",  "7e3"};

  std::vector<double> clusters_by_event;

 //Loop through all interaction rates

  for(int i = 0; i < intRate.size(); i++){
    // Files & Trees
    // MC
    auto clsFile = Form("%s/mftclusters.root",intRate_name[i]);
    auto o2sim_KineFile = Form("%s/o2sim_Kine.root",intRate_name[i]);

    TFile *o2sim_KineFileIn = new TFile(o2sim_KineFile);
    TTree *o2SimKineTree = (TTree*) o2sim_KineFileIn -> Get("o2sim");

    std::vector<MCTrackT<float>>* mcTr = nullptr;
    o2SimKineTree -> SetBranchAddress("MCTrack",&mcTr);

    // MFT Tracks
    TFile *clsFileIn = new TFile(clsFile);
    TTree *mftClusterTree = (TTree*) clsFileIn -> Get("o2sim");
    std::vector<o2::itsmft::CompClusterExt> clusterMFTVec, *clusterMFTVecP = &clusterMFTVec;
    mftClusterTree->SetBranchAddress("MFTClusterComp", &clusterMFTVecP);

    o2::dataformats::MCTruthContainer<o2::MCCompLabel>* mcLabels = nullptr;
    mftClusterTree -> SetBranchAddress("MFTClusterMCTruth",&mcLabels);



    // All clusters are on the same entry
    std::cout << " Number of entries on mftClusterTree: " << mftClusterTree->GetEntries() << std::endl;
    // Load all Clusters
    mftClusterTree -> GetEntry(0);

    // Kinematics tree has one entry per event
    std::cout << " Number of entries on o2SimKineTree: " << o2SimKineTree->GetEntries() << std::endl;
    auto nEvents = o2SimKineTree->GetEntries();
    o2SimKineTree -> GetEntry(0);

    std::cout << "Loop over clusters! Filtering preciousSensorID #" << preciousSensorID << std::endl;
    auto  iCluster = 0;
    auto clusterFromTracks = 0;
    for (auto &cluster : clusterMFTVec) {
      if(cluster.getSensorID() == preciousSensorID) {
        auto label = mcLabels->getLabels(iCluster);
        auto nLabels = label.size();
        //std::cout << "Cluster # " << iCluster << " has " << nLabels << " labels" << std::endl;
        for ( uint32_t i_lbl = 0 ; i_lbl < nLabels ; i_lbl++) {
          if (label[i_lbl].isValid()) {
        //    std::cout << "a" << std::endl;
            clusterFromTracks++;
          }

        }

      }
      iCluster++;
     }
     
     std::cout << 1.0 *clusterFromTracks/nEvents << " for interaction rate of: " << intRate[i] <<std::endl;
     clusters_by_event.push_back(1.0 *clusterFromTracks/nEvents);
  }

  TGraph *gr1 = new TGraph(intRate.size(),&intRate[0],&clusters_by_event[0]);
  gr1->SetTitle("; IntRate; Cluster/Ev");

  TCanvas *c1 = new TCanvas();
  c1->cd();
  gr1->Draw("A*");
  c1->SaveAs("IntRate_Analyis.pdf");

}
