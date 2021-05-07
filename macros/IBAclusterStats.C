#if !defined(__CLING__) || defined(__ROOTCLING__)

#include "TROOT.h"
#include "TFile.h"
#include "TTree.h"
#include <TMath.h>
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
//_________________________________________________________________________________________________
void IBAclusterStats(int preciousSensorID = 492)
{

  const Char_t *clsFile = "mftclusters.root";
  const Char_t *o2sim_KineFile = "o2sim_Kine.root";

 // Files & Trees
 // MC
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

 // Loop over MFT Clusters
 std::cout << "Loop over clusters! Filtering preciousSensorID #" << preciousSensorID << std::endl;
  auto  iCluster = 0;
  auto clusterFromTracks = 0;
   for (auto &cluster : clusterMFTVec) {
     if(cluster.getSensorID() == preciousSensorID) {
       auto label = mcLabels->getLabels(iCluster);
       auto nLabels = label.size();
       std::cout << "Cluster # " << iCluster << " has " << nLabels << " labels" << std::endl; // TODO: add to histogram
       auto nPilleUp = 0;
        for ( uint32_t i_lbl = 0 ; i_lbl < nLabels ; i_lbl++) {
          if (DEBUG_VERBOSE) {
            std::cout << "   EventID = " << label[i_lbl].getEventID() <<
     			   " ; trackID = " << label[i_lbl].getTrackID() <<
     			   " ; SourceID = " << label[i_lbl].getSourceID() <<
     			   " ; isFake = " << label[i_lbl].isFake() << std::endl;
          }
          if (label[i_lbl].isValid()) {
            nPilleUp++; // Increment pile up counter for this cluster. If > 1, more than one track contributed to this cluster.
            auto eventID = label[i_lbl].getEventID();
            auto trackID = label[i_lbl].getTrackID();
            auto sourceID = label[i_lbl].getSourceID();

            // Since we know the trackID and the event this comes from, we get her information from the kinematics file (o2sim_Kine.root)
            o2SimKineTree -> GetEntry(eventID);
            MCTrackT<float>* mcTrack =  &(*mcTr).at(trackID);

            if (DEBUG_VERBOSE) {
              std::cout << "    This is a valid label.\n";
              std::cout << "    Add to histograms: trackID " << trackID << " SourceID = " << sourceID << std::endl; // To check the efects of secondaries
              std::cout << "      This track comes from a " << mcTrack->getProdProcessAsString() << std::endl;
              std::cout << "      This track has " << mcTrack->GetEnergy() << " Gev" << std::endl;
              clusterFromTracks++;
            }

          } // isValid
        } // Loop labels
        if (nPilleUp > 1 ) std::cout << "  ================> Pilled up cluster with " << nPilleUp << " tracks" << std::endl;
        // Perhaps nPilleUP should fill a histogram, rather than nLabels. nLabels counts noise.
     } // preciousSensorID
     iCluster++;
 } // Loop on Clusters


 std::cout << std::endl << std::endl;;
 std::cout << "SUMMARY for preciousSensorID = " << preciousSensorID <<  ":"  << std::endl;
 std::cout << nEvents << " events"  << std::endl;
 std::cout << "Total Clusters from tracks: " << clusterFromTracks << std::endl;
 std::cout << 1.0 * clusterFromTracks / nEvents << " clusterfromTracks per event" << std::endl << std::endl;

}
