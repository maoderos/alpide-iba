#!/bin/bash

workers=4;

while getopts "n:i:j:" opt; do
  case $opt in
    n) events=$OPTARG;;
    i) ir=$OPTARG;;
    j) workers=$OPTARG;;
  esac
done
p_evt=1;
#shift 3;
#ir=$@;
echo "Number of events is : $events";
echo "Interaction Rates: $ir";
echo "Number of protons per event: ${p_evt}";

start=$SECONDS;
mkdir ${p_evt};
cd ${p_evt};
cp ../alpide-iba/MFTdictionary.bin .;
o2-sim --timestamp 1635659148972 -m MFT -e TGeant3 -g boxgen -j $workers -n $events --configKeyValues 'BoxGun.pdg=2212 ; BoxGun.eta[0]=180.0 ; BoxGun.eta[1]=180.00; BoxGun.number='${p_evt}'; Diamond.position[0]=-2.00;Diamond.position[1]=-3.546;Diamond.position[2]=-45.0;Diamond.width[2]=0; MFTBase.buildHeatExchanger=false; MFTBase.buildFlex=false; MFTBase.minimal=true;';
for i in $ir
do 
  o2-sim-digitizer-workflow -b --skipDet TPC,ITS,TOF,FT0,EMC,HMP,ZDC,TRD,MCH,MID,FDD,PHS,FV0,CPV --interactionRate $i --configKeyValues "MFTDigitizerParam.noisePerPixel=0";
  o2-mft-reco-workflow -b;
  mkdir $i;
  cd $i;
  root.exe -b -q ../../alpide-iba/macros/IBAclusterStats.C;
  cd ..;
done
cp ../alpide-iba/macros/IBAInteractionRate.C .;
../alpide-iba/scripts/plot_eff.sh -i "$ir";
rm IBAInteractionRate.C;
cd ..;
duration=$(( SECONDS - start ));
echo "Simulation ended sucessfuly";
echo "Time of SIM: ${duration}s";
