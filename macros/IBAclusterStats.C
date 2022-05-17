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

void IBAclusterStats(int preciousSensorID = 21) {
  std::cout << "funciona" << std::endl;
  int nSim = GetnSim("nSim.txt");
  int clusterFromPrimaries = 0;
  int numberEvents = 0;
  int nPrimaries = 0;
  int TrackWithMultipleClusters = 0;

  // Histogram
  TH1* h_nlabel = new TH1F("nLabel","nLabel",100,0,10);
  //TH1* h_EnergyDeposited = new TH1F("dE/dx (keV/um)","dE/dx (keV/um)",100,0,10);
  TH1* h_sourceid = new TH1F("sourceID","sourceID",100,0,300);
  TH1* h_pileUp = new TH1F("pileUp","pileUp",100,0,10);
  TH1* h_nROF_size = new TH1F("size_nROF", "size_nROF",100,0,10);


  std::map<double,std::vector<double>> energy_edep;

  for(int i=1; i<=nSim; i++){

    std::string clsFile = std::to_string(i) + "/mftclusters.root";
    std::string o2sim_KineFile = std::to_string(i) + "/o2sim_Kine.root";

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

    // All clusters are on the same entry
    std::cout << " Number of entries on mftClusterTree: " << mftClusterTree->GetEntries() << std::endl;
    // Load all Clusters
    mftClusterTree -> GetEntry(0);

    // Kinematics tree has one entry per event
    std::cout << " Number of entries on o2SimKineTree: " << o2SimKineTree->GetEntries() << std::endl;
    auto nEvents = o2SimKineTree->GetEntries();
    numberEvents = numberEvents + nEvents;
    o2SimKineTree -> GetEntry(0);


    // Fill MC tracks Histos
    for  (int nEvent = 0 ; nEvent < nEvents ; nEvent++) {
      o2SimKineTree -> GetEntry(nEvent);
      int nTracks = mcTr->size(); // Should be 1 for single pixel efficiency
      if (DEBUG_VERBOSE) {
      std::cout << "MC Histo event " << nEvent << " with nTracks = " << nTracks << std::endl;
      }
      for (int iTrack = 0 ; iTrack < nTracks ; iTrack++ ) {
        MCTrackT<float>* mcTrack =  &(*mcTr).at(iTrack);
        if(mcTrack->isPrimary()) {
          nPrimaries++;
        }
      }
    }

    // MFT Clusters analysis
    std::cout << "Loop over clusters! Filtering preciousSensorID #" << preciousSensorID << std::endl;
    auto  iCluster = 0;
    for (auto &cluster : clusterMFTVec) {
      if(cluster.getSensorID() == preciousSensorID) {
      	bool have_particle = false;
        auto label = mcLabels->getLabels(iCluster);
        auto nLabels = label.size();
        if (DEBUG_VERBOSE){
          std::cout << "Cluster # " << iCluster << " has " << nLabels << " labels" << std::endl; // TODO: add to histogram
        }
        auto nPilleUp = 0;
        for ( uint32_t i_lbl = 0 ; i_lbl < nLabels ; i_lbl++) {
          if (DEBUG_VERBOSE) {
            std::cout << "   EventID = " << label[i_lbl].getEventID() <<
     			   " ; trackID = " << label[i_lbl].getTrackID() <<
     			   " ; SourceID = " << label[i_lbl].getSourceID() <<
     			   " ; isFake = " << label[i_lbl].isFake() << std::endl;
             h_sourceid->Fill(label[i_lbl].getSourceID());
          }
          if (label[i_lbl].isValid()) {
            nPilleUp++; // Increment pile up counter for this cluster. If > 1, more than one track contributed to this cluster.
            auto eventID = label[i_lbl].getEventID();
            auto trackID = label[i_lbl].getTrackID();
            auto sourceID = label[i_lbl].getSourceID();
            // Since we know the trackID and the event this comes from, we get her information from the kinematics file (o2sim_Kine.root)
            o2SimKineTree -> GetEntry(eventID);
            MCTrackT<float>* mcTrack =  &(*mcTr).at(trackID);
            if (mcTrack->isPrimary()) {
             // h_energyObserved->Fill(mcTrack->GetEnergy());
              have_particle = true;
            }
            if (DEBUG_VERBOSE) {
              std::cout << "    This is a valid label.\n";
              std::cout << "    Add to histograms: trackID " << trackID << " SourceID = " << sourceID << std::endl; // To check the efects of secondaries
              std::cout << "      This track comes from a " << mcTrack->getProdProcessAsString() << std::endl;
              std::cout << "      This track has " << mcTrack->GetEnergy() << " Gev" << std::endl;
            }

          } // isValid
        } // Loop labels
        if(have_particle){
          clusterFromPrimaries++;
        }
        if (nPilleUp > 1 ) {
          if(DEBUG_VERBOSE){
            std::cout << "  ================> Pilled up cluster with " << nPilleUp << " tracks" << std::endl;
          }
          h_pileUp->Fill(nPilleUp); // Fill pullup
          // Perhaps nPilleUP should fill a histogram, rather than nLabels. nLabels counts noise.
      }
     } // preciousSensorID
     iCluster++;
    } // Loop on Clusters

    
  }


  std::cout << "SUMMARY for preciousSensorID = " << preciousSensorID <<  ":"  << std::endl;
  std::cout << numberEvents << " events"  << std::endl;
  std::cout << "Total Primaries: " << nPrimaries << std::endl;
  std::cout << "Total Clusters from primaries: " << clusterFromPrimaries << std::endl;
  //std::cout << 1.0 * clusterFromTracks / nEvents << " clusterfromTracks per event" << std::endl;
  std::cout << 1.0 * nPrimaries / numberEvents << " nPrimaries per event" << std::endl;
  std::cout << 1.0 * clusterFromPrimaries / nPrimaries << " clusterfromTracks per primary" << std::endl;
  
  TFile* pFile = new TFile("ALPIDESinglePixel.root","recreate");

  h_sourceid->Write();
  h_pileUp->Write();
  h_nlabel->Write();
  h_nROF_size->Write();

  // Store some parameters
  TParameter nClustersFromPrimaries("nClustersFromPrimaries", clusterFromPrimaries);
  TParameter nPrimaries_("nPrimaries", nPrimaries);
  pFile->WriteObject(&nPrimaries_,"nPrimaries");
  pFile->WriteObject(&nClustersFromPrimaries,"nClustersFromPrimaries");
  pFile->Close();



}
