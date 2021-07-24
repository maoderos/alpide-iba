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

void IBAclusterStats(int preciousSensorID = 21)
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
  TH1* h_energyObserved = new TH1F("MC Kinetic energy of observed ions","MC Kinetic energy of observed ions",250,0.0002,0.0007);
  h_energyObserved->GetXaxis()->SetTitle("Kinetic Energy (GeV)");
  TH1* h_energyMC = new TH1F("MC Kinetic energy","MC Kinetic energy",250,0.0002,0.0007);
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
          auto kinEn = mcTrack->GetEnergy() - protonRestEnergy;
          h_energyMC->Fill(kinEn);
          nPrimaries++;
        }
      }
    }

    // Hits analysis
    // Needs improvement.

     //std::map<std::tupĺe<int,int>, >
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


     // MFT Clusters analysis
     std::map<std::tuple<int,int>,int > clusterPrimaryTrackCount; // Define container
     std::map<std::tuple<int,int>,std::vector<int>> clusterPrimaryTrackROFs;
     std::map<std::tuple<int,int>,std::vector<std::tuple<int,int>> >  PixelPositionOfPrimaryClusterROFs;
     std::map<std::tuple<int,int>,std::vector<int>>  PatternIDOfPrimaryClusterROFs;
     std::cout << "Loop over clusters! Filtering preciousSensorID #" << preciousSensorID << std::endl;
     auto iROF = 0;
     for(auto &ROF:rofRecVec){
      for (auto iCluster = ROF.getFirstEntry(); iCluster < (ROF.getFirstEntry() + ROF.getNEntries()); iCluster++) {
        auto &cluster = clusterMFTVec[iCluster];
        if(cluster.getSensorID() == preciousSensorID) {
          auto label = mcLabels->getLabels(iCluster);
          auto nLabels = label.size();
          if (DEBUG_VERBOSE) {
          std::cout << "Cluster # " << iCluster << " has " << nLabels << " labels" << std::endl;
          }
          //add to histogram
          h_nlabel->Fill(nLabels);
          auto nPilleUp = 0;
           for ( uint32_t i_lbl = 0 ; i_lbl < nLabels ; i_lbl++) {
             h_sourceid->Fill(label[i_lbl].getSourceID());
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
               if (mcTrack->isPrimary()) {

                 clusterPrimaryTrackCount[{eventID,trackID}]=clusterPrimaryTrackCount[{eventID,trackID}]+1;  // Increment number of clusters observed for a track (will create the element if non-existent)
                 clusterPrimaryTrackROFs[{eventID,trackID}].push_back(iROF);
                 PixelPositionOfPrimaryClusterROFs[{eventID,trackID}].push_back(make_tuple(cluster.getRow(), cluster.getCol()));
                 PatternIDOfPrimaryClusterROFs[{eventID,trackID}].push_back(cluster.getPatternID());
                 //std::cout << clusterPrimaryTrackCount[{eventID,trackID}] << std::endl;

                if (clusterPrimaryTrackROFs[{eventID,trackID}].size() < 2) {
                  auto kinEn = mcTrack->GetEnergy() - protonRestEnergy;
                  h_energyObserved->Fill(kinEn);
                  clusterFromPrimaries++;

                }
               }
               if (DEBUG_VERBOSE) {
                 std::cout << "    This is a valid label.\n";
                 std::cout << "    Add to histograms: trackID " << trackID << " SourceID = " << sourceID << std::endl; // To check the efects of secondaries
                 std::cout << "      This track comes from a " << mcTrack->getProdProcessAsString() << std::endl;
                 std::cout << "      This track has " << mcTrack->GetEnergy() << " Gev" << std::endl;

               }

             } // isValid
           } // Loop labels
           if (nPilleUp > 1 ) {
             if (DEBUG_VERBOSE) {
             std::cout << "  ================> Pilled up cluster with " << nPilleUp << " tracks" << std::endl;
             }
             //h_pileUp->Fill(nPilleUp); // Fill pullup
             // Perhaps nPilleUP should fill a histogram, rather than nLabels. nLabels counts noise.
         }
        } // preciousSensorID

    } // Loop on Clusters
    iROF++;
    }

    //loop through map values to identify tracks with 2 or more clusters.
    std::map<std::tuple<int,int>,int>::iterator it = clusterPrimaryTrackCount.begin();
    while(it != clusterPrimaryTrackCount.end()){
      if(it->second > 1){
         TrackWithMultipleClusters++;
         h_nROF_size->Fill(clusterPrimaryTrackROFs[it->first].size());
         if (DEBUG_VERBOSE) {
           std::cout << "size of nROF for trackID " << std::get<1>(it->first) << " EventID "<< std::get<0>(it->first) <<":" << clusterPrimaryTrackROFs[it->first].size() << std::endl;
           std::cout << "ROFs are: ";
           for(auto i=0; i < clusterPrimaryTrackROFs[it->first].size();i++){
             std::cout << clusterPrimaryTrackROFs[it->first][i] << "(" << get<0>(PixelPositionOfPrimaryClusterROFs[it->first][i]) <<
                                                                   "," << get<1>(PixelPositionOfPrimaryClusterROFs[it->first][i]) << ")("
                                                                       << PatternIDOfPrimaryClusterROFs[it->first][i]
                                                                       << ")";
           }
           std::cout << " " << std::endl;
         }
      }
      it++;
     }

  }


  std::cout << "SUMMARY for preciousSensorID = " << preciousSensorID <<  ":"  << std::endl;
  std::cout << numberEvents << " events"  << std::endl;
  std::cout << "Total Primaries: " << nPrimaries << std::endl;
  std::cout << "Total Clusters from primaries: " << clusterFromPrimaries << std::endl;
  //std::cout << 1.0 * clusterFromTracks / nEvents << " clusterfromTracks per event" << std::endl;
  std::cout << 1.0 * nPrimaries / numberEvents << " nPrimaries per event" << std::endl;
  std::cout << 1.0 * clusterFromPrimaries / nPrimaries << " clusterfromTracks per primary" << std::endl;
  std::cout << "Clusters in more than 1 ROF: " << TrackWithMultipleClusters << std::endl;

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
  hEff = new TEfficiency(*h_energyObserved, *h_energyMC);
  hEff->SetTitle("Efficiency");

  TCanvas *c1 = new TCanvas();
  c1->Divide(2,2); // divides
  

  c1->cd(1);
  TPad* p1 = (TPad*)(c1->cd(1));
  
  h_energyMC->Draw();
  c1->cd(2);
  h_energyObserved->Draw();
  TPad* p2 = (TPad*)(c1->cd(2));

  c1->cd(3);
  TPad* p3 = (TPad*)(c1->cd(3));
  p3->SetLogx();
  p3->SetLogy();
  p_energyDeposited->Draw("E1");
  gr1->Draw("SAME l");
  p3->BuildLegend();
  c1->cd(4);
  hEff->Draw();
  TPad* p4 = (TPad*)(c1->cd(4));
  c1->Update();

  //Get painted graph in order to set a minimal value to the y axis.
  auto eff_graph = hEff->GetPaintedGraph();
  eff_graph->SetMinimum(0.0);
  eff_graph->SetMaximum(1.05);

  c1->cd(4);
  eff_graph->Draw();

  c1->SaveAs("ALPIDESinglePixel_Eff.pdf");

  TFile* pFile = new TFile("ALPIDESinglePixel.root","recreate");

  h_energyObserved->Write();
  h_energyMC->Write();
  hEff->Write();
  p_energyDeposited->Write();
  h_sourceid->Write();
  h_nlabel->Write();
  h_nROF_size->Write();
  p_energyDeposited->Write();
  gr1->Write();
  c1->Write();

  // Store some parameters
  TParameter nClustersFromPrimaries("nClustersFromPrimaries", clusterFromPrimaries);
  TParameter nPrimaries_("nPrimaries", nPrimaries);
  pFile->WriteObject(&nPrimaries_,"nPrimaries");
  pFile->WriteObject(&nClustersFromPrimaries,"nClustersFromPrimaries");
  pFile->Close();


/*
  //Energy analysis
  TCanvas* c2 = new TCanvas();
  c2->cd();
  c2->SetLogy();
  c2->SetLogx();
  //c2->SetLogx();
  //p_energyDeposited->SetRangeUser(0, 1e-2, "X");
  p_energyDeposited->Draw("l");
  gr1->Draw("SAME");
  c2->BuildLegend();
  c2->SaveAs(".pdf");
*/

}
